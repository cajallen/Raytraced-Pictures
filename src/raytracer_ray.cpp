#include "raytracer_ray.h"

namespace Raytracer {

Ray Ray::Reflect(vec3 dir_in, vec3 origin, vec3 dir_mirror, int bounces_left) {

    vec3 midpoint = dir_mirror * dot(dir_in, dir_mirror);
    vec3 out_dir = (2 * midpoint - dir_in).normalized();
    return Ray(origin, out_dir, bounces_left);
}

Ray Ray::Refract(vec3 dir_in, vec3 origin, vec3 norm, float iornew, int bounces_left) {
    float ratio = iornew;
    float dotValue = dot(dir_in, norm);
    // in direction opposite with normal, we're entering
    if (dotValue < 0) {
        ratio = (1 / iornew);
    }
    // in direction is aligned with normal; we're leaving
    else {
        dotValue = -dotValue;
    }
    float k = 1 - pow(ratio, 2) * (1 - pow(dotValue, 2));
    if (k < 0)
        return Ray(vec3(), vec3(), -1);
    else {
        vec3 dir_out = ratio * dir_in - (ratio * dotValue + sqrt(k)) * norm;
        dir_out = dir_out.normalized();
        return Ray(origin, dir_out, bounces_left);
    }
}

}  // namespace Raytracer
