#include "raytracer_light.h"

namespace Raytracer {

void Light::UpdateMult() {
    color = color * mult;
    mult = 1;
}

void Light::ClampColor() {
    if (color.r > 1 || color.g > 1 || color.b > 1) {
        float max_col = fmax(color.r, fmax(color.g, color.b));
        mult = max_col;
        color.r /= mult;
        color.g /= mult;
        color.b /= mult;
    }
}


Ray DirectionalLight::ReverseLightRay(vec3 from) {
    return Ray{from, -direction, -1};
}

Ray PointLight::ReverseLightRay(vec3 from) {
    vec3 offset = position - from;

    return Ray{from, offset.normalized(), -1};
}

Ray SpotLight::ReverseLightRay(vec3 from) {
    vec3 offset = position - from;

    return Ray{from, offset.normalized(), -1};
}

float DirectionalLight::DistanceTo2(vec3 to) {
    return INFINITY;
}

float PointLight::DistanceTo2(vec3 to) {
    vec3 offset = to - position;
    return offset.mag2();
}

float SpotLight::DistanceTo2(vec3 to) {
    vec3 offset = to - position;
    return offset.mag2();
}

Color DirectionalLight::Intensity(vec3 to) {
    return color;
}

Color PointLight::Intensity(vec3 to) {
    float dist2 = DistanceTo2(to);
    float r = color.r / dist2;
    float g = color.g / dist2;
    float b = color.b / dist2;

    return Color(r, g, b);
}

Color SpotLight::Intensity(vec3 to) {
    vec3 angle_to = (to - position).normalized();
    float diff = 180.0 * acos(dot(angle_to, direction)) / PI;
    float dist2 = DistanceTo2(to);

    if (diff < angle1)
        return color * (1 / dist2);
    float amount = 1 - ((diff - angle1) / (angle2 - angle1));
    if (diff < angle2)
        return color * amount * (1 / dist2);

    return Color(0, 0, 0);
}

}  // namespace Raytracer