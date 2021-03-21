#include "raytracer_main.h"

#define UI_HSPACING 4
#define TAB_SIZE 6.0

float cameraView[16] = { 1.f, 0.f, 0.f, 0.f,
                 0.f, 1.f, 0.f, 0.f,
                 0.f, 0.f, 1.f, 0.f,
                 0.f, 0.f, 0.f, 1.f };
float cameraDistance = 8.0f;
bool print_debug = false;
namespace P3 {

string Object::WithId(string s) {
    return s + std::to_string(id);
}

void Camera::PreRender() {
	mid_res = { res[X] / 2, res[Y] / 2 };

	forward = forward.normalized();
	right = cross(forward, up).normalized();
	up = cross(right, forward).normalized();
}

// assumes normalized
void ForwardUpToMatrix(float* mat, vec3& forward, vec3& up) {
    vec3 right = cross(forward, up);
    for (int i = 0; i < 3; i++) {
        mat[0 + (i * 4)] = right[i];
        mat[1 + (i * 4)] = up[i];
        mat[2 + (i * 4)] = forward[i];
        mat[3 + (i * 4)] = 0; // right row
        mat[12 + i] = 0; // bottom row
    }
    mat[15] = 1;
}

void UpdateCameraWidget() {
    ForwardUpToMatrix(cameraView, camera->forward, camera->up);
};

void MatrixToForwardUp(float* mat, vec3& forward, vec3& up) {
	for (int i = 0; i < 3; i++) {
        up[i] = mat[1+(i*4)];
        forward[i] = mat[2+(i*4)];
	}
}

void Camera::ImGui() {
	bool updated = false;
    if (ImGui::CollapsingHeader("Camera Parameters")) {
        ImGuiIO& io = ImGui::GetIO();

        ImGuizmo::SetDrawlist();
        float viewManipulateRight = ImGui::GetWindowPos().x + (float)ImGui::GetWindowWidth();
        float viewManipulateTop = ImGui::GetWindowPos().y + ImGui::GetItemRectMax().y - ImGui::GetWindowPos().y;
        updated |= ImGuizmo::ViewManipulate(cameraView, cameraDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);
        MatrixToForwardUp(cameraView, forward, up);
        ImGui::DragDoubleN("camera_forward", &forward.x, 3, 0.0, 0.0, 0.0, "%.2f", ImGuiSliderFlags_NoInput);
        ImGui::DragDoubleN("camera_up", &up.x, 3, 0.0, 0.0, 0.0, "%.2f", ImGuiSliderFlags_NoInput);

        updated |= ImGui::DragDoubleN("Camera Pos", &position.x, 3);
        updated |= ImGui::SliderFloat("FOV", &half_vfov, 0.0, 180.0);
        updated |= ImGui::DragInt2("Resolution", &res[0], 1);
        updated |= ImGui::SliderInt("Max Depth", &max_depth, 1, 8);
        updated |= ImGui::ColorEdit3("Background Color", &background_color.r);
    }
	if (updated) {
		RequestRender();
	}
}

void Geometry::ImGui() {
    ImGui::Indent(TAB_SIZE);
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
    ImGui::Unindent(TAB_SIZE);
}

void Sphere::ImGui() {
	bool updated = false;
    ImGui::Indent(TAB_SIZE);
    if (ImGui::CollapsingHeader(WithId("Sphere ").c_str())) {
        updated |= ImGui::DragDoubleN(WithId("pos##").c_str(), &position.x, 3);
        updated |= ImGui::DragFloat(WithId("radius##").c_str(), &radius, 0.01, 0.01);
        ImGui::Indent(3.0);
        material->ImGui();
        ImGui::Unindent(3.0);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(TAB_SIZE);
	if (updated) RequestRender();
}

void Triangle::ImGui() {
	bool updated = false;
	ImGui::Indent(TAB_SIZE);
	if (ImGui::CollapsingHeader(WithId("Triangle ").c_str())) {
		updated |= ImGui::DragDoubleN(WithId("pos1##").c_str(), &v1.x, 3, 0.05);
		updated |= ImGui::DragDoubleN(WithId("pos2##").c_str(), &v2.x, 3, 0.05);
		updated |= ImGui::DragDoubleN(WithId("pos3##").c_str(), &v3.x, 3, 0.05);
        ImGui::Indent(3.0);
        material->ImGui();
        ImGui::Unindent(3.0);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
	}
	ImGui::Unindent(TAB_SIZE);
	if (updated) RequestRender();
}

void NormalTriangle::ImGui() {
	bool updated = false;
	ImGui::Indent(TAB_SIZE);
	if (ImGui::CollapsingHeader(WithId("NormTriangle ").c_str())) {
		updated |= ImGui::DragDoubleN(WithId("pos1##").c_str(), &v1.x, 3, 0.05);
		updated |= ImGui::DragDoubleN(WithId("pos2##").c_str(), &v2.x, 3, 0.05);
		updated |= ImGui::DragDoubleN(WithId("pos3##").c_str(), &v3.x, 3, 0.05);
		updated |= ImGui::DragDoubleN(WithId("norm1##").c_str(), &n1.x, 3, 0.05);
		updated |= ImGui::DragDoubleN(WithId("norm2##").c_str(), &n2.x, 3, 0.05);
		updated |= ImGui::DragDoubleN(WithId("norm3##").c_str(), &n3.x, 3, 0.05);
        ImGui::Indent(3.0);
        material->ImGui();
        ImGui::Unindent(3.0);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
	}
	ImGui::Unindent(TAB_SIZE);
	if (updated) RequestRender();
}

void Material::ImGui() {
	bool updated = false;
    if (ImGui::CollapsingHeader(WithId("Material ").c_str())) {
        ImGui::Indent(4.0);
        updated |= ImGui::ColorEdit3(WithId("ambient##").c_str(), &ambient.r);
        updated |= ImGui::ColorEdit3(WithId("diffuse##").c_str(), &diffuse.r);
        updated |= ImGui::ColorEdit3(WithId("specular##").c_str(), &specular.r);
        updated |= ImGui::ColorEdit3(WithId("transmissive##").c_str(), &transmissive.r);
        updated |= ImGui::DragFloat(WithId("phong##").c_str(), &phong, 1, 2.0, 128.0, "%.1f", ImGuiSliderFlags_Logarithmic);
        updated |= ImGui::DragFloat(WithId("ior##").c_str(), &ior, 0.1f);
        ImGui::Unindent(4.0);
    }
	if (updated) RequestRender();
}

void Light::ImGui() {
    ImGui::Indent(TAB_SIZE);
    if (ImGui::CollapsingHeader(WithId("Light ").c_str())) {
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(TAB_SIZE);
}

void AmbientLight::ImGui() {
    bool updated = false;
    ImGui::Indent(TAB_SIZE);
    if (ImGui::CollapsingHeader(WithId("Ambient ").c_str())) {
        updated |= ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.05);
        updated |= ImGui::ColorEdit3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(TAB_SIZE);
	if (updated) RequestRender();
}

void PointLight::ImGui() {
    bool updated = false;
    ImGui::Indent(TAB_SIZE);
    if (ImGui::CollapsingHeader(WithId("Point ").c_str())) {
        updated |= ImGui::DragDoubleN(WithId("position##").c_str(), &position.x, 3);
        updated |= ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01, 1000.0);
        updated |= ImGui::ColorEdit3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(TAB_SIZE);
	if (updated) RequestRender();
}

void SpotLight::ImGui() {
    bool updated = false;
    ImGui::Indent(TAB_SIZE);
    if (ImGui::CollapsingHeader(WithId("Spot ").c_str())) {
        updated |= ImGui::DragDoubleN(WithId("position##").c_str(), &position.x, 3);
        updated |= ImGui::DragDoubleN(WithId("direction##").c_str(), &direction.x, 3, 0.01, -1, 1);
        updated |= ImGui::SliderFloat(WithId("interior_angle##").c_str(), &angle1, 0, 90);
        angle2 = max(angle1, angle2);
        updated |= ImGui::SliderFloat(WithId("exterior_angle##").c_str(), &angle2, angle1, 90);
        updated |= ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01);
        updated |= ImGui::ColorEdit3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
        direction = direction.normalized();
    }
    ImGui::Unindent(TAB_SIZE);
	if (updated) RequestRender();
}

void DirectionalLight::ImGui() {
    bool updated = false;
    ImGui::Indent(TAB_SIZE);
    if (ImGui::CollapsingHeader(WithId("Directional ").c_str())) {
        updated |= ImGui::DragFloat(WithId("Multiplier").c_str(), &mult, 0.05, 0.01);
        updated |= ImGui::DragDoubleN(WithId("direction##").c_str(), &direction.x, 3, 0.05, -1.0, 1.0);
        updated |= ImGui::ColorEdit3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
        direction = direction.normalized();
    }
    ImGui::Unindent(TAB_SIZE);
	if (updated) RequestRender();
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE);  // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // Same

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

void Log(string s) {
    debug_log.push_back(s);
    if (debug_log.size() > 100) {
        debug_log.erase(debug_log.begin(), debug_log.begin() + 20);
    }
}

void DisplayLog() {
    ImGui::Checkbox("Show Debug Log", &print_debug);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) debug_log.clear();
    if (print_debug) {
        ImGui::BeginChild("Log");
        for (string s : debug_log) {
            ImGui::Text(s.c_str());
        }
        ImGui::EndChild();
    }
}


}  // namespace P3
