// Main templated from Dear ImGui
#include "raytracer_main.h"

inline float randf() {
    return rand() / (float)RAND_MAX;
}

namespace Raytracer {

// UI STATE
int entity_count = 0;
char scene_name[256] = "";
char output_name[256] = "raytraced.bmp";
vector<Material*> materials{new Material(&entity_count)};
Camera* camera = new Camera(&entity_count);
vector<Geometry*> shapes{};
vector<Light*> lights{};
vector<AmbientLight*> ambient_lights{};
steady_clock::time_point last_request;
bool update_automatically = false;
vector<string> debug_log{};

ImVec2 disp_img_size{0.0, 0.0};
GLuint disp_img_tex = -1;

ImVec2 operator - (ImVec2& lhs, ImVec2& rhs) {
    return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

ImVec2 operator + (ImVec2& lhs, ImVec2& rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

vector<Geometry*>::iterator GetIter(Geometry* geo) {
    for (vector<Geometry*>::iterator it = shapes.begin(); it < shapes.end(); it++) {
        if ((*it)->id == geo->id) {
            return it;
        }
    }
    return shapes.end();
}

vector<Light*>::iterator GetIter(Light* light) {
    for (vector<Light*>::iterator it = lights.begin(); it < lights.end(); it++) {
        if ((*it)->id == light->id) {
            return it;
        }
    }
    return lights.end();
}

void Delete(Geometry* geo) {
	shapes.erase(GetIter(geo));
}

void Delete(Light* light) {
	lights.erase(GetIter(light));
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
    load_state = LoadState{};

    strcpy(output_name, "raytraced.bmp");

    entity_count = 0;
    materials.push_back(new Material(&entity_count));
    camera = new Camera(&entity_count);
}

Color ApplyLighting(Ray ray, HitInformation hit_info) {
    Color current(0, 0, 0);

    for (Light* light : lights) {
        if (light->Intensity(hit_info.pos) < Color(0.001, 0.001, 0.001))
            continue;

        Ray to_light = light->ReverseLightRay(hit_info.pos);
        HitInformation light_intersection;
        // If light is blocked
        if (FindIntersection(shapes, to_light, &light_intersection) &&
            light_intersection.dist < sqrt(light->DistanceTo2(hit_info.pos)))
            continue;

        Color diffuse = CalculateDiffuse(light, hit_info);
        current = current + diffuse;

        Color specular = CalculateSpecular(light, hit_info);
        current = current + specular;
    }
    Ray reflected = Ray::Reflect(-hit_info.viewing, hit_info.pos, hit_info.normal, ray.bounces_left - 1);
	reflected.last_material = hit_info.material;
    Color refl_col =  hit_info.material->specular * EvaluateRay(reflected);
    current = current + refl_col;


    float next_ior = hit_info.material->ior;
    Color t = hit_info.material->transmissive;
    if (t.r + t.g + t.b > 0) {
        Ray refracted = Ray::Refract(hit_info.viewing, hit_info.pos, hit_info.normal, next_ior, ray.bounces_left - 1);
        refracted.last_material = hit_info.material;
        if (refracted.bounces_left != -1)
            current = current + hit_info.material->transmissive * EvaluateRay(refracted);
    }

    current = current + CalculateAmbient(hit_info);
    IM_ASSERT(!isnan(current.r) && !isnan(current.g) && !isnan(current.b));
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
    vec3 to_light = light->ReverseLightRay(hit.pos).dir;
    float amount = max(0, dot(hit.normal, to_light));
    return hit.material->diffuse * il * amount;
}

Color CalculateSpecular(Light* light, HitInformation hit) {
    vec3 to_light = light->ReverseLightRay(hit.pos).dir;
    vec3 to_viewer = -hit.viewing;
    Ray test = Ray::Reflect(to_light, hit.pos, hit.normal, -1);
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


float PreRender() {
    camera->PreRender();
    for (Geometry* geo : shapes) geo->PreRender();
    for (Light* light : lights) light->PreRender();
    
    for (Light* light : lights) {
        light->UpdateMult();
        AmbientLight* al = dynamic_cast<AmbientLight*>(light);
        if (al != NULL) {
            ambient_lights.push_back(al);
        }
    }

    return camera->mid_res.y / tanf(camera->half_vfov * (M_PI / 180.0f));
}

void PostRender() {
    ambient_lights.clear();

    for (Light* light : lights) {
        light->ClampColor();
    }
}

void Render() {
	float d = PreRender();
    
	Image outputImg = Image(camera->res.x, camera->res.y);
#pragma omp parallel for num_threads(3) schedule(dynamic, camera->res.x)
    for (int i = 0; i < camera->res.x * camera->res.y; i++) {
        if (i == 0) Log(to_string(omp_get_num_threads()));
        int x = i % camera->res.x;
        int y = i / camera->res.x;
        vector<ImVec2> offsets;
#if SAMPLING == -1
        offsets = {ImVec2(0.50, 0.50), ImVec2(0.15, 0.15), ImVec2(0.85, 0.15), ImVec2(0.85, 0.85), ImVec2(0.15, 0.85)};
#elif SAMPLING == 0
        offsets = {ImVec2(0.5, 0.5)};
#else
        for (int samp_i = 0; samp_i < SAMPLING; samp_i++)
            offsets.push_back(ImVec2(randf(), randf()));
#endif
        Color col = Color(0, 0, 0);
        for (ImVec2 offset : offsets) {
            float u = camera->mid_res.x - x + offset.x;
            float v = camera->mid_res.y - y + offset.y;

            vec3 rayDir = (d * camera->forward + u * camera->right + v * camera->up).normalized();

            Ray ray = Ray(camera->position, rayDir, camera->max_depth);
            Color new_color = EvaluateRay(ray);
            new_color.Clamp();
            col = col + new_color * (1.0 / offsets.size());
        }
        outputImg.setPixel(x, y, col);
    }

	PostRender();

    // TODO: Instead of displaying image, write to a buffer.
    string relative_output_name = "output/" + string(output_name);
    outputImg.write(relative_output_name.c_str());
}

void RenderOne() {
    PreRender();

    Color col = Color(0, 0, 0);
    float u = camera->mid_res.x;
    float v = camera->mid_res.y;

    vec3 rayDir = camera->forward.normalized();

    Ray ray = Ray(camera->position, rayDir, camera->max_depth);
    Color new_color = EvaluateRay(ray);
    new_color.Clamp();

    PostRender();
}

void RequestRender() {
    last_request = chrono::steady_clock::now();
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

}  //  namespace Raytracer

using namespace Raytracer;

// TODO: move more code out of main.
// TODO: Changing program state shouldn't happen during a render call.
// This can throw exceptions if we change something like image size during a render.
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
    (void)io; // ?

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    future<void> render_call;

    // Main loop
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYDOWN) {
                char key = event.key.keysym.sym;
                if (key == SDLK_SPACE) {
                    camera->position += camera->up * 0.1;
                }
                if (key == SDLK_x) {
                    camera->position -= camera->up * 0.1;
                }
                if (key == SDLK_w) {
                    camera->position += camera->forward * 0.1;
                }
                if (key == SDLK_s) {
                    camera->position -= camera->forward * 0.1;
                }
                if (key == SDLK_a) {
                    camera->position += camera->right * 0.1;
                }
                if (key == SDLK_d) {
                    camera->position -= camera->right * 0.1;
                }
                RequestRender();
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::Begin("Renderer Settings");

        duration<double> span = duration_cast<duration<double>>(steady_clock::now() - last_request);
        bool time_update = update_automatically && (last_request != steady_clock::time_point() && span.count() > RENDER_DELAY);
        if (ImGui::Button("Render", ImVec2(ImGui::GetWindowWidth() * 0.55, 0)) || time_update) {
            last_request = steady_clock::time_point();
            render_call = async(Render);
        }
        ImGui::SameLine();
        if (ImGui::Button("RenderOne", ImVec2(ImGui::GetWindowWidth() * 0.20, 0))) {
            RenderOne();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto", &update_automatically);

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

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.7, 0.7, 1.0, 1.0});
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4{0.4, 0.4, 0.5, 1.0});
        camera->ImGui();
        ImGui::PopStyleColor(2);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0, 0.7, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Geometry")) {
            for (Geometry* shape : shapes) {
                shape->ImGui();
            }
            if (ImGui::Button("New Sphere", ImVec2(ImGui::GetWindowWidth() / 3 - H_SPACING * 2, 0))) {
                Material* mat = new Material(&entity_count);
                materials.push_back(mat);
                shapes.push_back(new Sphere(&entity_count, mat));
            }
			ImGui::SameLine(0.0f, H_SPACING);
            if (ImGui::Button("New Triangle", ImVec2(ImGui::GetWindowWidth() / 3 - H_SPACING * 2, 0))) {
                Material* mat = new Material(&entity_count);
                materials.push_back(mat);
                shapes.push_back(new Triangle(&entity_count, mat));
            }
			ImGui::SameLine(0.0f, H_SPACING);
            if (ImGui::Button("New NormTriangle", ImVec2(ImGui::GetWindowWidth() / 3 - H_SPACING * 2, 0))) {
                Material* mat = new Material(&entity_count);
                materials.push_back(mat);
                shapes.push_back(new NormalTriangle(&entity_count, mat));
            }
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0, 1.0, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Lighting")) {
            for (Light* light : lights) {
                light->ImGui();
            }
            if (ImGui::Button("New Ambient", ImVec2(ImGui::GetWindowWidth() / 4 - H_SPACING * 2, 0))) {
                lights.push_back(new AmbientLight(&entity_count));
            }
            ImGui::SameLine(0.0f, H_SPACING);
            if (ImGui::Button("New Point", ImVec2(ImGui::GetWindowWidth() / 4 - H_SPACING * 2, 0))) {
                lights.push_back(new PointLight(&entity_count));
            }
            ImGui::SameLine(0.0f, H_SPACING);
            if (ImGui::Button("New Spot", ImVec2(ImGui::GetWindowWidth() / 4 - H_SPACING * 2, 0))) {
                lights.push_back(new SpotLight(&entity_count));
            }
            ImGui::SameLine(0.0f, H_SPACING);
            if (ImGui::Button("New Directional", ImVec2(ImGui::GetWindowWidth() / 4 - H_SPACING * 2, 0))) {
                lights.push_back(new DirectionalLight(&entity_count));
            }
        }
        ImGui::PopStyleColor();

        DisplayLog();

        ImGui::End();


        if (render_call.valid()) {
            render_call.get();  // Calling get makes the render_call invalid, storing that we used it.
            string relative_output_name = "output/" + string(output_name);
            if (_access(relative_output_name.c_str(), 0) != -1)
                DisplayImage(relative_output_name);
        }

        if (disp_img_tex != -1) {
            if (false) {
                ImGui::Begin("Output");
                ImGui::Image((void*)disp_img_tex, disp_img_size);
                ImGui::End();
            }
            else {
                Log(to_string(disp_img_tex));
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                ImVec2 window_center = ImVec2(w * 0.5f, h * 0.5f);
                float vert = h / disp_img_size.y;
                float hori = w / disp_img_size.x;
                float scale = fmin(vert, hori);
                ImVec2 half_size = ImVec2(disp_img_size.x * 0.5 * scale, disp_img_size.y * 0.5 * scale);
                ImGui::GetBackgroundDrawList()->AddImage((void*)disp_img_tex, window_center - half_size, window_center + half_size);
                if (!io.WantCaptureMouse) {

                }
            }
        }


        // Rendering
        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    delete camera;
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
