#include "raytracer_object.h"

namespace Raytracer {
	
string Object::WithId(string s) {
    return s + std::to_string(id);
}

Object::Object(int* entity_count) {
    id = *entity_count;
    *entity_count++;
}

Object::Object(int old_id) {
    id = old_id;
}

}  // namespace Raytracer
