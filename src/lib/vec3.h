#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <ostream>

inline float fclamp(float a, float min, float max) {
    return fmax(min, fmin(a, max));
}

struct vec3i {
    int x, y, z;

    vec3i(int x, int y, int z) : x(x), y(y), z(z) {}
    vec3i() : x(0), y(0), z(0) {}
};

vec3i imin(vec3i a, vec3i b) {
	return vec3i(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

vec3i imax(vec3i a, vec3i b) {
	return vec3i(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

template<>
struct hash<vec3i>{
	#define BASE 16
	size_t operator()(const vec3i& v) const {
		return v.x * (BASE*BASE) + v.y * BASE + v.z;
	}
};

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

	float& operator[](int index) {
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		throw runtime_error("vec3[] out of bounds");
		return x;
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