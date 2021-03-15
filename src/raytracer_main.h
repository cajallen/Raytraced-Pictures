#ifndef _MAIN_H
#define _MAIN_H

#include <PGA_3D.h>
#include <SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <array>
#include <fstream>
#include <image_lib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::array;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::stringstream;
using std::vector;

#define X 0
#define Y 1

namespace P3 {

struct Geometry;
struct Sphere;
struct Material;
struct Light;

// This function is used as a helper to parse the keyed lines
// Pseudo: if prefix_matches ? string_without_prefix : "";
inline string rest_if_prefix(const string prefix, string content);

// spaced string stream
struct ossstream {
	ossstream(ostringstream& sstream) : sstream(sstream) {}
	ostringstream& sstream;
};
// Hahahahahahahhaha
inline ossstream& operator << (ossstream& inp_stream, string start) { 
	inp_stream.sstream << endl << start; 
	return inp_stream;
}
template<class T>
ossstream& operator << (ossstream& inp_stream, const T& x) {
	inp_stream.sstream << " " << x ;
	return inp_stream;
}


struct Ray {
    Point3D pos;
    Dir3D dir;
    int bounces_left;

    Ray(Point3D p, Dir3D d, int b) : pos(p), dir(d.normalized()), bounces_left(b) {}
};

struct HitInformation {
    float dist;
    Point3D pos;
    Dir3D viewing;
    Dir3D normal;
    Material* material;
};

// Non-enforced abstract class for raytracer objects. These are both UI objects,
// and entities.
struct Object {
    int id;

    // Call with id to create new
    Object();
    // Call with id to replace
    Object(int old_id);

    virtual void ImGui() {}
    virtual string Encode() { return ""; }
    virtual void Decode(string& s) {}

    string WithId(string s);
    bool operator==(Object rhs) { return id == rhs.id; }
};

struct Material : Object {
    Color ambient = Color(0.5, 0.5, 0.5);
    Color diffuse = Color(0.5, 0.5, 0.5);
    Color specular = Color(0.2, 0.2, 0.2);
    Color transmissive = Color(0, 0, 0);
    float phong = 8.0f;
    float ior = 0.0f;

    using Object::Object;

    void ImGui();
    string Encode();
    void Decode(string& s);
};

struct Camera : Object {
    Point3D position = Point3D(0, 0, 0);
    Dir3D forward = Dir3D(-1, 0, 0), up = Dir3D(0, 1, 0), right = Dir3D(0, 0, 0);
    Color background_color = Color(0, 0, 0);
    float half_vfov = 45;
    array<int, 2> res{640, 480};
    array<int, 2> mid_res{320, 240};
    int max_depth = 5;

    using Object::Object;

    void ImGui();
    string Encode();
    void Decode(string& s);
};

struct Geometry : Object {
    Material* material;

    Geometry();
    Geometry(int old_id);

    vector<Geometry*>::iterator Geometry::GetIter();
    void Delete();

    void ImGui();
    string Encode();
    void Decode(string& s);

    virtual bool FindIntersection(Ray ray, HitInformation* intersection) { return false; }
};

struct Sphere : Geometry {
    Point3D position = Point3D(0, 0, 0);
    float radius = 1.0;

    using Geometry::Geometry;

    void ImGui();
    string Encode();
    void Decode(string& s);

    bool FindIntersection(Ray ray, HitInformation* intersection);
};

// Non-enforced abstract class for lights
struct Light : Object {
    Color color = Color(1, 1, 1);
    float mult = 1.0;
    void UpdateMult();
    void ClampColor();

    using Object::Object;

    void ImGui();
    string Encode();
    void Decode(string& s);
    vector<Light*>::iterator Light::GetIter();
    void Delete();

    // Non-enforced abstract
    virtual Ray ReverseLightRay(Point3D from) { return Ray(Point3D(), Dir3D(), -1); }
    virtual float DistanceTo(Point3D to) { return -1.0f; }
    virtual Color Intensity(Point3D to) { return Color(0, 0, 0); }
};

struct AmbientLight : Light {
    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);
};

struct DirectionalLight : Light {
    Dir3D direction = Dir3D(0, -1, 0);

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(Point3D from);
    float DistanceTo(Point3D to);
    Color Intensity(Point3D to);
};

struct PointLight : Light {
    Point3D position = Point3D(0, 0, 0);

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(Point3D from);
    float DistanceTo(Point3D to);
    Color Intensity(Point3D to);
};

struct SpotLight : Light {
    Point3D position = Point3D(0, 0, 0);
    Dir3D direction = Dir3D(0, -1, 0);
    float angle1 = 30.0;
    float angle2 = 45.0;

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(Point3D from);
    float DistanceTo(Point3D to);
    Color Intensity(Point3D to);
};

Ray Reflect(Dir3D ang, Point3D pos, Dir3D norm, int bounces_left);
Color EvaluateRay(Ray ray);
Color CalculateDiffuse(Light* light, HitInformation hit);
Color CalculateSpecular(Light* light, HitInformation hit);
Color CalculateAmbient(HitInformation hit);

void Reset();
void Load();
void Save();
void Render();

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
void DisplayImage(string name);

bool FindIntersection(vector<Geometry*> geometry, Ray ray, HitInformation* intersection);

}  // namespace P3

#endif
