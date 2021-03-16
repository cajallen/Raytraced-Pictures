// Main templated from Dear ImGui
#include "raytracer_main.h"

// any SAMPLING > 0 is randomly sampled.
#define AA_RANDOM 4
#define AA_NONE 0
#define AA_FIVE -1

#define SAMPLING AA_NONE

inline float randf() {
    return rand() / (float)RAND_MAX;
}

namespace P3 {

// UI STATE
vector<Geometry*> shapes{};
vector<Light*> lights{};
vector<AmbientLight*> ambient_lights{};
vector<Material*> materials{new Material()};
Camera* camera = new Camera();
int entity_count = 0;
char scene_name[256] = "";
char output_name[256] = "raytraced.bmp";

ImVec2 disp_img_size{0.0, 0.0};
GLuint disp_img_tex = -1;


Ray Reflect(Dir3D ang, Point3D pos, Dir3D norm, int bounces_left) {
    Line3D normal_dir = vee(Point3D(0, 0, 0), norm).normalized();
    Point3D midpoint = proj(Point3D(ang.x, ang.y, ang.z), normal_dir);
    Point3D diff = midpoint - ang;
    Point3D to_out = (midpoint + diff).normalized();

    Point3D from = pos + norm * 0.001;

    return Ray(from, Dir3D(to_out.x, to_out.y, to_out.z), bounces_left);
}

// Call without index to create new shapes
Object::Object() {
    id = entity_count;
    entity_count++;
}

// Call with index to replace shapes (use old id)
Object::Object(int old_id) {
    id = old_id;
}

Geometry::Geometry() : Object() {
    material = materials.back();
}
Geometry::Geometry(int old_id) : Object(old_id) {
    material = materials.back();
}

vector<Geometry*>::iterator Geometry::GetIter() {
    for (vector<Geometry*>::iterator it = shapes.begin(); it < shapes.end(); it++) {
        if ((*it)->id == id) {
            return it;
        }
    }
    cout << "Geometry not found in shapes" << endl;
    return shapes.end();
}
vector<Light*>::iterator Light::GetIter() {
    for (vector<Light*>::iterator it = lights.begin(); it < lights.end(); it++) {
        if ((*it)->id == id) {
            return it;
        }
    }
    cout << "Light not found in lights" << endl;
    return lights.end();
}

void Geometry::Delete() {
    shapes.erase(GetIter());
}
void Light::Delete() {
    lights.erase(GetIter());
}

void Light::UpdateMult() {
    color = color * mult;
    mult = 1;
}

void Light::ClampColor() {
    if (color.r > 1 || color.g > 1 || color.b > 1) {
        float max_col = max(color.r, max(color.g, color.b));
        mult = max_col;
        color.r /= mult;
        color.g /= mult;
        color.b /= mult;
    }
}

bool Sphere::FindIntersection(Ray ray, HitInformation* intersection) {
    Dir3D toStart = (ray.pos - position);
    float b = 2 * dot(ray.dir, toStart);
    float c = dot(toStart, toStart) - pow(radius, 2);
    float discr = pow(b, 2) - 4 * c;

    if (discr < 0)
        return false;

    float t0 = (-b + sqrt(discr)) / 2;
    float t1 = (-b - sqrt(discr)) / 2;
    float mint = t0;
    if (t1 > 0.0 && t1 < t0)
        mint = t1;

    if (mint < 0.0)
        return false;

    Point3D hit_pos = ray.pos + mint * ray.dir;
    Dir3D hit_norm = (hit_pos - position).normalized();
    *intersection = HitInformation{mint, hit_pos, ray.dir, hit_norm, material};

    return true;
}

Ray DirectionalLight::ReverseLightRay(Point3D from) {
    return Ray{from, -direction, -1};
}

Ray PointLight::ReverseLightRay(Point3D from) {
    Dir3D offset = position - from;

    return Ray{from, offset.normalized(), -1};
}

Ray SpotLight::ReverseLightRay(Point3D from) {
    Dir3D offset = position - from;

    return Ray{from, offset.normalized(), -1};
}

float DirectionalLight::DistanceTo(Point3D to) {
    return INFINITY;
}

float PointLight::DistanceTo(Point3D to) {
    Dir3D offset = to - position;
    return offset.magnitude();
}

float SpotLight::DistanceTo(Point3D to) {
    Dir3D offset = to - position;
    return offset.magnitude();
}

Color DirectionalLight::Intensity(Point3D to) {
    return color;
}

Color PointLight::Intensity(Point3D to) {
    float dist2 = pow(DistanceTo(to), 2);
    float r = color.r / dist2;
    float g = color.g / dist2;
    float b = color.b / dist2;

    return Color(r, g, b);
}

Color SpotLight::Intensity(Point3D to) {
    Dir3D angle_to = (to - position).normalized();
    float diff = 180.0 * acos(dot(angle_to, direction)) / M_PI;
    float dist2 = pow(DistanceTo(to), 2);

    if (diff < angle1)
        return color * (1 / dist2);
    float amount = 1 - ((diff - angle1) / (angle2 - angle1));
    if (diff < angle2)
        return color * amount * (1 / dist2);

    return Color(0, 0, 0);
}

void Reset() {
    delete camera;
    for (Geometry* o : shapes) {
        delete o;
    }
    for (Light* o : lights) {
        delete o;
    }
    for (Material* o : materials) {
        delete o;
    }
    shapes.clear();
    lights.clear();
    ambient_lights.clear();
    materials.clear();

    strcpy(output_name, "");

    entity_count = 0;
    materials.push_back(new Material());
    camera = new Camera();
}


Color ApplyLighting(Ray ray, HitInformation hit_info) {
    Color current(0, 0, 0);

    for (Light* light : lights) {
        if (light->Intensity(hit_info.pos) == Color(0, 0, 0))
            continue;

        Ray to_light = light->ReverseLightRay(hit_info.pos);
        to_light.pos = to_light.pos + hit_info.normal * 0.0001;
        HitInformation light_intersection;
        // If light is blocked
        if (FindIntersection(shapes, to_light, &light_intersection) &&
            light_intersection.dist < light->DistanceTo(hit_info.pos))
            continue;

        current = current + CalculateDiffuse(light, hit_info);
        current = current + CalculateSpecular(light, hit_info);
    }
    Ray reflected = Reflect(-hit_info.viewing, hit_info.pos, hit_info.normal, ray.bounces_left - 1);
    current = current + hit_info.material->specular * EvaluateRay(reflected);
    current = current + CalculateAmbient(hit_info);

    return current;
}

Color EvaluateRay(Ray ray) {
    if (ray.bounces_left <= 0)
        return camera->background_color;

    HitInformation hit_info;
    if (FindIntersection(shapes, ray, &hit_info)) {
        return ApplyLighting(ray, hit_info);
    } else {
        return camera->background_color;
    }
}

Color CalculateDiffuse(Light* light, HitInformation hit) {
    Color il = light->Intensity(hit.pos);
    Dir3D to_light = light->ReverseLightRay(hit.pos).dir;
    float amount = max(0, dot(hit.normal, to_light));
    return hit.material->diffuse * il * amount;
}

Color CalculateSpecular(Light* light, HitInformation hit) {
    Dir3D to_light = light->ReverseLightRay(hit.pos).dir;
    Dir3D to_viewer = -hit.viewing;
    Ray test = Reflect(to_light, hit.pos, hit.normal, -1);
    float amount = pow(max(0, dot(test.dir, to_viewer)), hit.material->phong);

    Color il = light->Intensity(hit.pos);

    return hit.material->specular * il * amount;
}

Color CalculateAmbient(HitInformation hit) {
    Color c = Color(0, 0, 0);

    for (AmbientLight* al : ambient_lights) {
        c = c + hit.material->ambient * al->color;
    }

    return c;
}

void Render() {
    float d = camera->mid_res[Y] / tanf(camera->half_vfov * (M_PI / 180.0f));

    for (Light* light : lights) {
        light->UpdateMult();
        AmbientLight* al = dynamic_cast<AmbientLight*>(light);
        if (al != NULL) {
            ambient_lights.push_back(al);
        }
    }

    Image outputImg = Image(camera->res[X], camera->res[Y]);

#pragma omp parallel for num_threads(16) schedule(dynamic, camera->res[X])
    for (int i = 0; i < camera->res[X] * camera->res[Y]; i++) {
        int x = i % camera->res[X];
        int y = i / camera->res[X];
        vector<ImVec2> offsets;
#if SAMPLING == -1
        offsets = {ImVec2(0.50, 0.50), ImVec2(0.15, 0.15), ImVec2(0.85, 0.15), ImVec2(0.85, 0.85),
                    ImVec2(0.15, 0.85)};
#elif SAMPLING == 0
        offsets = {ImVec2(0.5, 0.5)};
#else
        for (int samp_i = 0; samp_i < SAMPLING; samp_i++)
            offsets.push_back(ImVec2(randf(), randf()));
#endif
        Color col = Color(0, 0, 0);
        for (ImVec2 offset : offsets) {
            float u = camera->mid_res[X] - x + offset.x;
            float v = camera->mid_res[Y] - y + offset.y;

            Dir3D rayDir = (-d * camera->forward + u * camera->right + v * camera->up).normalized();

            Ray ray = Ray(camera->position, rayDir, camera->max_depth);
            Color new_color = EvaluateRay(ray);
            new_color.Clamp();
            col = col + new_color * (1.0 / offsets.size());
        }
        outputImg.setPixel(x, y, col);
    }

    ambient_lights.clear();

    for (Light* light : lights) {
        light->ClampColor();
    }

    // TODO: Instead of displaying image, write to a buffer.
    string relative_output_name = "output/" + string(output_name);
    outputImg.write(relative_output_name.c_str());
}

bool FindIntersection(vector<Geometry*> geometry, Ray ray, HitInformation* intersection) {
    HitInformation current_inter;
    float dist = -1.0;
    for (Geometry* geo : geometry) {
        if (geo->FindIntersection(ray, &current_inter)) {
            if (dist == -1.0 || current_inter.dist < dist) {
                *intersection = current_inter;
                dist = current_inter.dist;
            }
        }
    }
    return dist != -1.0;
}

void DisplayImage(string name) {
    int im_x, im_y;
    bool ret = LoadTextureFromFile(name.c_str(), &disp_img_tex, &im_x, &im_y);
    disp_img_size = ImVec2(im_x, im_y);
    IM_ASSERT(ret);
}

}  //  namespace P3

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
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Project 3, Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280,
                                          720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);  // Enable vsync

    // Initialize OpenGL loader
    int err = gladLoadGL();
    if (!err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return err;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    future<void> render_call;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::Begin("Renderer Settings");


        if (ImGui::Button("Render", ImVec2(ImGui::GetWindowWidth(), 0))) {
            render_call = async(Render);
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

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0, 0.7, 0.7, 1.0 });
        if (ImGui::CollapsingHeader("Geometry")) {
            for (Geometry* shape : shapes) {
                shape->ImGui();
            }
            if (ImGui::Button("New Sphere", ImVec2(ImGui::GetWindowWidth(), 0))) {
                Material* mat = new Material();
                materials.push_back(mat);
                shapes.push_back(new Sphere());
            }
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0, 1.0, 0.7, 1.0 });
        if (ImGui::CollapsingHeader("Lighting")) {
            float spacing = 4.0f;
            for (Light* light : lights) {
                light->ImGui();
            }
            if (ImGui::Button("New Ambient", ImVec2(ImGui::GetWindowWidth() / 4 - spacing * 2, 0))) {
                lights.push_back(new AmbientLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Point", ImVec2(ImGui::GetWindowWidth() / 4 - spacing * 2, 0))) {
                lights.push_back(new PointLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Spot", ImVec2(ImGui::GetWindowWidth() / 4 - spacing * 2, 0))) {
                lights.push_back(new SpotLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Directional", ImVec2(ImGui::GetWindowWidth() / 4 - spacing * 2, 0))) {
                lights.push_back(new DirectionalLight());
            }
        }
        ImGui::PopStyleColor();

        if (render_call.valid()) {
            render_call.get(); // Calling get makes the render_call invalid, storing that we used it.
            string relative_output_name = "output/" + string(output_name);
            if (_access(relative_output_name.c_str(), 0) != -1)
                DisplayImage(relative_output_name);
        }

        if (disp_img_tex != -1) {
            ImGui::Begin("Output");
            ImGui::Image((void*)(intptr_t)disp_img_tex, disp_img_size);
            ImGui::End();
        }

        ImGui::End();

        // Rendering
        ImGui::Render();


        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    for (Geometry* geo : shapes) {
        delete geo;
    }

    for (Light* light : lights) {
        delete light;
    }

    for (Material* mat : materials) {
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
