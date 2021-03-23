#ifndef _RAYTRACER_GRID_H
#define _RAYTRACER_GRID_H

#include <unordered_map>
#include <vector>
#include <vec3.h>
#include <math.h>

#include "raytracer_geometry.h"

namespace Raytracer {

struct Geometry;

using namespace std;


#define BASE 16
auto hash = [](const vec3i& v) {
	return v.x * (BASE * BASE) + v.y * BASE + v.z;
};
auto equal = [](const vec3i& a, const vec3i& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
};

struct Grid {
	float size;
	unordered_map<vec3i, vector<Geometry*>, decltype(hash), decltype(equal)> objects(8, hash, equal);
	vec3i bb_min;
	vec3i bb_max;

	vec3 pos;
	vec3 dir;
	vec3 dir_sign;

	Grid(float s) : size(s) {}

	void AddGeometry(Geometry* geo);
	void RemoveGeometry(Geometry* geo);
	void MoveGeometry(Geometry* geo);

	void InitializeIter(vec3 p, vec3 d);
	bool GetNext(vector<Geometry*>& out);
};

}  // namespace Raytracer

#endif