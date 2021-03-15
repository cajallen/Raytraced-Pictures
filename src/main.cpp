// Main templated from Dear ImGui
#include "main.h"

#define AA_NONE 0
#define AA_FIVE -1

#define SAMPLING 0


namespace P3 {


char output_name[256] = "raytraced.bmp";

// UI STATE
vector<Geometry*> shapes{};
vector<Light*> lights{};
vector<AmbientLight*> ambient_lights{};
vector<Material*> materials{ new Material() };
Camera* camera = new Camera();
int entity_count = 0;
char scene_name[256] = "";

ImVec2 disp_img_size{ 0.0, 0.0 };
GLuint disp_img_tex = -1;


string rest_if_prefix(const string prefix, string content) {
    if (content.compare(0, prefix.length(), prefix) == 0) {
        return content.substr(prefix.length());
    }
    else {
        return "";
    }
}


Ray Reflect(Dir3D ang, Point3D pos, Dir3D norm, int bounces_left) {
    Line3D normal_dir = vee(Point3D(0, 0, 0), norm).normalized();
    Point3D midpoint = proj(Point3D(ang.x, ang.y, ang.z), normal_dir);
    Point3D diff = midpoint - ang;
    Point3D to_out = (midpoint + diff).normalized();

    Point3D from = pos + norm * 0.001;

    return Ray(from, Dir3D(to_out.x, to_out.y, to_out.z), bounces_left);
}


Object::Object() {
    id = entity_count;
    entity_count++;
}

Object::Object(int old_id) {
    id = old_id;
}


string Object::WithId(string s) {
    return (s + std::to_string(id)).c_str();
}


// Call without index to create new shapes
Geometry::Geometry() : Object() {
    material = materials.back();
}
// Call with index to replace shapes (use old id)
Geometry::Geometry(int old_id) : Object(old_id) {
    material = materials.back();
}


void Geometry::Delete() { shapes.erase(GetIter()); }
void Light::Delete() { lights.erase(GetIter()); }


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


void Camera::ImGui() {
    if (ImGui::CollapsingHeader("Camera Parameters")) {
        ImGui::DragFloat3("Camera Position", &position.x, 0.1);
        ImGui::SliderFloat3("Camera Forward", &forward.x, -1.0f, 1.0f);
        ImGui::SliderFloat3("Camera Up", &up.x, -1.0f, 1.0f);
        ImGui::SliderFloat("FOV", &half_vfov, 0.0f, 90.0f);
        ImGui::DragInt2("Resolution", &res[0], 1);
        ImGui::SliderInt("Max Depth", &max_depth, 1, 20);
        ImGui::ColorPicker3("Background Color", &background_color.r);
    }
    mid_res = { res[X] / 2, res[Y] / 2 };

    forward = forward.normalized();
    right = cross(up, forward).normalized();
    up = cross(forward, right).normalized();
}

void Geometry::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Geometry ").c_str())) {
        if (ImGui::Button(WithId("Sphere").c_str())) {
            auto iter = GetIter();
            Geometry* old = *iter;
            *iter = new Sphere(id);
            delete old;
        }
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void Sphere::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Sphere ").c_str())) {
        ImGui::DragFloat3(WithId("pos##").c_str(), &position.x, 0.02);
        ImGui::DragFloat(WithId("radius##").c_str(), &radius, 0.01, 0.01);
        ImGui::Indent(3.0f);
        material->ImGui();
        ImGui::Unindent(3.0f);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}


void Material::ImGui() {
    if (ImGui::CollapsingHeader(WithId("Material ").c_str())) {
        ImGui::Indent(4.0f);
        ImGui::SliderFloat3(WithId("ambient##").c_str(), &ambient.r, 0.0f, 1.0f);
        ImGui::SliderFloat3(WithId("diffuse##").c_str(), &diffuse.r, 0.0f, 1.0f);
        ImGui::SliderFloat3(WithId("specular##").c_str(), &specular.r, 0.0f, 1.0f);
        ImGui::SliderFloat3(WithId("transmissive##").c_str(), &transmissive.r, 0.0f, 1.0f);
        ImGui::SliderFloat(WithId("phong##").c_str(), &phong, 0.0f, 50.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat(WithId("ior##").c_str(), &ior, 0.0f, 1.0f);
        ImGui::Unindent(4.0f);
    }
}


void Light::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Light ").c_str())) {
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void AmbientLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Ambient ").c_str())) {
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        ImGui::ColorPicker3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void PointLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Point ").c_str())) {
        ImGui::DragFloat3(WithId("pos##").c_str(), &position.x, 0.1);
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        ImGui::ColorPicker3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void SpotLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Spot ").c_str())) {
        ImGui::DragFloat3(WithId("pos##").c_str(), &position.x, 0.1);
        ImGui::SliderFloat3(WithId("dir##").c_str(), &direction.x, -1, 1);
        ImGui::SliderFloat(WithId("interior_angle##").c_str(), &angle1, 0, 90);
        angle2 = max(angle1, angle2);
        ImGui::SliderFloat(WithId("exterior_angle##").c_str(), &angle2, angle1, 90);
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        ImGui::ColorPicker3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void DirectionalLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Directional ").c_str())) {
        ImGui::DragFloat(WithId("Multiplier").c_str(), &mult, 0.05, 0.01);
        ImGui::DragFloat3(WithId("direction##").c_str(), &direction.x, 0.01, -1.0, 1.0);
        ImGui::ColorPicker3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}


void Camera::Decode(string& s) {
    string rest;
    rest = rest_if_prefix("camera_pos: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> position.x >> position.y >> position.z;
    }

    rest = rest_if_prefix("camera_fwd: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> forward.x >> forward.y >> forward.z;
    }

    rest = rest_if_prefix("camera_up: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> up.x >> up.y >> up.z;
    }

    rest = rest_if_prefix("camera_fov_ha: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> half_vfov;
    }

    rest = rest_if_prefix("film_resolution: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> res[0] >> res[1];
    }

    rest = rest_if_prefix("output_image: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> output_name;
    }

    rest = rest_if_prefix("background: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> background_color.r >> background_color.g >> background_color.b;
    }

    rest = rest_if_prefix("max_depth: ", s);
    if (rest != "") {
        stringstream ss(rest);
        ss >> max_depth;
    }
}


void Material::Decode(string& s) {
    stringstream ss(s);
    ss >> ambient.r >> ambient.g >> ambient.b >>
        diffuse.r >> diffuse.g >> diffuse.b >>
        specular.r >> specular.g >> specular.b >> phong >>
        transmissive.r >> transmissive.g >> transmissive.b >> ior;
}

void Geometry::Decode(string& s) {
}


void Sphere::Decode(string& s) {
    stringstream ss(s);
    ss >> position.x >> position.y >> position.z >> radius;
}

void Light::Decode(string& s) {
    stringstream ss(s);
    ss >> color.r >> color.g >> color.b;
    ClampColor();

    getline(ss, s);
}

void AmbientLight::Decode(string& s) {
    Light::Decode(s);
}

void PointLight::Decode(string& s) {
    Light::Decode(s);
    stringstream ss(s);
    ss >> position.x >> position.y >> position.z;
}

void DirectionalLight::Decode(string& s) {
    Light::Decode(s);
    stringstream ss(s);
    ss >> direction.x >> direction.y >> direction.z;
}

void SpotLight::Decode(string& s) {
    Light::Decode(s);
    stringstream ss(s);
    ss >> position.x >> position.y >> position.z >> direction.x >> direction.y >> direction.z >> angle1 >> angle2;
}


string Camera::Encode() {
    stringstream ss;

    ss << "camera_pos: " << position.x << " " << position.y << " " << position.z << " " << endl;
    ss << "camera_fwd: " << forward.x << " " << forward.y << " " << forward.z << " " << endl;
    ss << "camera_up: " << up.x << " " << up.y << " " << up.z << " " << endl;
    ss << "camera_fov_ha: " << half_vfov << endl;
    ss << "film_resolution: " << res[0] << " " << res[1] << endl;
    ss << "output_image: " << output_name << endl;
    ss << "background: " << background_color.r << " " << background_color.g << " " << background_color.b << " " << endl;
    ss << "max_depth: " << max_depth << endl;

    return ss.str();
}


string Material::Encode() {
    char temp[256];
    sprintf(temp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f",
        ambient.r, ambient.g, ambient.b,
        diffuse.r, diffuse.g, diffuse.b,
        specular.r, specular.g, specular.b, phong,
        transmissive.r, transmissive.g, transmissive.b, ior);

    return string(temp);
}

string Geometry::Encode() {
    return string("material: ") + material->Encode() + string("\n");
}

string Sphere::Encode() {
    char temp[256];
    sprintf(temp, "sphere: %f %f %f %f", position.x, position.y, position.z, radius);

    return Geometry::Encode() + string(temp);
}

string Light::Encode() {
    char temp[256];
    sprintf(temp, "%f %f %f", color.r * mult, color.g * mult, color.b * mult);

    return string(temp);
}

string AmbientLight::Encode() {
    stringstream ss;
    ss << "ambient_light: " << Light::Encode();

    return ss.str();
}

string DirectionalLight::Encode() {
    stringstream ss;
    ss << "directional_light: " << Light::Encode() << " " << direction.x << " " << direction.y << " " << direction.z;

    return ss.str();
}

string PointLight::Encode() {
    stringstream ss;
    ss << "point_light: " << Light::Encode() << " " << position.x << " " << position.y << " " << position.z;

    return ss.str();
}

string SpotLight::Encode() {
    stringstream ss;
    ss << "spot_light: " << Light::Encode() << " " << position.x << " " << position.y << " " << position.z <<
			" " << direction.x << " " << direction.y << " " << direction.z << " " << angle1 << " " << angle2;

    return ss.str();
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

    if (discr < 0) return false;

    float t0 = (-b + sqrt(discr)) / 2;
    float t1 = (-b - sqrt(discr)) / 2;
    float mint = t0;
    if (t1 > 0.0 && t1 < t0) mint = t1;

    if (mint < 0.0) return false;

    Point3D hit_pos = ray.pos + mint * ray.dir;
    Dir3D hit_norm = (hit_pos - position).normalized();
    *intersection = HitInformation{ mint, hit_pos, ray.dir, hit_norm, material };
    
    return true;
}


Ray DirectionalLight::ReverseLightRay(Point3D from) {
    return Ray{ from, -direction, -1 };
}


Ray PointLight::ReverseLightRay(Point3D from) {
    Dir3D offset = position - from;

    return Ray{ from, offset.normalized(), -1 };
}

Ray SpotLight::ReverseLightRay(Point3D from) {
    Dir3D offset = position - from;

    return Ray{ from, offset.normalized(), -1 };
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

    return Color(r,g,b);
}


Color SpotLight::Intensity(Point3D to) {
    Dir3D angle_to = (to - position).normalized();
    float diff = 180 * acos(dot(angle_to, direction)) / M_PI;
    float dist2 = pow(DistanceTo(to), 2);

    if (diff < angle1) return color * (1/dist2);
    float amount = 1 - ((diff - angle1) / (angle2 - angle1));
    if (diff < angle2) return color * amount * (1 / dist2);

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


void Load() {
    Reset();

    string scene_string = "scenes/" + string(scene_name) + ".p3";

    ifstream scene_file(scene_string);
    if (!scene_file.is_open()) return;

    string line;
    while (getline(scene_file, line)) {
        string rest;

        // Only camera can Decode this way because only camera is pseudostatic
        camera->Decode(line);

        rest = rest_if_prefix("sphere: ", line);
        if (rest != "") {
            Sphere* new_sphere = new Sphere();
            new_sphere->Decode(rest);
            shapes.push_back(new_sphere);
        }

        rest = rest_if_prefix("material: ", line);
        if (rest != "") {
            Material* new_mat = new Material();
            new_mat->Decode(rest);
            materials.push_back(new_mat);
        }

        rest = rest_if_prefix("ambient_light: ", line);
        if (rest != "") {
            AmbientLight* new_light = new AmbientLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("directional_light: ", line);
        if (rest != "") {
            DirectionalLight* new_light = new DirectionalLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("point_light: ", line);
        if (rest != "") {
            PointLight* new_light = new PointLight();
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("spot_light: ", line);
        if (rest != "") {
            SpotLight* new_light = new SpotLight();
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

    for (Geometry* geo : shapes) {
        scene_file << geo->Encode() << endl;
    }

    for (Light* light : lights) {
        scene_file << light->Encode() << endl;
    }

    scene_file.close();
}


Color ApplyLighting(Ray ray, HitInformation hit_info) {
    Color current(0, 0, 0);

    for (Light* light : lights) {
        if (light->Intensity(hit_info.pos) == Color(0, 0, 0)) continue;

        Ray to_light = light->ReverseLightRay(hit_info.pos);
        to_light.pos = to_light.pos + hit_info.normal * 0.0001;
        HitInformation light_intersection;
        // If light is blocked
        if (FindIntersection(shapes, to_light, &light_intersection) && light_intersection.dist < light->DistanceTo(hit_info.pos)) continue;

        current = current + CalculateDiffuse(light, hit_info);
        current = current + CalculateSpecular(light, hit_info);
    }
    Ray reflected = Reflect(-hit_info.viewing, hit_info.pos, hit_info.normal, ray.bounces_left - 1);
    current = current + hit_info.material->specular * EvaluateRay(reflected);
    current = current + CalculateAmbient(hit_info);

    return current;
}


Color EvaluateRay(Ray ray) {
    if (ray.bounces_left <= 0) return camera->background_color;

    HitInformation hit_info;
    if (FindIntersection(shapes, ray, &hit_info)) {
        return ApplyLighting(ray, hit_info);
    }
    else {
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
    for (int i = 0; i < camera->res[X]; i++) {
        for (int j = 0; j < camera->res[Y]; j++) {
#if SAMPLING == -1
            Color col = Color(0, 0, 0);
            Color last_color;
            for (ImVec2 offset : {ImVec2(0.5, 0.5), ImVec2(0.1, 0.1), ImVec2(0.9, 0.1), ImVec2(0.9, 0.9), ImVec2(0.1, 0.9)}) {
                float ii = i + offset.x;
                float jj = j + offset.y;
                float u = camera->mid_res[X] - ii;
                float v = camera->mid_res[Y] - jj;

                Dir3D rayDir = (-d * camera->forward + u * camera->right + v * camera->up).normalized();

                Ray ray = Ray(camera->position, rayDir, camera->max_depth);
                Color new_color = EvaluateRay(ray);
                new_color.Clamp();
                col = col + (new_color * 0.25);
            }
#elif SAMPLING == 0
            float u = camera->mid_res[X] - i + 0.5;
            float v = camera->mid_res[Y] - j + 0.5;

            Dir3D rayDir = (-d * camera->forward + u * camera->right + v * camera->up).normalized();

            Ray ray = Ray(camera->position, rayDir, camera->max_depth);
            Color col = EvaluateRay(ray);
#else
            Color col = Color(0, 0, 0);
            Color last_color;
            for (int samp_i = 0; samp_i < SAMPLING; samp_i++) {
                float ii = i + rand() / (float) RAND_MAX;
                float jj = j + rand() / (float) RAND_MAX;
                float u = camera->mid_res[X] - ii;
                float v = camera->mid_res[Y] - jj;

                Dir3D rayDir = (-d * camera->forward + u * camera->right + v * camera->up).normalized();

                Ray ray = Ray(camera->position, rayDir, camera->max_depth);
                Color new_color = EvaluateRay(ray);
                new_color.Clamp();
                col = col + (new_color * 0.25);
            }
#endif
            outputImg.setPixel(i, j, col);
        }
    }

    ambient_lights.clear();

    for (Light* light : lights) {
        light->ClampColor();
    }

	// TODO: Instead of displaying image, write to a buffer.
    string relative_output_name = "output/" + string(output_name);
    outputImg.write(relative_output_name.c_str());
    DisplayImage(relative_output_name);
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

void DisplayImage(string name) {
    int im_x, im_y;
    bool ret = LoadTextureFromFile(name.c_str(), &disp_img_tex, &im_x, &im_y);
    disp_img_size = ImVec2(im_x, im_y);
    IM_ASSERT(ret);
}

} //  namespace P3

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
        ImGui::InputTextWithHint("", "<scene filename>", scene_name, 256, ImGuiInputTextFlags_CharsNoBlank);
        ImGui::InputTextWithHint("Output Name", "output.png", output_name, 256, ImGuiInputTextFlags_CharsNoBlank);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.7, 0.7, 1.0, 1.0 });
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4{ 0.4, 0.4, 0.5, 1.0 });
        camera->ImGui();
        ImGui::PopStyleColor(2);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0, 0.7, 0.7, 1.0});
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

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0, 1.0, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Lighting")) {
            float spacing = 4.0f;
            for (Light* light : lights) {
                light->ImGui();
            }
            if (ImGui::Button("New Ambient", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new AmbientLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Point", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new PointLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Spot", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new SpotLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Directional", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                lights.push_back(new DirectionalLight());
            }
        }
        ImGui::PopStyleColor();

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
