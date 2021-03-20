#ifndef _RAYTRACER_MAIN_H
#define _RAYTRACER_MAIN_H

#include <SDL.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
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
#include <chrono>
#include "ImGuizmo.h"

using std::array;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::stringstream;
using std::vector;
using namespace std::chrono;

// User settings
#define AA_RANDOM 4
#define AA_NONE 0
#define AA_FIVE -1
#define SAMPLING AA_NONE // any SAMPLING > 0 is randomly sampled.
#define RENDER_DELAY 0.01

// Constants
#define RAY_EPSILON 0.001
#define H_SPACING 4
#define CHARARRAY_LEN

// Readability
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
extern int entity_count;
extern Camera* camera;
extern vector<Geometry*> shapes;
extern vector<Light*> lights;
extern vector<Material*> materials;
extern vector<AmbientLight*> ambient_lights;
extern char scene_name[CHARARRAY_LEN];
extern char output_name[CHARARRAY_LEN];
extern steady_clock::time_point last_request;
extern vector<string> debug_log;

extern ImVec2 disp_img_size;
extern GLuint disp_img_tex;


// Out Spaced String Stream
// If you want to understand any of the Encode functions, this is a prerequisite.
struct ossstream {
    ossstream(ostringstream& sstream) : sstream(sstream) {}
    ostringstream& sstream;
};
// Prepend a newline if we're <<'ing a string literal (ALWAYS a key for scene files)
inline ossstream& operator<<(ossstream& inp_stream, string start) {
    inp_stream.sstream << endl << start;
    return inp_stream;
}
inline ossstream& operator<<(ossstream& inp_stream, const char* start) {
    inp_stream.sstream << endl << start;
    return inp_stream;
}
// Prepend a space if we're <<'ing anything else (ALWAYS a value for scene files)
template <class T>
ossstream& operator<<(ossstream& inp_stream, const T& x) {
    inp_stream.sstream << " " << x;
    return inp_stream;
}

struct Ray {
    vec3 pos;
    vec3 dir;
    int bounces_left;
	Material* last_material = NULL;

    Ray(vec3 p, vec3 d, int b) : pos(p), dir(d.normalized()), bounces_left(b) {}
    static Ray Reflect(vec3 ang, vec3 pos, vec3 norm, int bounces_left);
	static Ray Refract(vec3 dir_in, vec3 origin, vec3 norm, float iornew, int bounces_left);
};

struct HitInformation {
    float dist;
    vec3 pos;
    vec3 viewing;
	vec3 normal;
	// normal can be used as a barycentric coordinate. Code below screws with constructors, so nah.
	/*
	union {
    	vec3 normal;
		vec3 barycentric;
	};
	*/
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
	// String representation for saving.
    virtual string Encode() { return ""; }
    virtual void Decode(string& s) {}
	// Called once right before rendering, allows objects to finalize intermediate data
	virtual void PreRender() {}

    string WithId(string s);
    bool operator==(Object rhs) { return id == rhs.id; }
};

struct Material : Object {
    Color ambient = Color(0.5, 0.5, 0.5);
    Color diffuse = Color(0.5, 0.5, 0.5);
    Color specular = Color(0.2, 0.2, 0.2);
    Color transmissive = Color(0, 0, 0);
    float phong = 8.0f;
    float ior = 1.0f;

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
	void PreRender();
};

struct BoundingBox {
	vec3 min = vec3(0,0,0);
	vec3 max = vec3(0,0,0);
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
	virtual bool OverlapsCube(vec3 pos, float hwidth) { return false; }
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
	bool OverlapsCube(vec3 pos, float hwidth);
	BoundingBox GetBoundingBox();
};

struct Triangle : Geometry {
    vec3 v1 = vec3(), v2 = vec3(), v3 = vec3();

    using Geometry::Geometry;

    virtual void ImGui();
    virtual string Encode();
    virtual void Decode(string& s);

    virtual bool FindIntersection(Ray ray, HitInformation* intersection);
	bool OverlapsCube(vec3 pos, float hwidth);
	BoundingBox GetBoundingBox();
};

struct NormalTriangle : Triangle  {
    vec3 n1 = vec3(), n2 = vec3(), n3 = vec3();
    
    using Triangle::Triangle;

    void ImGui();
    string Encode();
    void Decode(string& s);
	void PreRender();

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

struct LoadState {
    int vertex_i = 0;
    vector<vec3> vertices{};
    int normal_i = 0;
    vector<vec3> normals{};
};

extern LoadState load_state;

bool FindIntersection(vector<Geometry*> geometry, Ray ray, HitInformation* intersection);
Color EvaluateRay(Ray ray);
Color CalculateDiffuse(Light* light, HitInformation hit);
Color CalculateSpecular(Light* light, HitInformation hit);
Color CalculateAmbient(HitInformation hit);

void Reset();
void Load();
void Save();
void Render();
void RenderOne();

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
void DisplayImage(string name);
void DisplayLog();
void Log(string s);

void RequestRender();

}  // namespace P3

#endif
