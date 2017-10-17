// Example raw2_raycast
// - Simple demonstration of ray casting
// - Casts rays from camera space into scene
// - Computes collisions with scene geometry
// - For each collision point calculates lighting

#include <future>
#include <iostream>
#include <ppgso/ppgso.h>
#include <thread>

using namespace std;
using namespace glm;
using namespace ppgso;

// Global constants
const double INF = numeric_limits<double>::max();           // Will be used for infinity
const double EPS = numeric_limits<double>::epsilon();       // Numerical Epsilon
const double DELTA = sqrt(EPS);                             // Delta to use

// Parallel FOR template
template <typename F>
void parallel_for(int begin, int end, F fn, int fragment_size = 0) {
  int fragment_count = thread::hardware_concurrency();
  int length = end - begin;

  if (fragment_size == 0) {
    fragment_size = length / fragment_count;
  }

  if (length <= fragment_size) {
    for (int i = begin; i < end; i++) {
      fn(i);
    }
    return;
  }

  int mid = (begin + end) / 2;
  auto handle = async(std::launch::async, parallel_for<F>, begin, mid, fn, fragment_size);
  parallel_for(mid, end, fn, fragment_size);
  handle.get();
}

/*!
 * Structure holding origin and direction that represents a ray
 */
struct Ray {
  dvec3 origin, direction;

  /*!
   * Compute a point on the ray
   * @param t Distance from origin
   * @return Point on ray where t is the distance from the origin
   */
  inline dvec3 point(double t) const {
    return origin + direction * t;
  }
};

/*!
 * Material coefficients for diffuse and emission
 */
struct Material {
  dvec3 emission, diffuse;
  double shininess;
};

/*!
 * Structure to represent a ray to object collision, the Hit structure will contain material surface normal
 */
struct Hit {
  double distance;
  dvec3 point, normal;
  Material material;
};

/*!
 * Constant for collisions that have not hit any object in the scene
 */
const Hit noHit = { INF, {0,0,0}, {0,0,0}, { {0,0,0}, {0,0,0}, 0 } };

/*!
 * Structure representing a simple camera that is composed on position, up, back and right vectors
 */
struct Camera {
  dvec3 position, back, up, right;

  /*!
 * Generate a new Ray for the given viewport size and position
 * @param x Horizontal position in the viewport
 * @param y Vertical position in the viewport
 * @param width Width of the viewport
 * @param height Height of the viewport
 * @return Ray for the giver viewport position with small random deviation applied to support multi-sampling
 */
  Ray generateRay(int x, int y, int width, int height) const {
    // Camera deltas
    dvec3 vdu = 2.0 * right / (double)width;
    dvec3 vdv = 2.0 * -up / (double)height;

    Ray ray;
    ray.origin = position;
    ray.direction = -back
                    + vdu * ((double)(-width/2 + x) + linearRand(0.0, 1.0))
                    + vdv * ((double)(-height/2 + y) + linearRand(0.0, 1.0));
    ray.direction = normalize(ray.direction);
    return ray;
  }
};

/*!
 * Point light represented by color and position
 */
struct Light {
  dvec3 position, color;
  double att_const, att_linear, att_quad;
};

/*!
 * Abstract class for all shapes.
 */
class Shape {
  public:
    virtual Hit hit(const Ray &ray) const = 0;
};

/*!
 * Structure representing a sphere which is defined by its center position, radius and material
 */
class Sphere: public Shape {
  double radius;
  dvec3 center;
  Material material;

  /*!
   * Compute ray to sphere collision
   * @param ray Ray to compute collision against
   * @return Hit structure that represents the collision or noHit.
   */
  public:
    Sphere(double radius, dvec3 center, Material material) {
      this->radius = radius;
      this->center = center;
      this->material = material;
    }

    Hit hit(const Ray &ray) const {
      auto oc = ray.origin - center;
      auto a = glm::dot(ray.direction, ray.direction);
      auto b = dot(oc, ray.direction);
      auto c = dot(oc, oc) - radius * radius;
      auto dis = b * b - a * c;

      if (dis > 0) {
        auto e = sqrt(dis);
        auto t = (-b - e) / a;

        if ( t > EPS ) {
          auto pt = ray.point(t);
          auto n = normalize(pt - center);
          return {t, pt, n, material};
        }

        t = (-b + e) / a;

        if ( t > EPS ) {
          auto pt = ray.point(t);
          auto n = normalize(pt - center);
          return {t, pt, n, material};
        }
      }
      return noHit;
    }
};

/*!
 * Triangle structure for meshes.
 */
class Triangle: public Shape {
  vector<dvec3> points;
  Material material;

  public:
    Triangle(dvec3 v1, dvec3 v2, dvec3 v3, Material material) {
      this->points.push_back(dvec3(v1));
      this->points.push_back(dvec3(v2));
      this->points.push_back(dvec3(v3));
      this->material = material;
    }

    Hit hit(const Ray &ray) const {
      double t, u, v;

      dvec3 v0v1 = points[1] - points[0];
      dvec3 v0v2 = points[2] - points[0];
      dvec3 pvec = cross(ray.direction, v0v2);
      double det = dot(v0v1, pvec);

      if (fabs(det) < EPS) return noHit;

      double invDet = 1 / det;

      dvec3 tvec = ray.origin - points[0];

      u = dot(tvec, pvec) * invDet;
      if (u < 0 || u > 1) return noHit;

      dvec3 qvec = cross(tvec, v0v1);
      v = dot(ray.direction, qvec) * invDet;
      if (v < 0 || u + v > 1) return noHit;

      t = -1 * dot(v0v2, qvec) * invDet;

      return {t, ray.point(t), normalize(cross(v0v1, v0v2)), material};
    }
};

/*!
 * Mesh structure for meshes.
 */
class MeshObject: public Shape {
  vector<unique_ptr<Shape>> triangles;

public:
  MeshObject(vector<unique_ptr<Shape>> triangles) {
    this->triangles = move(triangles);
  }

  Hit hit(const Ray &ray) const {
    Hit hit = noHit;
    for (auto &triangle : triangles) {
      Hit lh = triangle->hit(ray);
      if (lh.distance < hit.distance) {
        hit = lh;
      }
    }

    return hit;
  }
};

/*!
 * Generate a normalized vector that sits on the surface of a half-sphere which is defined using a normal. Used to generate random diffuse reflections.
 * @param normal Normal that defines the dome/half-sphere direction
 * @return Random 3D vector on the dome surface
 */
inline dvec3 RandomDome(const dvec3 &normal) {
  double d;
  dvec3 p;

  do {
    p = sphericalRand(1.0);
    d = dot(p, normal);
  } while(d < 0);

  return p;
}

/*!
 * Structure to represent the scene/world to render
 */
struct World {
  Camera camera;
  vector<Light> lights;
  vector<unique_ptr<Shape>> spheres;

  // Statistical reports
  mutable atomic<int> current_rows{0};
  mutable atomic<int> current_samples{0};
  mutable atomic<int> current_rays{0};

  /*!
   * Compute ray to object collision with any object in the world
   * @param ray Ray to trace collisions for
   * @return Hit or noHit structure which indicates the material and distance the ray has collided with
   */
  inline Hit cast(const Ray &ray) const {
    auto hit = noHit;
    for ( auto& sphere : spheres) {
      auto lh = sphere->hit(ray);

      if (lh.distance < hit.distance) {
        hit = lh;
      }
    }
    return hit;
  }

  /*!
   * Trace a ray as it collides with objects in the world
   * @param ray Ray to cast
   * @param depth Maximum number of collisions to trace
   * @return Color representing the accumulated lighting for earch ray collision
   */
  inline dvec3 trace(const Ray &ray) const {
    Hit hit = cast(ray);

    // No hit
    if (hit.distance >= INF) return {0, 0, 0};

    // Phong components
    dvec3 ambientColor = {0.1, 0.1, 0.1};
    dvec3 emissionColor = hit.material.emission;
    dvec3 diffuseColor = {0,0,0};
    dvec3 specularColor = {0,0,0};
    for( auto& light : lights) {
      auto lightDirection = light.position - hit.point;
      auto lightDistance = length(lightDirection);
      auto lightNormal = normalize(lightDirection);
      Ray lightRay = {hit.point + hit.normal * DELTA, lightNormal};

      // Light is obscured by object
      auto shadowTest = cast(lightRay);
      if(shadowTest.distance < lightDistance ) continue;

      // Light is visible
      auto att_factor = 1.0 / (light.att_const + light.att_linear * lightDistance + light.att_quad * lightDistance * lightDistance);
      auto dif = glm::clamp(dot(lightRay.direction, hit.normal), 0.0, 1.0);
      diffuseColor += hit.material.diffuse * att_factor * light.color * dif;

      auto spec = glm::clamp(dot(reflect(ray.direction, hit.normal), lightRay.direction), 0.0, 1.0);
      specularColor += light.color * att_factor * pow(spec, hit.material.shininess);
    }

    // Additive lighting result
    dvec3 color = ambientColor + emissionColor + diffuseColor + specularColor;

    return clamp(color, 0.0, 1.0);
  }

  /*!
   * Render the world to the provided image
   * @param image Image to render to
   */
  void render(Image& image, unsigned int samples) const {
    // Render section of the framebuffer
    for(int y = 0; y < image.height; ++y) {
      for (int x = 0; x < image.width; ++x) {
        dvec3 color;
        for (unsigned int i = 0; i < samples; i++) {
          auto ray = camera.generateRay(x, y, image.width, image.height);
          color = color + trace(ray);
        }
        color = color / (double) samples;
        image.setPixel(x, y, (float) color.r, (float) color.g, (float) color.b);
      }
    }
  }
};

/*!
 * Load Wavefront obj file data as vector of faces for simplicity
 * @return vector of Faces that can be rendered
 */
vector<unique_ptr<Shape>> loadObjFile(const string filename) {
  // Using tiny obj loader from ppgso lib
  vector<tinyobj::shape_t> shapes;
  vector<tinyobj::material_t> materials;
  string err = tinyobj::LoadObj(shapes, materials, filename.c_str());

  // Will only convert 1st shape to Faces
  auto &mesh = shapes[0].mesh;

  // Collect data in vectors
  vector<dvec3> positions;
  for (int i = 0; i < (int) mesh.positions.size() / 3; ++i) {
    positions.emplace_back(mesh.positions[3 * i], mesh.positions[3 * i + 1], mesh.positions[3 * i + 2]);
  }

  // Fill the vector of Faces with data
  vector<unique_ptr<Shape>> triangles;
  for (int i = 0; i < (int) (mesh.indices.size() / 3); i++) {
    dvec3 v1 = {positions[mesh.indices[i * 3]].x, positions[mesh.indices[i * 3]].y, positions[mesh.indices[i * 3]].z};
    dvec3 v2 = {positions[mesh.indices[i * 3 + 1]].x, positions[mesh.indices[i * 3 + 1]].y, positions[mesh.indices[i * 3 + 1]].z};
    dvec3 v3 = {positions[mesh.indices[i * 3 + 2]].x, positions[mesh.indices[i * 3 + 2]].y, positions[mesh.indices[i * 3 + 2]].z};
    triangles.emplace_back(unique_ptr<Triangle>(new Triangle(v1, v2, v3, { { 0, 0, 0}, { 1, 0, 0}, 0, 0 })));
  }
  return triangles;
}

int main() {
  // Image to render to
  Image image {512, 512};

  vector<unique_ptr<Shape>> shapes;
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0, -10010, 0}, { {0, 0, 0}, {.8, .8, .8}, 1, 0 } ) ));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0,10010, 0}, { { .3, .3, .3}, { .8, .8, .8}, 1, 0 } )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, { -10010, 0, 0}, { { 0, 0, 0}, { 1, 0, 0}, 1, 0 } )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  10010, 0, 0}, { { 0, 0, 0}, { 0, 1, 0}, 1, 0 } )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0,0, -10010}, { { 0, 0, 0}, { .8, .8, 0}, 1, 1 } )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0,0, 10030}, { { 0, 0, 0}, { .8, .8, 0}, 1, 1 } )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 2, { -5,  -8,  3}, { { 0, 0, 0}, { .7, .7, 0}, 3, 0 } )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 4, {  0,  -6,  0}, { { 0, 0, 0}, { .7, .5, .1}, 5, 0 } )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10, {  10, 10, -10}, { { 0, 0, 0}, { 0, 0, 1}, 30, 0 } )));
  shapes.push_back(unique_ptr<MeshObject>(new MeshObject (loadObjFile("bunny.obj"))));

  vector<unique_ptr<Shape>> vec1 (move(shapes));

  // World to render
  const World world = {
      { // Camera
          {  0,   0, 25}, // pos
          {  0,   0,  1}, // back
          {  0,  .5,  0}, // up
          { .5,   0,  0}, // right
      },
      { // Lights
          { {-5, 5, 9}, {1, 1, 1}, 1, .1, 0 },
          { { 5, 0, 15}, {0.2, 0.5, 0.2}, 1, .1, .01 },
      },
      { // Spheres
          move(vec1)
      },
  };

  // Render the scene
  world.render(image, 4);

  // Save the result
  image::saveBMP(image, "raw2_raycast.bmp");

  cout << "Done." << endl;
  return EXIT_SUCCESS;
}
