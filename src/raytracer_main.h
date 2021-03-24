#ifndef _RAYTRACER_MAIN_H
#define _RAYTRACER_MAIN_H

#include <SDL.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <glad/glad.h>
#include <image_lib.h>
#include <vec3.h>
#include <omp.h>
#include <io.h>
#include <fstream>
#include <future>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include "ImGuizmo.h"
#include "raytracer_imgui_extra.h"
#include "raytracer_ray.h"
#include "raytracer_light.h"
#include "raytracer_geometry.h"
#include "ossstream.h"


using namespace std;
using namespace std::chrono;

// User settings
#define AA_RANDOM 4
#define AA_NONE 0
#define AA_FIVE -1
#define SAMPLING AA_NONE // any SAMPLING > 0 is randomly sampled.
#define RENDER_DELAY 0.0

// Constants
#define H_SPACING 4
#define CHARARRAY_LEN 256

namespace Raytracer {

struct Camera : Object {
    vec3 position = vec3(0, 0, 0);
    vec3 forward = vec3(-1, 0, 0), up = vec3(0, 1, 0), right = vec3(0, 0, 0);
    Color background_color = Color(0, 0, 0);
    float half_vfov = 45;
    vec3i res = vec3i(640, 480, 0);
    vec3i mid_res;
    int max_depth = 5;

    using Object::Object;

    void ImGui();
    string Encode();
    void Decode(string& s);
	void PreRender();
};

struct LoadState {
    int vertex_i = 0;
    vector<vec3> vertices{};
    int normal_i = 0;
    vector<vec3> normals{};
};

// UI STATE
extern int entity_count;
extern Camera* camera;
extern vector<Geometry*> shapes;
extern vector<Material*> materials;
extern vector<Light*> lights;
extern vector<AmbientLight*> ambient_lights;
extern char scene_name[CHARARRAY_LEN];
extern char output_name[CHARARRAY_LEN];
extern steady_clock::time_point last_request;
extern vector<string> debug_log;
extern LoadState load_state;

extern ImVec2 disp_img_size;
extern GLuint disp_img_tex;


vector<Geometry*>::iterator GetIter(Geometry* geo);
vector<Light*>::iterator GetIter(Light* light);
void Delete(Geometry* geo);
void Delete(Light* light);



bool FindIntersection(vector<Geometry*> geometry, Ray ray, HitInformation* intersection);
Color EvaluateRay(Ray ray);
Color CalculateDiffuse(Light* light, HitInformation hit);
Color CalculateSpecular(Light* light, HitInformation hit);
Color CalculateAmbient(HitInformation hit);

void Reset();
void Load();
void UpdateCameraWidget();
void Save();
float PreRender();
void PostRender();
void Render();
void RenderOne();

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
void DisplayImage(string name);
void DisplayLog();
void Log(string s);

void RequestRender();

}  // namespace Raytracer

#endif
