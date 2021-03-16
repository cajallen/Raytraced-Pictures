#include "raytracer_main.h"

#define UI_HSPACING 4

namespace P3 {

string Object::WithId(string s) {
    return s + std::to_string(id);
}


void Camera::ImGui() {
    if (ImGui::CollapsingHeader("Camera Parameters")) {
        ImGui::DragFloat3("Camera Position", &position.x, 0.1);
        ImGui::SliderFloat3("Camera Forward", &forward.x, -1.0f, 1.0f);
        ImGui::SliderFloat3("Camera Up", &up.x, -1.0f, 1.0f);
        ImGui::SliderFloat("FOV", &half_vfov, 0.0f, 180.0f);
        ImGui::DragInt2("Resolution", &res[0], 1);
        ImGui::SliderInt("Max Depth", &max_depth, 1, 8);
        ImGui::ColorEdit3("Background Color", &background_color.r);
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
        ImGui::ColorEdit3(WithId("ambient##").c_str(), &ambient.r);
        ImGui::ColorEdit3(WithId("diffuse##").c_str(), &diffuse.r);
        ImGui::ColorEdit3(WithId("specular##").c_str(), &specular.r);
        ImGui::ColorEdit3(WithId("transmissive##").c_str(), &transmissive.r);
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
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.05);
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
        ImGui::DragFloat(WithId("Multiplier##").c_str(), &mult, 0.05, 0.01, 1000.0);
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
        ImGui::ColorEdit3(WithId("color##").c_str(), &color.r);
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
        ImGui::ColorEdit3(WithId("color##").c_str(), &color.r);
        if (ImGui::Button(WithId("Delete##").c_str())) {
            Delete();
        }
    }
    ImGui::Unindent(6.0f);
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

}  // namespace P3
