// Author: Cole Johnson, 3/16/2021
// Uses details from http://www.cse.chalmers.se/edu/year/2012/course/_courses_2011/TDA361/grid.pdf

#include "raytracer_grid.h"


namespace Raytracer { 

// TODO: If there is an intersection, check to make sure it's in the current voxel.
// Otherwise, check non-mailboxed shapes in the next voxel. Stop when our marked intersection
// is on our current voxel.

void Grid::AddGeometry(Geometry* geo) {
	BoundingBox bb = geo->GetBoundingBox();
	for (int x = (int) bb.min.x; x < ceilf(bb.max.x); x++) {
		for (int y = (int) bb.min.y; y < ceilf(bb.max.y); y++) {
			for (int z = (int) bb.min.z; z < ceilf(bb.max.z); z++) {
				bb_min = imin(bb_min, vec3i(x,y,z));
				bb_max = imax(bb_max, vec3i(x,y,z));
				if (geo->OverlapsCube(vec3(size*x,size*y,size*z), size)) {
					vec3i key = vec3i(x,y,z);
					if (objects.count(key) == 0) {
						pair<vec3i, vector<Geometry*>> to_insert(key, {geo});
						objects.insert(to_insert);
					} else {
						objects.at(key).push_back(geo);
					}
				} 
			}
		}
	}
}

// TODO: cull bounding box
void Grid::RemoveGeometry(Geometry* geo) {
	for (auto const& kvp : objects) {
		value.erase(remove(value.begin(), value.end(), geo));
	}
}

void Grid::MoveGeometry(Geometry* geo) {
	RemoveGeometry(geo);
	AddGeometry(geo);
}

void Grid::InitializeIter(vec3 p, vec3 d) {
	pos = p;
	dir = d;
	// TODO: min(dir) should be 1, not 0.7-1
	dir_sign.x = d.x < 0 ? -1 : 1;
	dir_sign.y = d.y < 0 ? -1 : 1;
	dir_sign.z = d.z < 0 ? -1 : 1;
}

bool Grid::GetNext(vector<Geometry*>& out) {
	// TODO: There is a lot of recalculation here. Save more state.
	// next doesn't change if t does not belong to the axis, but maybe the if statement isnt' worth it.
	// also, coordinate can't be out of bounds in same condition
	if (pos.x < bb_min.x || pos.x > bb_max.x) return false;
	if (pos.y < bb_min.y || pos.y > bb_max.y) return false;
	if (pos.z < bb_min.z || pos.z > bb_max.z) return false;

	vec3i posi = vec3i((int) pos.x, (int) pos.y, (int) pos.z);
	if (objects.count(posi) == 1)
		out = objects.at(posi);
	else 
		out.clear();

	vec3i next;
	next.x = (int) (pos.x + dir_sign.x);
	next.y = (int) (pos.y + dir_sign.y);
	next.z = (int) (pos.z + dir_sign.z);

	float t = fmin(next.x - pos.x, fmin(next.y - pos.y, next.z - pos.z));

	pos = t*dir;
	return true;
}

}  // namespace Raytracer