// Main templated from Dear ImGui

//#define _USE_MATH_DEFINES
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "image_lib.h"
#include "main.h"


namespace P3 {

// GLOBAL STATE
vector<UIGeometry*> shapes{};
vector<UILight*> lights{};
vector<UIMaterial*> materials{ new UIMaterial() };
UICamera* camera = new UICamera();
int entity_count = 0;
char scene_name[256] = "";

// OTHER SETTINGS
char output_name[256] = "raytraced.bmp";


string rest_if_prefix(const string prefix, string content) {
    if (content.compare(0, prefix.length(), prefix) == 0) {
        return content.substr(prefix.length());
    }
    else {
        return "";
    }
}


UIObject::UIObject() {
    id = entity_count;
    entity_count++;
}

UIObject::UIObject(int old_id) {
    id = old_id;
}


string UIObject::WithId(string s) {
    return (s + std::to_string(id)).c_str();
}


// Call without index to create new shapes
UIGeometry::UIGeometry() : UIObject() {
    mat = materials.back();
}
// Call with index to replace shapes (use old id)
UIGeometry::UIGeometry(int old_id) : UIObject(old_id) {
    mat = materials.back();
}


void UIGeometry::Delete() { shapes.erase(GetIter()); }
void UILight::Delete() { lights.erase(GetIter()); }


vector<UIGeometry*>::iterator UIGeometry::GetIter() {
    for (vector<UIGeometry*>::iterator it = shapes.begin(); it < shapes.end(); it++) {
        if ((*it)->id == id) {
            return it;
        }
    }
    cout << "Geometry not found in shapes" << endl;
    return shapes.end();
}


vector<UILight*>::iterator UILight::GetIter() {
    for (vector<UILight*>::iterator it = lights.begin(); it < lights.end(); it++) {
        if ((*it)->id == id) {
            return it;
        }
    }
    cout << "Light not found in lights" << endl;
    return lights.end();
}


void UICamera::ImGui() {
    if (ImGui::CollapsingHeader("Camera Parameters")) {
        ImGui::InputFloat3("Camera Position", cp);
        ImGui::SliderFloat3("Camera Forward", cf, -1.0f, 1.0f);
        ImGui::SliderFloat3("Camera Up", cu, -1.0f, 1.0f);
        ImGui::SliderFloat("FOV", &fov_ha, 0.0f, 90.0f);
        ImGui::InputInt2("Resolution", window_res, 1);
        ImGui::SliderInt("Max Depth", &max_depth, 1, 20);
        ImGui::ColorPicker3("Background Color", bc);
    }
}

void UIGeometry::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Geometry ").c_str())) {
        if (ImGui::Button(WithId("Sphere").c_str())) {
            auto iter = GetIter();
            UIGeometry* old = *iter;
            *iter = new UISphere(id);
            delete old;
        }
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void UISphere::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Sphere ").c_str())) {
        ImGui::InputFloat3(WithId("pos##").c_str(), pos);
        ImGui::SliderFloat(WithId("radius##").c_str(), &rad, 0.001f, 10.0f);
        ImGui::Indent(3.0f);
        mat->ImGui();
        ImGui::Unindent(3.0f);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}


void UIMaterial::ImGui() {
    // TODO: "Make Unique" button
    if (ImGui::CollapsingHeader(WithId("Material ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::SliderFloat3(WithId("ambient##").c_str(), ambient, 0.0f, 1.0f);
        ImGui::SliderFloat3(WithId("diffuse##").c_str(), diffuse, 0.0f, 1.0f);
        ImGui::SliderFloat3(WithId("specular##").c_str(), specular, 0.0f, 1.0f);
        ImGui::SliderFloat3(WithId("transmissive##").c_str(), transmissive, 0.0f, 1.0f);
        ImGui::SliderFloat(WithId("phong##").c_str(), &phong, 0.0f, 10.0f);
        ImGui::SliderFloat(WithId("ior##").c_str(), &ior, 0.0f, 1.0f);
        ImGui::Unindent(4.0f);
    }
}


void UILight::ImGui() {
    if (ImGui::CollapsingHeader(WithId("Light ").c_str())) {
        ImGui::Indent(4.0f);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}


void UIAmbientLight::ImGui() {
    if (ImGui::CollapsingHeader(WithId("Ambient ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}

void UIPointLight::ImGui() {
    if (ImGui::CollapsingHeader(WithId("Point ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::InputFloat3(WithId("pos##").c_str(), pos);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}

void UISpotLight::ImGui() {
    if (ImGui::CollapsingHeader(WithId("Spot ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}

void UIDirectionalLight::ImGui() {
    if (ImGui::CollapsingHeader(WithId("Directional ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
        ImGui::Unindent(4.0f);
    }
}


void UICamera::Decode(string s) {
    string rest;
    rest = rest_if_prefix("camera_pos: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> cp[0] >> cp[1] >> cp[2];
    }

    rest = rest_if_prefix("camera_fwd: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> cf[0] >> cf[1] >> cf[2];
    }

    rest = rest_if_prefix("camera_up: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> cu[0] >> cu[1] >> cu[2];
    }

    rest = rest_if_prefix("camera_fov_ha: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> fov_ha;
    }

    rest = rest_if_prefix("film_resolution: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> window_res[0] >> window_res[1];
    }

    rest = rest_if_prefix("output_image: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> output_name;
    }

    rest = rest_if_prefix("background: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> bc[0] >> bc[1] >> bc[2];
    }

    rest = rest_if_prefix("max_depth: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> max_depth;
    }
}


void UIMaterial::Decode(string s) {
    stringstream ss(s);
    ss >> ambient[0] >> ambient[1] >> ambient[2] >>
        diffuse[0] >> diffuse[1] >> diffuse[2] >>
        specular[0] >> specular[1] >> specular[2] >> phong >>
        transmissive[0] >> transmissive[1] >> transmissive[2] >> ior;
}

void UIGeometry::Decode(string s) {
}


void UISphere::Decode(string s) {
    stringstream ss(s);
    ss >> pos[0] >> pos[1] >> pos[2] >> rad;
}

void UILight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2];
}

void UIAmbientLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2];
}

void UIPointLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2] >> pos[0] >> pos[1] >> pos[2];
}

void UIDirectionalLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2] >> dir[0] >> dir[1] >> dir[2];
}

void UISpotLight::Decode(string s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2] >> pos[0] >> pos[1] >> pos[2] >> dir[0] >> dir[1] >> dir[2] >> angle1 >> angle2;
}


string UICamera::Encode() {
    stringstream ss;

    ss << "camera_pos: " << cp[0] << " " << cp[1] << " " << cp[2] << " " << endl;
    ss << "camera_fwd: " << cf[0] << " " << cf[1] << " " << cf[2] << " " << endl;
    ss << "camera_up: " << cu[0] << " " << cu[1] << " " << cu[2] << " " << endl;
    ss << "camera_fov_ha: " << fov_ha << endl;
    ss << "film_resolution: " << window_res[0] << " " << window_res[1] << endl;
    ss << "output_image: " << output_name << endl;
    ss << "background: " << bc[0] << " " << bc[1] << " " << bc[2] << " " << endl;
    ss << "max_depth: " << max_depth << endl;

    return ss.str();
}


string UIMaterial::Encode() {
    char temp[256];
    sprintf(temp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f",
        ambient[0], ambient[1], ambient[2],
        diffuse[0], diffuse[1], diffuse[2],
        specular[0], specular[1], specular[2], phong,
        transmissive[0], transmissive[1], transmissive[2], ior);

    return string(temp);
}

string UIGeometry::Encode() {
    return string("material: ") + mat->Encode() + string("\n");
}

string UISphere::Encode() {
    char temp[256];
    sprintf(temp, "sphere: %f %f %f %f", pos[0], pos[1], pos[2], rad);

    return UIGeometry::Encode() + string(temp);
}

string UILight::Encode() {
    char temp[256];
    sprintf(temp, "%f %f %f", col[0], col[1], col[2]);

    return string(temp);
}

string UIAmbientLight::Encode() {
    stringstream ss;
    ss << "ambient_light: " << UILight::Encode();

    return ss.str();
}

string UIDirectionalLight::Encode() {
    stringstream ss;
    ss << "directional_light: " << UILight::Encode() << " " << dir[0] << " " << dir[1] << " " << dir[2];

    return ss.str();
}

string UIPointLight::Encode() {
    stringstream ss;
    ss << "point_light: " << UILight::Encode() << " " << pos[0] << " " << pos[1] << " " << pos[2];

    return ss.str();
}

string UISpotLight::Encode() {
    stringstream ss;
    ss << "spot_light: " << UILight::Encode() << " " << pos[0] << " " << pos[1] << " " << pos[2] << dir[0] << " " << dir[1] << " " << dir[2] << " " << angle1 << " " << angle2;

    return ss.str();
}


void Reset() {
    delete camera;
    for (UIGeometry* o : shapes) {
        delete o;
    }
    for (UILight* o : lights) {
        delete o;
    }
    for (UIMaterial* o : materials) {
        delete o;
    }
    shapes.clear();
    lights.clear();
    materials.clear();

    strcpy(output_name, "");

    entity_count = 0;
    materials.push_back(new UIMaterial());
    camera = new UICamera();
}


void Load() {
    Reset();

    if (string(scene_name) == "") {
        return;
    }

    string scene_string = "scenes/" + string(scene_name) + ".p3";



    ifstream scene_file(scene_string);
    if (!scene_file.is_open()) { return; }

    string line;
    while (getline(scene_file, line)) {
        string rest;

        camera->Decode(line);

        rest = rest_if_prefix("sphere: ", line);
        if (rest != "") {
            UISphere* new_sphere = new UISphere();
            new_sphere->Decode(rest);
            shapes.push_back(new_sphere);
        }

        rest = rest_if_prefix("material: ", line);
        if (rest != "") {
            UIMaterial* new_mat = new UIMaterial();
            new_mat->Decode(rest);
            materials.push_back(new_mat);
        }

        rest = rest_if_prefix("ambient_light: ", line);
        if (rest != "") {
            UIAmbientLight* new_light = new UIAmbientLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("directional_light: ", line);
        if (rest != "") {
            UIDirectionalLight* new_light = new UIDirectionalLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("point_light: ", line);
        if (rest != "") {
            UIPointLight* new_light = new UIPointLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("spot_light: ", line);
        if (rest != "") {
            UISpotLight* new_light = new UISpotLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }
    }
    scene_file.close();
}


void Save() {
    if (string(scene_name) == "") {
        return;
    }

    string scene_string = "scenes/" + string(scene_name) + ".p3";

    ofstream scene_file(scene_string);
    if (!scene_file.is_open()) { return; }

    scene_file << camera->Encode() << endl;

    for (UIGeometry* geo : shapes) {
        scene_file << geo->Encode() << endl;
    }

    for (UILight* light : lights) {
        scene_file << light->Encode() << endl;
    }

    scene_file.close();
}


void Render() {
    Point3D eye = Point3D(camera->cp[0], camera->cp[1], camera->cp[2]);
    Dir3D forward = Dir3D(camera->cf[0], camera->cf[1], camera->cf[2]).normalized();
    Dir3D up = Dir3D(cu[0], cu[1], cu[2]);
    Dir3D right = cross(up, forward).normalized();
    up = cross(forward, up).normalized();

    int imgW = window_res[0];
    int imgH = window_res[1];
    float halfW = imgW / 2.0, halfH = imgH / 2.0;
    float d = halfH / tanf(fov_ha * (M_PI / 180.0f));

    Image outputImg = Image(imgW, imgH);
    for (int i = 0; i < imgW; i++) {
        for (int j = 0; j < imgH; j++) {
            float u = (halfW - (window_res[0]) * ((i + 0.5) / imgW));
            float v = (halfH - (window_res[1]) * ((j + 0.5) / imgH));
            Point3D p = eye - d * forward + u * right + v * up;
            Dir3D rayDir = (p - eye);
            Line3D rayLine = vee(eye, rayDir).normalized();
            // intersection = FindIntersection(eye, rayLine, scene);
            // diff = intersection - sphere_center
            // sphere_normal = diff.normalized()
            //outputImg.setPixel(i, j, color);
        }
    }

    outputImg.write(output_name);
}


// From Dear ImGui wiki
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height) {
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}
}

using namespace P3;

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
    SDL_Window* window = SDL_CreateWindow("Project 3, Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2560, 720, window_flags);
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
        ImGui::InputTextWithHint("", "<scene filename>", scene_name, 256, ImGuiInputTextFlags_CharsNoBlank);
        ImGui::InputTextWithHint("Output Name", "output.png", output_name, 256, ImGuiInputTextFlags_CharsNoBlank);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.7, 0.7, 1.0, 1.0 });
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4{ 0.4, 0.4, 0.5, 1.0 });
        camera->ImGui();
        ImGui::PopStyleColor(2);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0, 0.7, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Geometry")) {
            for (UIGeometry* shape : shapes) {
                shape->ImGui();
            }
            if (ImGui::Button("New Sphere", ImVec2(ImGui::GetWindowWidth(), 0))) {
                shapes.push_back(new UISphere());
            }
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0, 1.0, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Lighting")) {
            float spacing = 4.0f;
            for (UILight* light : lights) {
                light->ImGui();
            }
            if (ImGui::Button("New Ambient", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new UIAmbientLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Point", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new UIPointLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Spot", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new UISpotLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Directional", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new UIDirectionalLight());
            }
        }
        ImGui::PopStyleColor();


        ImGui::Begin("Output");
        int im_h = 0, im_w = 0;
        GLuint im_tex = 0;

        bool ret = LoadTextureFromFile("smile.png", &im_tex, &im_w, &im_h);
        IM_ASSERT(ret);

        ImGui::Image((void*)(intptr_t)im_tex, ImVec2(im_w, im_h));
        ImGui::End();



        ImGui::End();

        // Rendering
        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    for (UIGeometry* geo : shapes) {
        delete geo;
    }

    for (UILight* light : lights) {
        delete light;
    }

    for (UIMaterial* mat : materials) {
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
