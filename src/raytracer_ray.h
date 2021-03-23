#ifndef _RAYTRACER_RAY_H
#define _RAYTRACER_RAY_H

#include <vec3.h>
#include "raytracer_object.h"
#include "image_lib.h"

namespace Raytracer {

struct Material : Object {
    Color ambient = Color(0.5, 0.5, 0.5);
    Color diffuse = Color(0.5, 0.5, 0.5);
    Color specular = Color(0.2, 0.2, 0.2);
    Color transmissive = Color(0, 0, 0);
    float phong = 8.0f;
    float ior = 1.0f;

    using Object::Object;

    void ImGui();
    string Encode();
    void Decode(string& s);
};

struct Ray {
    vec3 pos;
    vec3 dir;
    int bounces_left;
	Material* last_material = NULL;

    Ray(vec3 p, vec3 d, int b) : pos(p), dir(d.normalized()), bounces_left(b) {}
    static Ray Reflect(vec3 ang, vec3 pos, vec3 norm, int bounces_left);
	static Ray Refract(vec3 dir_in, vec3 origin, vec3 norm, float iornew, int bounces_left);
};

struct HitInformation {
    float dist;
    vec3 pos;
    vec3 viewing;
	vec3 normal; // normal can be used as a barycentric coordinate. 
    Material* material;
};

}  // namespace Raytracer

#endif
