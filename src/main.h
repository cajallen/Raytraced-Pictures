#pragma once

#include "imgui.h"
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

    P3Material();
    P3Material(string s);

    void ImGui();
    string String();
    string with_id(string s);
};

// This only needs to exist in preperation for other Geometry primitives.
struct P3Geometry {
    int id;
    P3Material mat;

    // Call without index to create new shapes
    P3Geometry();
    // Call with index to replace shapes (use old id)
    P3Geometry(int old_id);
    virtual void ImGui();
    virtual const string Name();
    vector<P3Geometry*>::iterator GetIter();
    void Delete();

    string with_id(string s);

    bool operator==(P3Geometry rhs) { return id == rhs.id; }
};


struct P3Sphere : P3Geometry {
    float pos[3]{ 0.0f, 0.0f, 0.0f };
    float rad = 1.0f;

    // Call without index to create new shapes
    P3Sphere() : P3Geometry() {}
    // Call with index to replace shapes (use old id)
    P3Sphere(int old_id) : P3Geometry(old_id) {}

    const string Name();
    void ImGui();
};


void Reset();
void Load();
void Render();