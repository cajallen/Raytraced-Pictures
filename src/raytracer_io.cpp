#include "raytracer_main.h"

namespace P3 {

string rest_if_prefix(const string prefix, string content) {
    return content.compare(0, prefix.length(), prefix) == 0 ? content.substr(prefix.length()) : "";
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
    ss >> ambient.r >> ambient.g >> ambient.b >> diffuse.r >> diffuse.g >> diffuse.b >> specular.r >> specular.g >>
        specular.b >> phong >> transmissive.r >> transmissive.g >> transmissive.b >> ior;
}

void Geometry::Decode(string& s) {}

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
    ostringstream oss;
    ossstream osss(oss);

    osss << "camera_pos:" << position.x << position.y << position.z;
    osss << "camera_fwd:" << forward.x << forward.y << forward.z;
    osss << "camera_up:" << up.x << up.y << up.z;
    osss << "camera_fov_ha:" << half_vfov;
    osss << "film_resolution:" << res[0] << res[1];
    osss << "output_image:" << output_name;
    osss << "background:" << background_color.r << background_color.g << background_color.b;
    osss << "max_depth:" << max_depth << "";

    return oss.str();
}

string Material::Encode() {
    ostringstream oss("material:");
    ossstream osss(oss);
    osss << ambient.r << ambient.g << ambient.b << diffuse.r << diffuse.g << diffuse.b << specular.r << specular.g
        << specular.b << phong << transmissive.r << transmissive.g << transmissive.b << ior;
    return oss.str();
}

string Geometry::Encode() {
    string temp = material->Encode();
    temp += "\n";
    return temp;
}

string Sphere::Encode() {
    ostringstream oss(Geometry::Encode() + "sphere:");
    ossstream osss(oss);
    osss << position.x << position.y << position.z << radius;
    return oss.str();
}

string Light::Encode() {
    ostringstream oss;
    ossstream osss(oss);
    osss << color.r * mult << color.g * mult << color.b * mult;
    return oss.str();
}

string AmbientLight::Encode() {
    string temp = "ambient_light:" + Light::Encode();
    return temp;
}

string DirectionalLight::Encode() {
    ostringstream oss("directional_light:" + Light::Encode());
    ossstream osss(oss);
    osss << direction.x << direction.y << direction.z;
    return oss.str();
}

string PointLight::Encode() {
    ostringstream oss("point_light:" + Light::Encode());
    ossstream osss(oss);
    osss << position.x << position.y << position.z;
    return oss.str();
}

string SpotLight::Encode() {
    ostringstream oss("spot_light:" + Light::Encode());
    ossstream osss(oss);
    osss << position.x << position.y << position.z << direction.x << direction.y << direction.z << angle1 << angle2;
    return oss.str();
}

void Load() {
    Reset();

    string scene_string = "scenes/" + string(scene_name) + ".p3";

    ifstream scene_file(scene_string);
    if (!scene_file.is_open())
        return;

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
    if (!scene_file.is_open()) {
        return;
    }

    scene_file << camera->Encode() << endl;

    for (Geometry* geo : shapes) {
        scene_file << geo->Encode() << endl;
    }

    for (Light* light : lights) {
        scene_file << light->Encode() << endl;
    }

    scene_file.close();
}


}  // namespace P3