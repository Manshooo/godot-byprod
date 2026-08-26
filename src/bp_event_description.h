// The authored template of an event, from which playable instances are made.
//
// Descriptions belong to the loaded project and are looked up by path, so they
// are borrowed like group buses and outlive every instance created from them.
// Parameter indices come from here: resolving a name once and driving the
// instance by index avoids a string lookup per frame.

#pragma once

#include "bp_api.h"
#include "bp_event_instance.h"

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class ByProdEventDescription : public RefCounted {
	GDCLASS(ByProdEventDescription, RefCounted)

protected:
	static void _bind_methods();

public:
	static Ref<ByProdEventDescription> wrap(byprod::EventDescriptionHandle p_handle);

	Ref<ByProdEventInstance> create_instance();

	// Returns -1 when the event has no such parameter.
	int get_parameter_index(const String &p_name) const;
	int get_parameter_count() const;
	float get_length() const;

	bool is_valid() const { return handle != nullptr; }

private:
	byprod::EventDescriptionHandle handle = nullptr;
};

} // namespace godot
