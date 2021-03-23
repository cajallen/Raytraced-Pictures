#ifndef _RAYTRACER_OBJECT_H
#define _RAYTRACER_OBJECT_H

#include <string>

using namespace std;

namespace Raytracer {

// Non-enforced abstract class for raytracer objects. These are both UI objects,
// and entities.
struct Object {
    int id;

    // Call with entity_count to new
    Object(int* entity_count);
    // Call with id to replace
    Object(int old_id);

    virtual void ImGui() {}
	// String representation for saving.
    virtual string Encode() { return ""; }
    virtual void Decode(string& s) {}
	// Called once right before rendering, allows objects to finalize intermediate data
	virtual void PreRender() {}

    string WithId(string s);
    bool operator==(Object rhs) { return id == rhs.id; }
};

}  // namespace Raytracer

#endif
