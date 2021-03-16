#ifndef _RAYTRACER_GRID_H
#define _RAYTRACER_GRID_H

#include <map>
#include <vector>
#include <vec3.h>
#include <math.h>

#include "raytracer_main.h"

namespace P3 {

struct Geometry;

using namespace std;

struct Grid {
	float size;
	map<vec3i, vector<Geometry*>> objects;
	vec3i bb_min;
	vec3i bb_max;

	vec3 pos;
	vec3 dir;
	vec3 dir_sign;

	void AddGeometry(Geometry* geo);
	void RemoveGeometry(Geometry* geo);
	void MoveGeometry(Geometry* geo);

	void InitializeIter(vec3 p, vec3 d);
	bool GetNext(vector<Geometry*>& out);
};

}  // namespace P3

#endif