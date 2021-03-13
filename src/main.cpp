// Main templated from Dear ImGui

//#define _USE_MATH_DEFINES
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "image_lib.h"
#include "main.h"

#define AA_NONE 0
#define AA_FIVE -1

#define SAMPLING 0


namespace P3 {


char output_name[256] = "raytraced.bmp";

// UI STATE
vector<UIGeometry*> ui_shapes{};
vector<UILight*> ui_lights{};
vector<UIMaterial*> ui_materials{ new UIMaterial() };
UICamera* ui_camera = new UICamera();
int entity_count = 0;
char scene_name[256] = "";

ImVec2 disp_img_size{ 0.0, 0.0 };
GLuint disp_img_tex = -1;

// RENDER STATE
Camera* camera;
vector<Geometry*> geometry;
vector<Light*> lights;
vector<AmbientLight*> ambient;


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


// Call without index to create new ui_shapes
UIGeometry::UIGeometry() : UIObject() {
    mat = ui_materials.back();
}
// Call with index to replace ui_shapes (use old id)
UIGeometry::UIGeometry(int old_id) : UIObject(old_id) {
    mat = ui_materials.back();
}


void UIGeometry::Delete() { ui_shapes.erase(GetIter()); }
void UILight::Delete() { ui_lights.erase(GetIter()); }


vector<UIGeometry*>::iterator UIGeometry::GetIter() {
    for (vector<UIGeometry*>::iterator it = ui_shapes.begin(); it < ui_shapes.end(); it++) {
        if ((*it)->id == id) {
            return it;
        }
    }
    cout << "Geometry not found in ui_shapes" << endl;
    return ui_shapes.end();
}


vector<UILight*>::iterator UILight::GetIter() {
    for (vector<UILight*>::iterator it = ui_lights.begin(); it < ui_lights.end(); it++) {
        if ((*it)->id == id) {
            return it;
        }
    }
    cout << "Light not found in ui_lights" << endl;
    return ui_lights.end();
}


void UICamera::ImGui() {
    if (ImGui::CollapsingHeader("Camera Parameters")) {
        ImGui::DragFloat3("Camera Position", cp, 0.1);
        ImGui::SliderFloat3("Camera Forward", cf, -1.0f, 1.0f);
        ImGui::SliderFloat3("Camera Up", cu, -1.0f, 1.0f);
        ImGui::SliderFloat("FOV", &fov_ha, 0.0f, 90.0f);
        ImGui::DragInt2("Resolution", window_res, 1);
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
        ImGui::DragFloat3(WithId("pos##").c_str(), pos, 0.02);
        ImGui::DragFloat(WithId("radius##").c_str(), &rad, 0.01, 0.01);
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
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Light ").c_str())) {
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}


void UIAmbientLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Ambient ").c_str())) {
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void UIPointLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Point ").c_str())) {
        ImGui::DragFloat3(WithId("pos##").c_str(), pos, 0.1);
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void UISpotLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Spot ").c_str())) {
        ImGui::DragFloat3(WithId("pos##").c_str(), pos, 0.1);
        ImGui::SliderFloat3(WithId("dir##").c_str(), dir, -1, 1);
        ImGui::SliderFloat(WithId("interior_angle##").c_str(), &angle1, 0, 90);
        angle2 = max(angle1, angle2);
        ImGui::SliderFloat(WithId("exterior_angle##").c_str(), &angle2, angle1, 90);
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}

void UIDirectionalLight::ImGui() {
    ImGui::Indent(6.0f);
    if (ImGui::CollapsingHeader(WithId("Directional ").c_str())) {
        ImGui::DragFloat(WithId("Multiplier").c_str(), &mult, 0.05, 0.01);
        ImGui::DragFloat3(WithId("direction##").c_str(), dir, 0.01, -1.0, 1.0);
        ImGui::ColorPicker3(WithId("color##").c_str(), col);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
}


void UICamera::Decode(string& s) {
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


void UIMaterial::Decode(string& s) {
    stringstream ss(s);
    ss >> ambient[0] >> ambient[1] >> ambient[2] >>
        diffuse[0] >> diffuse[1] >> diffuse[2] >>
        specular[0] >> specular[1] >> specular[2] >> phong >>
        transmissive[0] >> transmissive[1] >> transmissive[2] >> ior;
}

void UIGeometry::Decode(string& s) {
}


void UISphere::Decode(string& s) {
    stringstream ss(s);
    ss >> pos[0] >> pos[1] >> pos[2] >> rad;
}

void UILight::Decode(string& s) {
    stringstream ss(s);
    ss >> col[0] >> col[1] >> col[2];
    if (col[0] > 1 || col[1] > 1 || col[2] > 1) {
        float max_col = max(col[0], max(col[1], col[2]));
        mult = max_col;
        col[0] /= mult;
        col[1] /= mult;
        col[2] /= mult;
    }

    getline(ss, s);
}

void UIAmbientLight::Decode(string& s) {
    UILight::Decode(s);
}

void UIPointLight::Decode(string& s) {
    UILight::Decode(s);
    stringstream ss(s);
    ss >> pos[0] >> pos[1] >> pos[2];
}

void UIDirectionalLight::Decode(string& s) {
    UILight::Decode(s);
    stringstream ss(s);
    ss >> dir[0] >> dir[1] >> dir[2];
}

void UISpotLight::Decode(string& s) {
    UILight::Decode(s);
    stringstream ss(s);
    ss >> pos[0] >> pos[1] >> pos[2] >> dir[0] >> dir[1] >> dir[2] >> angle1 >> angle2;
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
    sprintf(temp, "%f %f %f", col[0] * mult, col[1] * mult, col[2] * mult);

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
    ss << "spot_light: " << UILight::Encode() << " " << pos[0] << " " << pos[1] << " " << pos[2] << " " <<dir[0] << " " << dir[1] << " " << dir[2] << " " << angle1 << " " << angle2;

    return ss.str();
}


Geometry* UIGeometry::ToGeometry() {
    return new Geometry(this);
}

Geometry* UISphere::ToGeometry() {
    return new Sphere(this);
}


Light* UILight::ToLight() {
    return new Light(this);
}


Light* UIAmbientLight::ToLight() {
    return new AmbientLight(this);
}


Light* UIDirectionalLight::ToLight() {
    return new DirectionalLight(this);
}


Light* UIPointLight::ToLight() {
    return new PointLight(this);
}

Light* UISpotLight::ToLight() {
    return new SpotLight(this);
}


Camera::Camera(UICamera* from) {
    position = Point3D(from->cp[0], from->cp[1], from->cp[2]);
    background_color = Color(from->bc[0], from->bc[1], from->bc[2]);
    half_vfov = from->fov_ha;
    res = {from->window_res[X], from->window_res[Y]};
    mid_res = { res[X]/2, res[Y]/2 };
    max_depth = from->max_depth;

    forward = Dir3D(from->cf[0], from->cf[1], from->cf[2]).normalized();
    up = Dir3D(from->cu[0], from->cu[1], from->cu[2]);
    right = cross(up, forward).normalized();
    up = cross(forward, right).normalized();
}


Material::Material(UIMaterial* from) {
    ambient = Color(from->ambient[0], from->ambient[1], from->ambient[2]);
    diffuse = Color(from->diffuse[0], from->diffuse[1], from->diffuse[2]);
    specular = Color(from->specular[0], from->specular[1], from->specular[2]);
    transmissive = Color(from->transmissive[0], from->transmissive[1], from->transmissive[2]);
    phong = from->phong;
    ior = from->ior;
}


Geometry::Geometry(UIGeometry* from) {
    material = Material(from->mat);
}


Sphere::Sphere(UISphere* from) : Geometry(from) {
    position = Point3D(from->pos[0], from->pos[1], from->pos[2]);
    radius = from->rad;
}


Light::Light(UILight* from) {
    float m = from->mult;
    color = Color(from->col[0] * m, from->col[1] * m, from->col[2] * m);
}

DirectionalLight::DirectionalLight(UIDirectionalLight* from) : Light(from) {
    direction = Dir3D(from->dir[0], from->dir[1], from->dir[2]);
}

PointLight::PointLight(UIPointLight* from) : Light(from) {
    position = Point3D(from->pos[0], from->pos[1], from->pos[2]);
}

SpotLight::SpotLight(UISpotLight* from) : Light(from) {
    position = Point3D(from->pos[0], from->pos[1], from->pos[2]);
    direction = Dir3D(from->dir[0], from->dir[1], from->dir[2]).normalized();

    angle1 = from->angle1;
    angle2 = from->angle2;
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
    delete ui_camera;
    for (UIGeometry* o : ui_shapes) {
        delete o;
    }
    for (UILight* o : ui_lights) {
        delete o;
    }
    for (UIMaterial* o : ui_materials) {
        delete o;
    }
    ui_shapes.clear();
    ui_lights.clear();
    ui_materials.clear();

    strcpy(output_name, "");

    entity_count = 0;
    ui_materials.push_back(new UIMaterial());
    ui_camera = new UICamera();
}


void Load() {
    Reset();

    string scene_string = "scenes/" + string(scene_name) + ".p3";

    ifstream scene_file(scene_string);
    if (!scene_file.is_open()) return;

    string line;
    while (getline(scene_file, line)) {
        string rest;

        // TODO: Should this be the syntax for all Decode calls?
        // Probably lol...
        ui_camera->Decode(line);

        rest = rest_if_prefix("sphere: ", line);
        if (rest != "") {
            UISphere* new_sphere = new UISphere();
            new_sphere->Decode(rest);
            ui_shapes.push_back(new_sphere);
        }

        rest = rest_if_prefix("material: ", line);
        if (rest != "") {
            UIMaterial* new_mat = new UIMaterial();
            new_mat->Decode(rest);
            ui_materials.push_back(new_mat);
        }

        rest = rest_if_prefix("ambient_light: ", line);
        if (rest != "") {
            UIAmbientLight* new_light = new UIAmbientLight();
            new_light->Decode(rest);
            ui_lights.push_back(new_light);
        }

        rest = rest_if_prefix("directional_light: ", line);
        if (rest != "") {
            UIDirectionalLight* new_light = new UIDirectionalLight();
            new_light->Decode(rest);
            ui_lights.push_back(new_light);
        }

        rest = rest_if_prefix("point_light: ", line);
        if (rest != "") {
            UIPointLight* new_light = new UIPointLight();
            new_light->Decode(rest);
            ui_lights.push_back(new_light);
        }

        rest = rest_if_prefix("spot_light: ", line);
        if (rest != "") {
            UISpotLight* new_light = new UISpotLight();
            new_light->Decode(rest);
            ui_lights.push_back(new_light);
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

    scene_file << ui_camera->Encode() << endl;

    for (UIGeometry* geo : ui_shapes) {
        scene_file << geo->Encode() << endl;
    }

    for (UILight* light : ui_lights) {
        scene_file << light->Encode() << endl;
    }

    scene_file.close();
}


Color ApplyLighting(Ray ray, HitInformation hit_info) {
    Color current(0, 0, 0);

    for (Light* light : lights) {
        Ray to_light = light->ReverseLightRay(hit_info.pos);
        to_light.pos = to_light.pos + hit_info.normal * 0.0001;
        HitInformation light_intersection;
        // If light is blocked
        if (FindIntersection(geometry, to_light, &light_intersection) && light_intersection.dist < light->DistanceTo(hit_info.pos)) continue;

        current = current + CalculateDiffuse(light, hit_info);
        current = current + CalculateSpecular(light, hit_info);
    }
    Ray reflected = Reflect(-hit_info.viewing, hit_info.pos, hit_info.normal, ray.bounces_left - 1);
    current = current + hit_info.material.specular * EvaluateRay(reflected);
    current = current + CalculateAmbient(hit_info);

    return current;
}


Color EvaluateRay(Ray ray) {
    if (ray.bounces_left <= 0) return camera->background_color;

    HitInformation hit_info;
    if (FindIntersection(geometry, ray, &hit_info)) {
        return ApplyLighting(ray, hit_info);
    }
    else {
        return camera->background_color;
    }
}

Color CalculateDiffuse(Light* light, HitInformation hit) {
    Dir3D to_light = light->ReverseLightRay(hit.pos).dir;
    float amount = max(0, dot(hit.normal, to_light));
    Color il = light->Intensity(hit.pos);
    return hit.material.diffuse * il * amount;
}

Color CalculateSpecular(Light* light, HitInformation hit) {
    Dir3D to_light = light->ReverseLightRay(hit.pos).dir;
    Dir3D to_viewer = -hit.viewing;
    Ray test = Reflect(to_light, hit.pos, hit.normal, -1);
    float amount = pow(max(0, dot(test.dir, to_viewer)), hit.material.phong);

    Color il = light->Intensity(hit.pos);

    return hit.material.specular * il * amount;
}

Color CalculateAmbient(HitInformation hit) {
    Color c = Color(0, 0, 0);

    for (AmbientLight* al : ambient) {
        c = c + hit.material.ambient * al->color;
    }
    return c;
}


void Render() {
    camera = new Camera(ui_camera);

    float d = camera->mid_res[Y] / tanf(camera->half_vfov * (M_PI / 180.0f));

    for (UIGeometry* geo : ui_shapes) {
        geometry.push_back(geo->ToGeometry());
    }

    for (UILight* light : ui_lights) {
        Light* l = light->ToLight();
        AmbientLight* al = dynamic_cast<AmbientLight*>(l);
        if (al == NULL) {
            lights.push_back(l);
        }
        else {
            ambient.push_back(al);
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

    string relative_output_name = "output/" + string(output_name);
    outputImg.write(relative_output_name.c_str());
    DisplayImage(relative_output_name);

    delete camera;
    for (Geometry* geo : geometry) {
        delete geo;
    }
    for (Light* light : lights) {
        delete light;
    }
    for (AmbientLight* light : ambient) {
        delete light;
    }
    geometry.clear();
    lights.clear();
    ambient.clear();

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
        ui_camera->ImGui();
        ImGui::PopStyleColor(2);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0, 0.7, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Geometry")) {
            for (UIGeometry* shape : ui_shapes) {
                shape->ImGui();
            }
            if (ImGui::Button("New Sphere", ImVec2(ImGui::GetWindowWidth(), 0))) {
                UIMaterial* mat = new UIMaterial();
                ui_materials.push_back(mat);
                ui_shapes.push_back(new UISphere());
            }
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0, 1.0, 0.7, 1.0});
        if (ImGui::CollapsingHeader("Lighting")) {
            float spacing = 4.0f;
            for (UILight* light : ui_lights) {
                light->ImGui();
            }
            if (ImGui::Button("New Ambient", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                ui_lights.push_back(new UIAmbientLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Point", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                ui_lights.push_back(new UIPointLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Spot", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                ui_lights.push_back(new UISpotLight());
            }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::Button("New Directional", ImVec2(ImGui::GetWindowWidth() / 4 - spacing*2, 0))) {
                ui_lights.push_back(new UIDirectionalLight());
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

    for (UIGeometry* geo : ui_shapes) {
        delete geo;
    }

    for (UILight* light : ui_lights) {
        delete light;
    }

    for (UIMaterial* mat : ui_materials) {
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
