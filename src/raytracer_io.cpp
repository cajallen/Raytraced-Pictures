#include "raytracer_main.h"

namespace Raytracer {

LoadState load_state{};

string rest_if_prefix(const string prefix, string content) {
    return content.compare(0, prefix.length(), prefix) == 0 ? content.substr(prefix.length()) : "";
}

vec3 DecodeVertex(string& s) {
	stringstream ss(s);
	vec3 new_vert;
	ss >> new_vert.x >> new_vert.y >> new_vert.z;
	return new_vert;
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
        ss >> res.x >> res.y;
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

void Triangle::Decode(string& s) {
	stringstream ss(s);
	int i_v1, i_v2, i_v3;
	ss >> i_v1 >> i_v2 >> i_v3;
    int vsize = load_state.vertices.size();
	IM_ASSERT(i_v1 < vsize && i_v2 < vsize && i_v3 < vsize);
	v1 = load_state.vertices.at(i_v1);
	v2 = load_state.vertices.at(i_v2);
	v3 = load_state.vertices.at(i_v3);
}


void NormalTriangle::Decode(string& s) {
	stringstream ss(s);
	int i_v1, i_v2, i_v3, i_n1, i_n2, i_n3;
	ss >> i_v1 >> i_v2 >> i_v3 >> i_n1 >> i_n2 >> i_n3;
    int vsize = load_state.vertices.size();
    int nsize = load_state.normals.size();
	IM_ASSERT(i_v1 < vsize && i_v2 < vsize && i_v3 < vsize);
	IM_ASSERT(i_n1 < nsize && i_n2 < nsize && i_n3 < nsize);
	v1 = load_state.vertices.at(i_v1);
	v2 = load_state.vertices.at(i_v2);
	v3 = load_state.vertices.at(i_v3);
	n1 = load_state.normals.at(i_n1).normalized();
	n2 = load_state.normals.at(i_n2).normalized();
	n3 = load_state.normals.at(i_n3).normalized();
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
    osss << "film_resolution:" << res.x << res.y;
    osss << "output_image: ";
	oss << output_name;
    osss << "background:" << background_color.r << background_color.g << background_color.b;
    osss << "max_depth:" << max_depth << "";

    return oss.str();
}

string Material::Encode() {
    ostringstream oss;
	oss << "material:";
    ossstream osss(oss);
    osss << ambient.r << ambient.g << ambient.b << diffuse.r << diffuse.g << diffuse.b << specular.r << specular.g
        << specular.b << phong << transmissive.r << transmissive.g << transmissive.b << ior;
    return oss.str();
}

string Geometry::Encode() {
    string temp = "\n" + material->Encode() + "\n";
    return temp;
}

string Sphere::Encode() {
    ostringstream oss;
	oss << Geometry::Encode() << "sphere:";
    ossstream osss(oss);
    osss << position.x << position.y << position.z << radius;
    return oss.str();
}

string Triangle::Encode() {
    ostringstream oss;
	oss << Geometry::Encode() << endl;
    oss << v1.keyed_string("vertex: ") << endl;
	oss << v2.keyed_string("vertex: ") << endl;
	oss << v3.keyed_string("vertex: ");
	ossstream osss(oss);
    osss << "triangle:" << load_state.vertex_i++ << load_state.vertex_i++ << load_state.vertex_i++;
    return oss.str();
}

string NormalTriangle::Encode() {
    ostringstream oss;
	oss << Geometry::Encode() << endl;
    oss << v1.keyed_string("vertex: ") << endl;
	oss << v2.keyed_string("vertex: ") << endl;
	oss << v3.keyed_string("vertex: ") << endl;
	oss << n1.keyed_string("normal: ") << endl;
	oss << n2.keyed_string("normal: ") << endl;
	oss << n3.keyed_string("normal: ");
	ossstream osss(oss);
    osss << "normal_triangle:" << load_state.vertex_i++ << load_state.vertex_i++ << load_state.vertex_i++
		 << load_state.normal_i++ << load_state.normal_i++ << load_state.normal_i++;
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
    ostringstream oss;
	oss << "directional_light:" << Light::Encode();
    ossstream osss(oss);
    osss << direction.x << direction.y << direction.z;
    return oss.str();
}

string PointLight::Encode() {
    ostringstream oss;
	oss << "point_light:" << Light::Encode();
    ossstream osss(oss);
    osss << position.x << position.y << position.z;
    return oss.str();
}

string SpotLight::Encode() {
    ostringstream oss;
	oss << "spot_light:" << Light::Encode();
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
		// It does decode this way because otherwise we would need to check every line for all of its keys
        camera->Decode(line);

		rest = rest_if_prefix("vertex: ", line);
        if (rest != "") {
            vec3 v = DecodeVertex(rest);
            load_state.vertices.push_back(v);
        }

		rest = rest_if_prefix("normal: ", line);
        if (rest != "") {
            vec3 v = DecodeVertex(rest).normalized();
            load_state.normals.push_back(v);
        }

        rest = rest_if_prefix("sphere: ", line);
        if (rest != "") {
            Sphere* new_sphere = new Sphere(&entity_count, materials.back());
            new_sphere->Decode(rest);
            shapes.push_back(new_sphere);
        }

		rest = rest_if_prefix("triangle: ", line);
        if (rest != "") {
            Triangle* new_triangle = new Triangle(&entity_count, materials.back());
            new_triangle->Decode(rest);
            shapes.push_back(new_triangle);
        }

		rest = rest_if_prefix("normal_triangle: ", line);
        if (rest != "") {
            NormalTriangle* new_triangle = new NormalTriangle(&entity_count, materials.back());
            new_triangle->Decode(rest);
            shapes.push_back(new_triangle);
        }

        rest = rest_if_prefix("material: ", line);
        if (rest != "") {
            Material* new_mat = new Material(&entity_count);
            new_mat->Decode(rest);
            materials.push_back(new_mat);
        }

        rest = rest_if_prefix("ambient_light: ", line);
        if (rest != "") {
            AmbientLight* new_light = new AmbientLight(&entity_count);
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("directional_light: ", line);
        if (rest != "") {
            DirectionalLight* new_light = new DirectionalLight(&entity_count);
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("point_light: ", line);
        if (rest != "") {
            PointLight* new_light = new PointLight(&entity_count);
            new_light->Decode(rest);
            lights.push_back(new_light);
        }

        rest = rest_if_prefix("spot_light: ", line);
        if (rest != "") {
            SpotLight* new_light = new SpotLight(&entity_count);
            new_light->Decode(rest);
            lights.push_back(new_light);
        }
    }
    scene_file.close();

    UpdateCameraWidget();
	RequestRender();
}

void Save() {
    load_state = LoadState();
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


}  // namespace Raytracer