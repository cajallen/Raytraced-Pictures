#ifndef _RAYTRACER_GEOMETRY_H
#define _RAYTRACER_GEOMETRY_H

#define RAY_EPSILON 0.01

#include <vector>
#include <iostream>
#include "raytracer_ray.h"
#include "raytracer_object.h"
#include <vec3.h>

using namespace std;

namespace Raytracer {

struct BoundingBox {
	vec3 min = vec3(0,0,0);
	vec3 max = vec3(0,0,0);
};

struct Geometry : Object {
    Material* material;

    Geometry(int* entity_count, Material* mat);
    Geometry(int old_id, Material* mat);

    void ImGui();
    string Encode();
    void Decode(string& s);

    virtual bool FindIntersection(Ray ray, HitInformation* intersection) { return false; }
	virtual bool OverlapsCube(vec3 pos, float hwidth) { return false; }
	virtual BoundingBox GetBoundingBox() { return BoundingBox(); }
};

struct Sphere : Geometry {
    vec3 position = vec3(0, 0, 0);
    float radius = 1.0;

    using Geometry::Geometry;

    void ImGui();
    string Encode();
    void Decode(string& s);

    bool FindIntersection(Ray ray, HitInformation* intersection);
	bool OverlapsCube(vec3 pos, float hwidth);
	BoundingBox GetBoundingBox();
};

struct Triangle : Geometry {
    vec3 v1 = vec3(), v2 = vec3(), v3 = vec3();

    using Geometry::Geometry;

    virtual void ImGui();
    virtual string Encode();
    virtual void Decode(string& s);

    virtual bool FindIntersection(Ray ray, HitInformation* intersection);
	bool OverlapsCube(vec3 pos, float hwidth);
	BoundingBox GetBoundingBox();
};

struct NormalTriangle : Triangle  {
    vec3 n1 = vec3(0.577, 0.577, 0.577), n2 = vec3(0.577, 0.577, 0.577), n3 = vec3(0.577, 0.577, 0.577);
    
    using Triangle::Triangle;

    void ImGui();
    string Encode();
    void Decode(string& s);
	void PreRender();

    bool FindIntersection(Ray ray, HitInformation* intersection);

};

}

#endif