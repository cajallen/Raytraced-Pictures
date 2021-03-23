#include "raytracer_geometry.h"

namespace Raytracer {

Geometry::Geometry(int* entity_count, Material* mat) : Object(entity_count) {
	material = mat;
}

Geometry::Geometry(int old_id, Material* mat) : Object(old_id) {
	material = mat;
}

bool Sphere::FindIntersection(Ray ray, HitInformation* intersection) {
    vec3 toStart = (ray.pos - position);
    float b = 2.0 * dot(ray.dir, toStart);
    float c = dot(toStart, toStart) - pow(radius, 2);
    float discr = pow(b, 2) - 4.0 * c;

	// no solutions to quadratic equation
    if (discr < 0)
        return false;

	// t is the distance to the solutions of the line sphere intersection
    float t0 = (-b + sqrt(discr)) / 2.0;
    float t1 = (-b - sqrt(discr)) / 2.0;
    float t_min = (RAY_EPSILON < t1 && t1 < t0) ? t1 : t0;

    if (t_min < RAY_EPSILON)
        return false;

    vec3 hit_pos = ray.pos + t_min * ray.dir;
    vec3 hit_norm = (hit_pos - position).normalized();
    *intersection = HitInformation{t_min, hit_pos, ray.dir, hit_norm, material};

    return true;
}

// OPTIMIZATION: 
bool Triangle::FindIntersection(Ray ray, HitInformation* intersection) {
	// Doesn't really matter how we do this. This gives us CCW winding.
    vec3 edge1 = v2 - v1;
    vec3 edge2 = v3 - v1;

    vec3 norm = cross(edge1, edge2).normalized();
    double dvn = dot(ray.dir, norm);
    if (dvn == 0) return false; // Parallel
    double t = -(dot(ray.pos, norm) + -dot(norm, v1)) / dvn;
    vec3 plane_point = ray.pos + ray.dir * t;

	// intersection is behind point
	if (t <= RAY_EPSILON) return false;
    // intersection outside of point
    bool same_side_v1 = same_side(plane_point, v1, v2, v3);
    bool same_side_v2 = same_side(plane_point, v2, v1, v3);
    bool same_side_v3 = same_side(plane_point, v3, v1, v2);
    if (!same_side_v1 || !same_side_v2 || !same_side_v3)
        return false;

	intersection->pos = plane_point;
	intersection->dist = t;
	intersection->material = material;
    intersection->viewing = ray.dir;
	intersection->normal = dvn < 0 ? norm : -norm;
	return true;
}

// TODO: aint too happy that this doesn't reuse code. Finding the plane point is mutual.
bool NormalTriangle::FindIntersection(Ray ray, HitInformation* intersection) {
    vec3 edge1 = v2 - v1;
    vec3 edge2 = v3 - v1;

    vec3 norm = cross(edge1, edge2);
    float d = (dot(ray.dir, norm));
    if (d == 0) return false; // Parallel

    float t = -(dot(ray.pos, norm) + -dot(norm, v1)) / d;
    // intersection is behind point
    if (t <= RAY_EPSILON) return false;

    vec3 plane_point = ray.pos + ray.dir * t;

    // check if intersection outside of point
	float triangle_area = norm.mag();
	float area_1 = cross(v2 - v3, plane_point - v3).mag() / triangle_area;
	float area_2 = cross(v3 - v1, plane_point - v1).mag() / triangle_area;
    float area_3 = cross(v1 - v2, plane_point - v2).mag() / triangle_area;
	if (area_1 > 1 || area_2 > 1 || area_3 > 1 || (area_1+area_2+area_3) > (1 + 100*FLT_EPSILON)) return false;

    intersection->pos = plane_point;
    intersection->dist = t;
    intersection->material = material;
    intersection->viewing = ray.dir;
	vec3 out_norm = n1*area_1 + n2*area_2 + n3*area_3;
    intersection->normal = out_norm;
    return true;
}


void NormalTriangle::PreRender() {
	n1 = n1.normalized();
	n2 = n2.normalized();
	n3 = n3.normalized();
}



BoundingBox Sphere::GetBoundingBox() {
    BoundingBox bb;
    bb.min = position - vec3(radius, radius, radius);
    bb.max = position + vec3(radius, radius, radius);
    return bb;
}

BoundingBox Triangle::GetBoundingBox() {
	BoundingBox bb{v1, v1};
	for (int i = 0; i < 3; i++) {
		bb.min[i] = fmin(bb.min[i], fmin(v2[i], v3[i]));
		bb.max[i] = fmax(bb.max[i], fmax(v2[i], v3[i]));
	}
	return bb;
}


bool Sphere::OverlapsCube(vec3 pos, float hwidth) {
    float d = 0;
    for (int i = 0; i < 3; i++) {
        float min = pos[i] - hwidth;
        float max = pos[i] + hwidth;
        if (position[i] < min) {
            float axis_d = position[i] - min;
            d += axis_d * axis_d;
        } else if (position[i] > max) {
            float axis_d = position[i] - max;
            d += axis_d * axis_d;
        }
    }
    return d < radius * radius;
}

// OPTIMIZATION: This is not correct.
bool Triangle::OverlapsCube(vec3 pos, float hwidth) {
	// This could be implemented, but it's written by a person who cannot code for their life.
    // https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox3.txt
	BoundingBox bb = GetBoundingBox();
	vec3 other_min = pos - vec3(hwidth, hwidth, hwidth);
	vec3 other_max = pos + vec3(hwidth, hwidth, hwidth);
	bool x_overlaps = (bb.max.x > other_min.x && bb.min.x < other_max.x);
	bool y_overlaps = (bb.max.y > other_min.y && bb.min.y < other_max.y);
	bool z_overlaps = (bb.max.z > other_min.z && bb.min.z < other_max.z);
	return x_overlaps && y_overlaps && z_overlaps;
}

}  // namespace Raytracer