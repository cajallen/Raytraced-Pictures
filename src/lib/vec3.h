#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <ostream>
#include <string>
#include <stdexcept>
#include <iomanip>

using std::string;
using std::to_string;
using std::runtime_error;

inline float fclamp(float a, float min, float max) {
    return fmax(min, fmin(a, max));
}

struct vec3i {
    int x, y, z;

    vec3i(int x, int y, int z) : x(x), y(y), z(z) {}
    vec3i() : x(0), y(0), z(0) {}
};


inline vec3i imin(vec3i a, vec3i b) {
	return vec3i(fmin(a.x, b.x), fmin(a.y, b.y), fmin(a.z, b.z));
}

inline vec3i imax(vec3i a, vec3i b) {
	return vec3i(fmax(a.x, b.x), fmax(a.y, b.y), fmax(a.z, b.z));
}

#define BASE 16
template<>
struct std::hash<vec3i>{
	size_t operator()(const vec3i& v) const {
		return v.x * (BASE*BASE) + v.y * BASE + v.z;
	}
};

struct vec3 {
    double x, y, z;

    vec3(double x, double y, double z) : x(x), y(y), z(z) {}
    vec3() : x(0), y(0), z(0) {}

    vec3 clamp(double min, double max) const { return vec3(fclamp(x, min, max), fclamp(y, min, max), fclamp(z, min, max)); }

    double mag() const { return sqrt(x * x + y * y + z * z); }
    double mag2() const { return x * x + y * y + z * z; }

    // Create a unit-length vector
    vec3 normalized() const {
        double len = mag();
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


	string keyed_string(string prefix) {
		return prefix + to_string(x) + " " + to_string(y) + " " + to_string(z);
	}

	double& operator[](int index) {
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		throw runtime_error("vec3[] out of bounds");
		return x;
	}
};

inline vec3 operator*(vec3 a, double f) {
    return vec3(a.x * f, a.y * f, a.z * f);
}

inline vec3 operator*(double f, vec3 a) {
    return vec3(a.x * f, a.y * f, a.z * f);
}

// Vector-vector dot product
inline double dot(vec3 a, vec3 b) {
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
    return os << std::fixed << std::setprecision(2) << "{" << v3.z << ", " << v3.y << "}";
}

inline bool same_side(vec3 p1, vec3 p2, vec3 a, vec3 b) {
    vec3 cp1 = cross(b - a, p1 - a);
    vec3 cp2 = cross(b - a, p2 - a);
    return dot(cp1, cp2) >= 0;
}

#endif