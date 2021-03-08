// Main templated from Dear ImGui

#include "main.h"

// GLOBAL STATE
vector<P3Geometry*> shapes{};
vector<P3Light*> lights{};
// We need to keep a vector of materials to free them.
vector<P3Material*> materials{ new P3Material() };
int entity_count = 0;
char scene_name[128] = "";

// CAMERA SETTINGS
float cp[3]{ 0.0f, 0.0f, 0.0f };
float cf[3]{ 0.0f, 0.0f, 1.0f };
float cu[3]{ 0.0f, 1.0f, 0.0f };
float fov_ha = 45.0f;
int window_res[2]{ 640, 480 };

// OTHER SETTINGS
int max_depth = 5;
char output_name[128] = "";


string rest_if_prefix(const string prefix, string content) {
    if (content.compare(0, prefix.length(), prefix) == 0) {
        return content.substr(prefix.length());
    } else {
        return "";
    }
}


string P3Geometry::with_id(string s) {
    return (s + std::to_string(id)).c_str();
}

string P3Material::with_id(string s) {
    return (s + std::to_string(id)).c_str();
}

string P3Light::with_id(string s) {
    return (s + std::to_string(id)).c_str();
}


// Call without index to create new shapes
P3Geometry::P3Geometry() {
	id = entity_count;
	entity_count++;
    mat = materials.back();
}
// Call with index to replace shapes (use old id)
P3Geometry::P3Geometry(int old_id) {
	id = old_id;
    mat = materials.back();
}

P3Light::P3Light() {
    id = entity_count;
    entity_count++;
}

P3Light::P3Light(int old_id) {
    id = old_id;
}

void P3Geometry::Delete() { shapes.erase(GetIter()); }
void P3Light::Delete() { lights.erase(GetIter()); }


vector<P3Geometry*>::iterator P3Geometry::GetIter() {
	for (vector<P3Geometry*>::iterator it = shapes.begin(); it < shapes.end(); it++) {
		if ((*it)->id == id) {
			return it;
		}
	}
    cout << "Geometry not found in shapes" << endl;
    return shapes.end();
}


vector<P3Light*>::iterator P3Light::GetIter() {
    for (vector<P3Light*>::iterator it = lights.begin(); it < lights.end(); it++) {
        if ((*it)->id == id) {
            return it;
        }
    }
    cout << "Light not found in lights" << endl;
    return lights.end();
}


void P3Geometry::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(with_id("Geometry ").c_str())) {
        if (ImGui::Button(with_id("Sphere").c_str())) {
            auto iter = GetIter();
            P3Geometry* old = *iter;
            *iter = new P3Sphere(id);
            delete old;
        }
        if (ImGui::Button(with_id("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void P3Sphere::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(with_id("Sphere ").c_str())) {
        ImGui::InputFloat3(with_id("pos##").c_str(), pos);
        ImGui::SliderFloat(with_id("radius##").c_str(), &rad, 0.001f, 10.0f);
        ImGui::Indent(3.0f);
        mat->ImGui();
        ImGui::Unindent(3.0f);
        if (ImGui::Button(with_id("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}


void P3Material::ImGui() {
    if (ImGui::CollapsingHeader(with_id("Material ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::SliderFloat3(with_id("ambient##").c_str(), ambient, 0.0f, 1.0f);
        ImGui::SliderFloat3(with_id("diffuse##").c_str(), diffuse, 0.0f, 1.0f);
        ImGui::SliderFloat3(with_id("specular##").c_str(), specular, 0.0f, 1.0f);
        ImGui::SliderFloat3(with_id("transmissive##").c_str(), transmissive, 0.0f, 1.0f);
        ImGui::SliderFloat(with_id("phong##").c_str(), &phong, 0.0f, 10.0f);
        ImGui::SliderFloat(with_id("ior##").c_str(), &ior, 0.0f, 1.0f);
        ImGui::Unindent(4.0f);
    }
}


void P3Light::ImGui() {
    if (ImGui::CollapsingHeader(with_id("Light ").c_str())) {
        ImGui::Indent(4.0f);
        if (ImGui::Button(with_id("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}


void P3AmbientLight::ImGui() {
    if (ImGui::CollapsingHeader(with_id("Ambient ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::ColorPicker3(with_id("color##").c_str(), col);
        if (ImGui::Button(with_id("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}

void P3PointLight::ImGui() {
    if (ImGui::CollapsingHeader(with_id("Point ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::InputFloat3(with_id("pos##").c_str(), pos);
        ImGui::ColorPicker3(with_id("color##").c_str(), col);
        if (ImGui::Button(with_id("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}

void P3SpotLight::ImGui() {
    if (ImGui::CollapsingHeader(with_id("Spot ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::ColorPicker3(with_id("color##").c_str(), col);
        if (ImGui::Button(with_id("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}

void P3DirectionalLight::ImGui() {
    if (ImGui::CollapsingHeader(with_id("Directional ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::ColorPicker3(with_id("color##").c_str(), col);
        if (ImGui::Button(with_id("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}


P3Material::P3Material() {
    id = entity_count;
    entity_count++;
}

void P3Material::Decode(string s) {
    stringstream ss(s);
    ss >>   ambient[0]      >> ambient[1]       >> ambient[2] >>
            diffuse[0]      >> diffuse[1]       >> diffuse[2] >>
            specular[0]     >> specular[1]      >> specular[2]      >> phong >>
            transmissive[0] >> transmissive[1]  >> transmissive[2]  >> ior;
}

void P3Geometry::Decode(string s) {
}


void P3Sphere::Decode(string s) {
    stringstream ss(s);
    ss >> pos[0] >> pos[1] >> pos[2] >> rad;
}

void P3Light::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2];
}

void P3AmbientLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2];
}

void P3PointLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2] >> pos[0] >> pos[1] >> pos[2];
}

void P3DirectionalLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2] >> dir[0] >> dir[1] >> dir[2];
}

void P3SpotLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2] >> pos[0] >> pos[1] >> pos[2] >> dir[0] >> dir[1] >> dir[2] >> angle1 >> angle2;
}


string P3Material::Encode() {
    char temp[128];
    sprintf(temp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f",
        ambient[0], ambient[1], ambient[2],
        diffuse[0], diffuse[1], diffuse[2],
        specular[0], specular[1], specular[2], phong,
        transmissive[0], transmissive[1], transmissive[2], ior);

    return string(temp);
}

string P3Geometry::Encode() {
    char temp[128];
    sprintf(temp, "material: %s\n", mat->Encode());

    return string(temp);
}

string P3Sphere::Encode() {
    char temp[128];
    sprintf(temp, "sphere: %f %f %f %f", pos[0], pos[1], pos[2], rad);

    return P3Geometry::Encode() + string(temp);
}

string P3Light::Encode() {
    char temp[128];
    sprintf(temp, "%f %f %f", col[0], col[1], col[2]);
    
    return string(temp);
}

string P3AmbientLight::Encode() {
    char temp[128];
    sprintf(temp, "ambient_light: %s", P3Light::Encode());
    
    return string(temp);
}

string P3DirectionalLight::Encode() {
    char temp[128];
    sprintf(temp, "directional_light: %s %f %f %f", P3Light::Encode(), dir[0], dir[1], dir[2]);

    return string(temp);
}

string P3PointLight::Encode() {
    char temp[128];
    sprintf(temp, "point_light: %s %f %f %f", P3Light::Encode(), pos[0], pos[1], pos[2]);

    return string(temp);
}

string P3SpotLight::Encode() {
    char temp[128];
    sprintf(temp, "spot_light: %s %f %f %f %f %f %f %f %f", P3Light::Encode(), pos[0], pos[1], pos[2], dir[0], dir[1], dir[2], angle1, angle2);

    return string(temp);
}


void Reset() {
    shapes.clear();
    lights.clear();
    materials.clear();
    entity_count = 0;
    materials.push_back(new P3Material());
    std::fill(cp, cp + 3, 0.0f);
    std::fill(cf, cf + 3, 0.0f);
    cf[2] = 1.0f;
    std::fill(cu, cu + 3, 0.0f);
    cu[1] = 1.0f;
    fov_ha = 45.0f;
    window_res[0] = 640;
    window_res[1] = 480;
    strcpy(output_name, "");
}


void Load() {
    Reset();
    string scene_string = "scenes/" + string(scene_name);
    string end_string = ".p3";
    if (scene_string == "") { return; }
    if (scene_string.length() < end_string.length()) { return; }
    if (scene_string.compare(scene_string.length() - end_string.length(),
        end_string.length(), end_string) != 0) {
        return;
    }

    ifstream scene_file(scene_string);
    if (!scene_file.is_open()) { return; }

    string line;
    while (getline(scene_file, line)) {
        string rest;
        rest = rest_if_prefix("sphere: ", line);
        if (rest != "") {
            P3Sphere* new_sphere = new P3Sphere();
            new_sphere->Decode(rest);
            shapes.push_back(new_sphere);
        }

        rest = rest_if_prefix("material: ", line);
        if (rest != "") {
            P3Material* new_mat = new P3Material();
            new_mat->Decode(rest);
            materials.push_back(new_mat);
        }

        rest = rest_if_prefix("ambient_light: ", line);
        if (rest != "") {
            P3AmbientLight* new_light = new P3AmbientLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("directional_light: ", line);
        if (rest != "") {
            P3DirectionalLight* new_light = new P3DirectionalLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("point_light: ", line);
        if (rest != "") {
            P3PointLight* new_light = new P3PointLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("spot_light: ", line);
        if (rest != "") {
            P3SpotLight* new_light = new P3SpotLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }
    }
}


void Save() {
    if (string(scene_name) == "") { return; }
}

void Render() {

}


// Main code
int main(int argc, char** argv) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Project 3, Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    int err = gladLoadGL();
    if (!err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return err;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();


        ImGui::Begin("Renderer Settings");

        if (ImGui::Button("Render", ImVec2(ImGui::GetWindowWidth(), 0))) {
            Render();
        }

        if (ImGui::Button("Save")) {
            Save();
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            Load();
        }
        ImGui::SameLine();
        ImGui::InputTextWithHint("", "file.p3", scene_name, 128, ImGuiInputTextFlags_CharsNoBlank);
        ImGui::InputTextWithHint("Output Name", "output.png", output_name, 128, ImGuiInputTextFlags_CharsNoBlank);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.7, 0.7, 1.0, 1.0 });
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4{ 0.4, 0.4, 0.5, 1.0 });
        if (ImGui::CollapsingHeader("Camera Parameters")) {
            ImGui::InputFloat3("Camera Position", cp);
            ImGui::SliderFloat3("Camera Forward", cf, -1.0f, 1.0f);
            ImGui::SliderFloat3("Camera Up", cu, -1.0f, 1.0f);
            ImGui::SliderFloat("FOV", &fov_ha, 0.0f, 90.0f);
            ImGui::InputInt2("Resolution", window_res, 1);
        }
        ImGui::PopStyleColor(2);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0, 0.7, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Geometry")) {
            for (P3Geometry* shape : shapes) {
                shape->ImGui();
            }
            if (ImGui::Button("New Sphere", ImVec2(ImGui::GetWindowWidth(), 0))) {
                shapes.push_back(new P3Sphere());
            }
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0, 1.0, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Lighting")) {
            float spacing = 4.0f;
            for (P3Light* light : lights) {
                light->ImGui();
            }
            if (ImGui::Button("New Ambient", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new P3AmbientLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Point", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new P3PointLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Spot", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new P3SpotLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Directional", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new P3DirectionalLight());
            }
        }
        ImGui::PopStyleColor();


        ImGui::End();

        // Rendering
        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    for (P3Geometry* geo : shapes) {
        delete geo;
    }

    for (P3Light* light : lights) {
        delete light;
    }

    for (P3Material* mat : materials) {
        delete mat;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
