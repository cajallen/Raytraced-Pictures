#ifndef _RAYTRACER_BVH_H
#define _RAYTRACER_BVH_H

#include <variant>
#include <vec3.h>
#include <algorithm>
#include <vector>
#include "raytracer_main.h"

using namespace std;

namespace P3 {

struct BVHNode {
	BoundingBox bb;
	variant<vector<BVHNode*>, Geometry*> holding;

	BVHNode(Geometry* geo);

	bool IsLeaf();
	void AddGeometry(Geometry* geo);

	void UpdateBB();
};

}

#endif