#pragma once

#include "imgui.h"
#include "PGA_3D.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <direct.h>

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::stringstream;

#include <glad/glad.h>          // Initialize with gladLoadGL()


// This function is used as a helper to parse the keyed lines
// Pseudo: if prefix_matches ? string_without_prefix : "";
string rest_if_prefix(const string prefix, string content);

struct P3Material {
    float ambient[3]{ 0.0f, 0.0f, 0.0f };
    float diffuse[3]{ 0.0f, 0.0f, 0.0f };
    float specular[3]{ 0.0f, 0.0f, 0.0f };
    float transmissive[3]{ 0.0f, 0.0f, 0.0f };
    float phong = 0.0f;
    float ior = 0.0f;

    int id = 0;

    // No old_id is needed, as we don't ever "replace" materials (non-polymorphic).
    P3Material();

    void ImGui();
    string Encode();
    void Decode(string s);
    string with_id(string s);
};

// This only needs to exist in preperation for other Geometry primitives.
struct P3Geometry {
    int id;
    P3Material* mat;

    // Call without index to create new shapes
    P3Geometry();
    // Call with index to replace shapes (use old id)
    P3Geometry(int old_id);

    virtual void ImGui();
    virtual string Encode();
    virtual void Decode(string s);

    string with_id(string s);
    vector<P3Geometry*>::iterator GetIter();
    void Delete();


    bool operator==(P3Geometry rhs) { return id == rhs.id; }
};


struct P3Sphere : P3Geometry {
    float pos[3]{ 0.0f, 0.0f, 0.0f };
    float rad = 1.0f;

    // Call without index to create new shapes
    P3Sphere() : P3Geometry() {}
    // Call with index to replace shapes (use old id)
    P3Sphere(int old_id) : P3Geometry(old_id) {}

    void ImGui();
    string Encode();
    void Decode(string s);
};


struct P3Light {
    float col[3] = { 0.0f, 0.0f, 0.0f };
    int id = 0;

    P3Light();
    P3Light(int old_id);

    virtual void ImGui();
    virtual string Encode();
    virtual void Decode(string s);
    vector<P3Light*>::iterator GetIter();
    void Delete();

    string with_id(string s);
};


struct P3AmbientLight : P3Light {
    P3AmbientLight() : P3Light() {}
    P3AmbientLight(int old_id) : P3Light(old_id) {}

    void ImGui();
    string Encode();
    void Decode(string s);
};

struct P3PointLight : P3Light {
    float pos[3] = { 0.0f, 0.0f, 0.0f };

    P3PointLight() : P3Light() {}
    P3PointLight(int old_id) : P3Light(old_id) {}

    void ImGui();
    string Encode();
    void Decode(string s);
};

struct P3DirectionalLight : P3Light {
    float dir[3] = { 0.0f, 0.0f, 0.0f };

    P3DirectionalLight() : P3Light() {}
    P3DirectionalLight(int old_id) : P3Light(old_id) {}

    void ImGui();
    string Encode();
    void Decode(string s);
};

struct P3SpotLight : P3Light {
    float pos[3] = { 0.0f, 0.0f, 0.0f };
    float dir[3] = { 0.0f, 0.0f, 0.0f };
    float angle1 = 0.0f;
    float angle2 = 0.0f;

    P3SpotLight() : P3Light() {}
    P3SpotLight(int old_id) : P3Light(old_id) {}

    void ImGui();
    string Encode();
    void Decode(string s);
};

void Reset();
void Load();
void Save();
void Render();

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);