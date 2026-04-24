#include "scene_io.h"

#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string_view> TokenizeLine(std::string& line) {
    if (const auto comment_pos = line.find('#'); comment_pos != std::string::npos) {
        line.resize(comment_pos);
    }

    std::vector<std::string_view> tokens;
    const char* cursor = line.data();
    const char* end = cursor + line.size();

    while (cursor < end && std::isspace(static_cast<unsigned char>(*cursor))) {
        ++cursor;
    }

    while (cursor < end) {
        const char* start = cursor;
        while (cursor < end && !std::isspace(static_cast<unsigned char>(*cursor))) {
            ++cursor;
        }
        if (start != cursor) {
            tokens.emplace_back(start, static_cast<size_t>(cursor - start));
        }
        while (cursor < end && std::isspace(static_cast<unsigned char>(*cursor))) {
            ++cursor;
        }
    }

    return tokens;
}

double ParseDouble(std::string_view token) {
    const std::string text(token);
    char* end = nullptr;
    return std::strtod(text.c_str(), &end);
}

int ParseInt(std::string_view token) {
    const std::string text(token);
    char* end = nullptr;
    return static_cast<int>(std::strtol(text.c_str(), &end, 10));
}

Vector ReadVec3(const std::vector<std::string_view>& tokens, size_t start = 1U) {
    if (tokens.size() < start + 3U) {
        throw std::runtime_error("Expected 3 vector components.");
    }
    return {ParseDouble(tokens[start + 0U]), ParseDouble(tokens[start + 1U]),
            ParseDouble(tokens[start + 2U])};
}

struct VertexRef {
    int vertex_index = 0;
    std::optional<int> normal_index;
};

VertexRef ParseVertexRef(std::string_view token) {
    VertexRef ref;

    const auto first_slash = token.find('/');
    if (first_slash == std::string_view::npos) {
        ref.vertex_index = ParseInt(token);
        return ref;
    }

    ref.vertex_index = ParseInt(token.substr(0, first_slash));
    token.remove_prefix(first_slash + 1U);

    const auto second_slash = token.find('/');
    if (second_slash == std::string_view::npos) {
        return ref;
    }

    token.remove_prefix(second_slash + 1U);
    if (!token.empty()) {
        ref.normal_index = ParseInt(token);
    }
    return ref;
}

template <typename T>
const T& ObjAt(const std::vector<T>& data, int index) {
    if (data.empty()) {
        throw std::runtime_error("OBJ index references an empty array.");
    }

    if (index > 0) {
        const auto i = static_cast<size_t>(index - 1);
        if (i >= data.size()) {
            throw std::runtime_error("OBJ index is out of range.");
        }
        return data[i];
    }

    const int64_t i = static_cast<int64_t>(data.size()) + index;
    if (i < 0 || static_cast<size_t>(i) >= data.size()) {
        throw std::runtime_error("Negative OBJ index is out of range.");
    }
    return data[static_cast<size_t>(i)];
}

void ReadMaterials(const std::filesystem::path& path, Scene* scene) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Cannot open materials file: " + path.string());
    }

    std::string current_name;
    Material current_material;

    const auto commit_material = [&]() {
        if (!current_name.empty()) {
            scene->AddMaterial(current_name, current_material);
            current_name.clear();
            current_material = Material{};
        }
    };

    std::string line;
    while (std::getline(input, line)) {
        const auto tokens = TokenizeLine(line);
        if (tokens.empty()) {
            continue;
        }

        const std::string_view key = tokens[0];
        if (key == "newmtl") {
            commit_material();
            if (tokens.size() >= 2U) {
                current_name = std::string(tokens[1]);
            }
        } else if (key == "Ka") {
            current_material.ambient_color = ReadVec3(tokens);
        } else if (key == "Kd") {
            current_material.diffuse_color = ReadVec3(tokens);
        } else if (key == "Ks") {
            current_material.specular_color = ReadVec3(tokens);
        } else if (key == "Ke") {
            current_material.intensity = ReadVec3(tokens);
        } else if (key == "Ns" && tokens.size() >= 2U) {
            current_material.specular_exponent = ParseDouble(tokens[1]);
        } else if (key == "Ni" && tokens.size() >= 2U) {
            current_material.refraction_index = ParseDouble(tokens[1]);
        } else if (key == "al") {
            current_material.albedo = ReadVec3(tokens);
        }
    }

    commit_material();
}

Triangle MakeTriangle(const std::array<Vector, 3>& vertices, const std::array<Vector, 3>& normals,
                      bool has_vertex_normals) {
    if (!has_vertex_normals) {
        return {vertices[0], vertices[1], vertices[2]};
    }
    return {vertices[0], vertices[1], vertices[2], normals[0], normals[1], normals[2]};
}

void AddFace(const std::vector<std::string_view>& tokens, const std::vector<Vector>& vertices,
             const std::vector<Vector>& normals, int material_id, Scene* scene) {
    if (tokens.size() < 4U) {
        return;
    }

    std::array<Vector, 3> face_vertices{};
    std::array<Vector, 3> face_normals{};
    bool has_vertex_normals = false;

    const auto fill_vertex = [&](std::string_view slot, size_t index) {
        const VertexRef ref = ParseVertexRef(slot);
        face_vertices[index] = ObjAt(vertices, ref.vertex_index);
        if (ref.normal_index.has_value()) {
            face_normals[index] = ObjAt(normals, *ref.normal_index);
            has_vertex_normals = true;
        } else {
            face_normals[index] = Vector(0.0, 0.0, 0.0);
        }
    };

    fill_vertex(tokens[1], 0U);
    fill_vertex(tokens[2], 1U);
    for (size_t i = 3U; i < tokens.size(); ++i) {
        fill_vertex(tokens[i], 2U);
        scene->AddTriangle(MakeTriangle(face_vertices, face_normals, has_vertex_normals),
                           material_id);
        face_vertices[1] = face_vertices[2];
        face_normals[1] = face_normals[2];
    }
}

Scene ReadScene(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Cannot open scene file: " + path.string());
    }

    std::vector<Vector> vertices;
    std::vector<Vector> normals;
    Scene scene;
    std::optional<int> current_material_id;

    std::string line;
    while (std::getline(input, line)) {
        const auto tokens = TokenizeLine(line);
        if (tokens.empty()) {
            continue;
        }

        const std::string_view tag = tokens[0];
        if (tag == "mtllib") {
            if (tokens.size() >= 2U) {
                auto material_path = path;
                material_path.replace_filename(tokens[1]);
                ReadMaterials(material_path, &scene);
            }
        } else if (tag == "usemtl") {
            if (tokens.size() < 2U) {
                throw std::runtime_error("usemtl requires a material name.");
            }
            current_material_id = scene.FindMaterialId(std::string(tokens[1]));
            if (!current_material_id.has_value()) {
                throw std::runtime_error("Unknown material: " + std::string(tokens[1]));
            }
        } else if (tag == "v") {
            vertices.push_back(ReadVec3(tokens));
        } else if (tag == "vn") {
            normals.push_back(ReadVec3(tokens));
        } else if (tag == "f") {
            if (!current_material_id.has_value()) {
                throw std::runtime_error("Face encountered before usemtl.");
            }
            AddFace(tokens, vertices, normals, *current_material_id, &scene);
        } else if (tag == "S") {
            if (!current_material_id.has_value()) {
                throw std::runtime_error("Sphere encountered before usemtl.");
            }
            if (tokens.size() < 5U) {
                throw std::runtime_error("Sphere requires x y z radius.");
            }
            scene.AddSphere(Sphere(ReadVec3(tokens), ParseDouble(tokens[4])), *current_material_id);
        } else if (tag == "P") {
            if (tokens.size() < 7U) {
                throw std::runtime_error("Light requires position and intensity.");
            }
            scene.AddLight(Light(ReadVec3(tokens), ReadVec3(tokens, 4U)));
        }
    }

    return scene;
}
