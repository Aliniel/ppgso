// Example raw2_raycast
// - Simple demonstration of ray casting
// - Casts rays from camera space into scene
// - Computes collisions with scene geometry
// - For each collision point calculates lighting

#include <chrono>
#include <future>
#include <iostream>
#include <ppgso/ppgso.h>

#include "../pt_pathtracer/material.h"
#include "../pt_pathtracer/ray.h"
#include "../pt_pathtracer/shape.h"
#include "../pt_pathtracer/sphere.h"

using namespace std;
using namespace glm;
using namespace ppgso;

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
 * Structure representing a simple camera that is composed on position, up, back and right vectors
 */
struct Camera {
  vec3 position, back, up, right;

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
    vec3 vdu = 2.0f * right / (float)width;
    vec3 vdv = 2.0f * -up / (float)height;

    Ray ray;
    ray.origin = position;
    ray.direction = -back
                    + vdu * ((float)(-width/2 + x) + linearRand(0.0f, 1.0f))
                    + vdv * ((float)(-height/2 + y) + linearRand(0.0f, 1.0f));
    ray.direction = normalize(ray.direction);
    return ray;
  }
};

/*!
 * Point light represented by color and position
 */
struct Light {
  vec3 position, color;
  float att_const, att_linear, att_quad;
};

/*!
 * Triangle structure for meshes.
 */
class Triangle: public Shape {
  vector<vec3> points;
  Material material;

  public:
    Triangle(vec3 v1, vec3 v2, vec3 v3, Material material) {
      this->points.push_back(vec3(v1));
      this->points.push_back(vec3(v2));
      this->points.push_back(vec3(v3));
      this->material = material;
    }

    Hit intersect(const Ray &ray) const {
      float t, u, v;

      vec3 v0v1 = points[1] - points[0];
      vec3 v0v2 = points[2] - points[0];
      vec3 pvec = cross(ray.direction, v0v2);
      float det = dot(v0v1, pvec);

      if (fabs(det) < EPS) return noHit;

      float invDet = 1 / det;

      vec3 tvec = ray.origin - points[0];

      u = dot(tvec, pvec) * invDet;
      if (u < 0 || u > 1) return noHit;

      vec3 qvec = cross(tvec, v0v1);
      v = dot(ray.direction, qvec) * invDet;
      if (v < 0 || u + v > 1) return noHit;

      t = -1 * dot(v0v2, qvec) * invDet;

      return {t, ray.point(t), normalize(cross(v0v1, v0v2)), &material};
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

  Hit intersect(const Ray &ray) const {
    Hit hit = noHit;
    for (auto &triangle : triangles) {
      Hit lh = triangle->intersect(ray);
      if (lh.distance < hit.distance) {
        hit = lh;
      }
    }

    return hit;
  }
};

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
      auto lh = sphere->intersect(ray);

      if (lh.distance < hit.distance && lh.distance > 0.01) {
        hit = lh;
      }
    }
    current_rays ++;
    return hit;
  }

  /*!
   * Trace a ray as it collides with objects in the world
   * @param ray Ray to cast
   * @param depth Maximum number of collisions to trace
   * @return Color representing the accumulated lighting for earch ray collision
   */
  inline vec3 trace(const Ray &ray, unsigned int depth) const {
    Hit hit = cast(ray);

    // No hit
    if (hit.distance >= INF) return {0, 0, 0};

    // Phong components
    vec3 ambientColor = {0.1f, 0.1f, 0.1f};
    vec3 emissionColor = hit.material->emission;
    vec3 diffuseColor = {0,0,0};
    vec3 specularColor = {0,0,0};
    vec3 color = {0, 0, 0};
    if (hit.material->type == MaterialType::SPECULAR && depth != 0) {
      // Reflect
      vec3 reflection = reflect(ray.direction, hit.normal);
      Ray reflection_ray = Ray{hit.position + hit.normal * DELTA, reflection};
      color += trace(reflection_ray, depth - 1);
    } else {
      for( auto& light : lights) {
        auto lightDirection = light.position - hit.position;
        auto lightDistance = length(lightDirection);
        auto lightNormal = normalize(lightDirection);
        Ray lightRay = {hit.position + hit.normal * DELTA, lightNormal};

        // Light is obscured by object
        auto shadowTest = cast(lightRay);
        if(shadowTest.distance < lightDistance ) continue;

        // Light is visible
        float att_factor = 1.0f / (light.att_const + light.att_linear * lightDistance + light.att_quad * lightDistance * lightDistance);
        auto dif = glm::clamp(dot(lightRay.direction, hit.normal), 0.0f, 1.0f);
        diffuseColor += hit.material->diffuse * att_factor * light.color * dif;

//        auto spec = glm::clamp(dot(reflect(ray.direction, hit.normal), lightRay.direction), 0.0, 1.0);
//        specularColor += light.color * att_factor * pow(spec, hit.material.shininess);
      }

      // Additive lighting result
      color = ambientColor + emissionColor + diffuseColor + specularColor;
    }

    return clamp(color, 0.0f, 1.0f);
  }


  /*!
   * Render the world to the provided image
   * @param image Image to render to
   */
  void render(Image& image, unsigned int samples) const {
    // Get the start time of the execution
    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    auto rendering = async(
      launch::async,
      [&] () {
        parallel_for(
          0,
          image.height,
          // Lambda function
          [&](int y) {
            for (int x = 0; x < image.width; ++x) {
              vec3 color;
              for (unsigned int i = 0; i < samples; i++) {
                auto ray = camera.generateRay(x, y, image.width, image.height);
                color = color + trace(ray, 2);
                current_samples ++;
              }
              color = color / (float) samples;
              image.setPixel(x, y, color.r, color.g, color.b);
            }
            current_rows ++;
          }
        );
      }
    );

    std::chrono::milliseconds span (1000);
    while (rendering.wait_for(span) == std::future_status::timeout) {
      int progress = (int)(((float)current_rows / (float)image.height) * 100.0f);
      cout << "Progress: " << progress << "%.\n"
           << "Samples per second: " << current_samples << ".\n"
           << "Rays per second: " << current_rays << ".\n"
           << "\n";
      current_samples = 0;
      current_rays = 0;
    }

    // Get the end time of the execution and log the duration into the stdout
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = chrono::duration_cast<chrono::duration<double>> (end - start);
    cout << "Rendering time: " << duration.count() << "\n";
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
  vector<vec3> positions;
  for (int i = 0; i < (int) mesh.positions.size() / 3; ++i) {
    positions.emplace_back(mesh.positions[3 * i], mesh.positions[3 * i + 1], mesh.positions[3 * i + 2]);
  }

  // Fill the vector of Faces with data
  vector<unique_ptr<Shape>> triangles;
  for (int i = 0; i < (int) (mesh.indices.size() / 3); i++) {
    vec3 v1 = {positions[mesh.indices[i * 3]].x, positions[mesh.indices[i * 3]].y, positions[mesh.indices[i * 3]].z};
    vec3 v2 = {positions[mesh.indices[i * 3 + 1]].x, positions[mesh.indices[i * 3 + 1]].y, positions[mesh.indices[i * 3 + 1]].z};
    vec3 v3 = {positions[mesh.indices[i * 3 + 2]].x, positions[mesh.indices[i * 3 + 2]].y, positions[mesh.indices[i * 3 + 2]].z};
    triangles.emplace_back(unique_ptr<Triangle>(new Triangle(v1, v2, v3, Material::Gray())));
  }
  return triangles;
}

int main() {
  // Image to render to
  Image image {512, 512};

  vector<unique_ptr<Shape>> shapes;
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0, -10010, 0}, Material::Green() ) ));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0,10010, 0}, Material::Red() )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, { -10010, 0, 0}, Material::White() )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  10010, 0, 0}, Material::Mirror() )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0,0, -10010}, Material::Mirror() )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10000, {  0,0, 10030}, Material::White() )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 2, { -5,  -8,  3}, Material::Blue() )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 4, {  0,  -6,  0}, Material::Cyan() )));
  shapes.push_back(unique_ptr<Sphere>(new Sphere( 10, {  10, 10, -10}, Material::Yellow() )));
//  shapes.push_back(unique_ptr<MeshObject>(new MeshObject (loadObjFile("bunny.obj"))));

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
