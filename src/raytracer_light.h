#ifndef _RAYTRACER_LIGHT_H
#define _RAYTRACER_LIGHT_H

#include <vector>
#include <image_lib.h>
#include <vec3.h>
#include <cmath>
#include "raytracer_object.h"
#include "raytracer_ray.h"

const float PI = 3.1415926536;

namespace Raytracer {

// Non-enforced abstract class for lights
struct Light : Object {
    Color color = Color(1, 1, 1);
    float mult = 1.0;
    void UpdateMult();
    void ClampColor();

    using Object::Object;

    void ImGui();
    string Encode();
    void Decode(string& s);

    // Non-enforced abstract
    virtual Ray ReverseLightRay(vec3 from) { return Ray(vec3(), vec3(), -1); }
    virtual float DistanceTo2(vec3 to) { return -1.0; }
    virtual Color Intensity(vec3 to) { return Color(0, 0, 0); }
};

struct AmbientLight : Light {
    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);
};

struct DirectionalLight : Light {
    vec3 direction = vec3(0, -1, 0);

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(vec3 from);
    float DistanceTo2(vec3 to);
    Color Intensity(vec3 to);
};

struct PointLight : Light {
    vec3 position = vec3(0, 0, 0);

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(vec3 from);
    float DistanceTo2(vec3 to);
    Color Intensity(vec3 to);
};

struct SpotLight : Light {
    vec3 position = vec3(0, 0, 0);
    vec3 direction = vec3(0, -1, 0);
    float angle1 = 30.0;
    float angle2 = 45.0;

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(vec3 from);
    float DistanceTo2(vec3 to);
    Color Intensity(vec3 to);
};

}  // namespace Raytracer

#endif