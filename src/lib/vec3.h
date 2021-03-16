#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <ostream>

inline float fclamp(float a, float min, float max) {
    return fmax(min, fmin(a, max));
}

struct vec3 {
    float x, y, z;

    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3() : x(0), y(0), z(0) {}

    vec3 clamp(float min, float max) const { return vec3(fclamp(x, min, max), fclamp(y, min, max), fclamp(z, min, max)); }

    float mag() const { return sqrt(x * x + y * y + z * z); }
    float mag2() const { return x * x + y * y + z * z; }

    // Create a unit-length vector
    vec3 normalized() const {
        float len = mag();
        return vec3(x / len, y / len, z / len);
    }

    vec3& operator+=(vec3& a) {
        x += a.x;
        y += a.y;
		z += a.z;
        return *this;
    }

    vec3& operator-=(vec3& a) {
        x -= a.x;
        y -= a.y;
		z -= a.z;
        return *this;
    }

	vec3 operator-() const {
		return vec3(-x, -y, -z);
	}
};

inline vec3 operator*(vec3 a, float f) {
    return vec3(a.x * f, a.y * f, a.z * f);
}

inline vec3 operator*(float f, vec3 a) {
    return vec3(a.x * f, a.y * f, a.z * f);
}

// Vector-vector dot product
inline float dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Vector-vector cross product
inline vec3 cross(vec3 a, vec3 b) {
    return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// Vector addition
inline vec3 operator+(vec3 a, vec3 b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// Vector subtraction
inline vec3 operator-(vec3 a, vec3 b) {
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline std::ostream& operator<<(std::ostream& os, vec3 v3) {
    return os << "{" << v3.x << ", " << v3.y << ", " << v3.z << "}";
}

#endif