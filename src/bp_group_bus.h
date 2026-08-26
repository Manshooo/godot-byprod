// A byProd group bus — the mix group events are routed into (music, UI, SFX).
//
// Buses are owned by the loaded project, not by this object, so the wrapper only
// borrows the handle and never destroys anything.

#pragma once

#include "bp_api.h"

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class ByProdGroupBus : public RefCounted {
	GDCLASS(ByProdGroupBus, RefCounted)

protected:
	static void _bind_methods();

public:
	static Ref<ByProdGroupBus> wrap(byprod::GroupBusHandle p_handle);

	void set_volume(float p_volume);
	float get_volume() const;

	bool is_valid() const { return handle != nullptr; }

private:
	byprod::GroupBusHandle handle = nullptr;
};

} // namespace godot
