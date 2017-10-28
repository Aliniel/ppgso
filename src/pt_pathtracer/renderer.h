#pragma once

#include <chrono>
#include <future>

#include "ray.h"
#include "shape.h"
#include "camera.h"

// Parallel FOR template
template <typename F>
void parallel_for(int begin, int end, F fn, int fragment_size = 0) {
  int fragment_count = std::thread::hardware_concurrency();
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

class Renderer {
  // Statistical reports
  mutable std::atomic<int> current_rows{0};
  mutable std::atomic<int> current_samples{0};
  mutable std::atomic<int> current_rays{0};

public:
  Camera camera;
  std::vector<std::unique_ptr<Shape>> scene;

  struct pixel {
    int samples = 0;
    Color color;

    inline void add(Color sample) {
      samples++;
      color = color + (sample - color) / (float) samples;
    }
  };

  std::vector<pixel> samples;
  int width;
  int height;

  inline Renderer(int width, int height) : width{width}, height{height}, samples{(size_t)width * height} {}

  inline void render(int depth = 5) {
    auto rendering = async(
      std::launch::async,
      [&] () {
        parallel_for(
          0,
          height,
          // Lambda function
          [&](int y) {
            for (int x = 0; x < width; ++x) {
              auto ray = camera.generateRay(x, y, width, height);
              Color sample = trace(ray, depth);
              samples[width*y+x].add(sample);
              this->current_samples ++;
            }
            this->current_rows ++;
          }
        );
      }
    );

    std::chrono::milliseconds span (1000);
    while (rendering.wait_for(span) == std::future_status::timeout) {
      int progress = (int)(((float)current_rows / (float)height) * 100.0f) % 100;
      std::cout << "Progress: " << progress << "%.\n"
           << "Samples per second: " << current_samples << ".\n"
           << "Rays per second: " << current_rays << ".\n"
           << "\n";
      current_samples = 0;
      current_rays = 0;
    }


//    // Render the scene
//    #pragma omp parallel for
//    for (int y = 0; y < height; ++y) {
//      for (int x = 0; x < width; ++x) {
//        auto ray = camera.generateRay(x, y, width, height);
//        Color sample = trace(ray, depth);
//        samples[width*y+x].add(sample);
//        this->current_samples ++;
//      }
//      this->current_rows ++;
//    }

 }


  /*!
   * Compute ray to object collision with any object in the world
   * @param ray Ray to trace collisions for
   * @return Hit or noHit structure which indicates the material and distance the ray has collided with
   */
  inline Hit cast(const Ray &ray) const {
    Hit hit = noHit;
    for (auto &shape : scene) {
      auto lh = shape->intersect(ray);

      if (lh.distance < hit.distance && lh.distance > DELTA) {
        hit = lh;
      }
    }
    this->current_rays ++;
    return hit;
  }

 /*!
 * Trace a ray as it collides with objects in the world
 * @param ray Ray to trace
 * @param depth Maximum number of collisions to trace
 * @return Color representing the accumulated lighting for each ray collision
 */
  inline Color trace(const Ray &ray, unsigned int depth) const {
    if (depth == 0) return {0, 0, 0};

    const Hit hit = cast(ray);

    // No hit
    if ( hit.distance >= INF) return {0, 0, 0};

    // Emission
    Color color = hit.material->emission;

    // Decide to reflect or refract using linear random
    if (hit.material->type == MaterialType::REFRACTIVE) {
      const float refractionIndex = 1.5;

      // Ideal specular reflection
      Direction reflection = reflect(ray.direction, hit.normal);
      // Ray of reflection
      Ray reflectedRay{hit.position, reflection};

       // Flip normal if the ray is "inside" a sphere
      Direction normal = dot(ray.direction, hit.normal) < 0 ? hit.normal : -hit.normal;
      // Reverse the refraction index as well
      float r_index = dot(ray.direction, hit.normal) < 0 ? 1/refractionIndex : refractionIndex;

      // Total internal refraction
      float ddn = dot(ray.direction, hit.normal);
      float cos2t = 1-r_index*r_index*(1-ddn*ddn);
      if(cos2t < 0)
        return hit.material->emission + trace(reflectedRay, depth - 1);

      // Prepare refraction ray
      Direction refraction = refract(ray.direction, normal, r_index);
      Ray refractionRay{hit.position, refraction};
      // Trace the ray recursively
      color += trace(refractionRay, depth - 1);
    }

    if (hit.material->type == MaterialType::SPECULAR) {
      // Ideal specular reflection
      Direction reflection = reflect(ray.direction, hit.normal);
      // Ray of reflection
      Ray reflectedRay{hit.position, reflection};
      // Trace the ray recursively
      color += trace(reflectedRay, depth - 1);
    }

   if (hit.material->type == MaterialType::DIFFUSE) {
     // Random diffuse reflection
     Direction diffuse = CosineSampleHemisphere(hit.normal);
     // Random diffuse ray
     Ray diffuseRay{hit.position, diffuse};
     // Trace the ray recursively
     color += hit.material->diffuse * trace(diffuseRay, depth - 1);
   }

    return color;
  }

  template<typename T>
  void add(T shape) {
    scene.emplace_back(std::make_unique<T>(std::move(shape)));
  }
};
