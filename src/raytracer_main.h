#ifndef _RAYTRACER_MAIN_H
#define _RAYTRACER_MAIN_H

#include <SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <image_lib.h>
#include <io.h>
#include <vec3.h>
#include <array>
#include <fstream>
#include <future>
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

struct Camera;
struct Geometry;
struct Sphere;
struct Material;
struct Light;
struct AmbientLight;

// UI STATE
extern vector<Geometry*> shapes;
extern vector<Light*> lights;
extern vector<AmbientLight*> ambient_lights;
extern vector<Material*> materials;
extern Camera* camera;
extern int entity_count;
extern char scene_name[256];
extern char output_name[256];

extern ImVec2 disp_img_size;
extern GLuint disp_img_tex;

// spaced string stream
struct ossstream {
    ossstream(ostringstream& sstream) : sstream(sstream) {}
    ostringstream& sstream;
};
// Hahahahahahahhaha
inline ossstream& operator<<(ossstream& inp_stream, string start) {
    inp_stream.sstream << endl << start;
    return inp_stream;
}
template <class T>
ossstream& operator<<(ossstream& inp_stream, const T& x) {
    inp_stream.sstream << " " << x;
    return inp_stream;
}

struct Ray {
    vec3 pos;
    vec3 dir;
    int bounces_left;

    Ray(vec3 p, vec3 d, int b) : pos(p), dir(d.normalized()), bounces_left(b) {}
};

struct HitInformation {
    float dist;
    vec3 pos;
    vec3 viewing;
    vec3 normal;
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
    vec3 position = vec3(0, 0, 0);
    vec3 forward = vec3(-1, 0, 0), up = vec3(0, 1, 0), right = vec3(0, 0, 0);
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

struct BoundingBox {
	vec3 min = vec3(0, 0, 0);
	vec3 max = vec3(0, 0, 0);

	bool Intersects(BoundingBox other);
	BoundingBox Union(BoundingBox other);
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
	BoundingBox GetBoundingBox();
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
    virtual Ray ReverseLightRay(vec3 from) { return Ray(vec3(), vec3(), -1); }
    virtual float DistanceTo2(vec3 to) { return -1.0; }
    virtual Color Intensity(vec3 to) { return Color(0, 0, 0); }
};

struct AmbientLight : Light {
    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);
};

struct DirectionalLight : Light {
    vec3 direction = vec3(0, -1, 0);

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(vec3 from);
    float DistanceTo2(vec3 to);
    Color Intensity(vec3 to);
};

struct PointLight : Light {
    vec3 position = vec3(0, 0, 0);

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(vec3 from);
    float DistanceTo2(vec3 to);
    Color Intensity(vec3 to);
};

struct SpotLight : Light {
    vec3 position = vec3(0, 0, 0);
    vec3 direction = vec3(0, -1, 0);
    float angle1 = 30.0;
    float angle2 = 45.0;

    using Light::Light;

    void ImGui();
    string Encode();
    void Decode(string& s);

    Ray ReverseLightRay(vec3 from);
    float DistanceTo2(vec3 to);
    Color Intensity(vec3 to);
};

bool FindIntersection(vector<Geometry*> geometry, Ray ray, HitInformation* intersection);
Color EvaluateRay(Ray ray);
Color CalculateDiffuse(Light* light, HitInformation hit);
Color CalculateSpecular(Light* light, HitInformation hit);
Color CalculateAmbient(HitInformation hit);
Ray Reflect(vec3 ang, vec3 pos, vec3 norm, int bounces_left);

void Reset();
void Load();
void Save();
void Render();

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
void DisplayImage(string name);

}  // namespace P3

#endif
