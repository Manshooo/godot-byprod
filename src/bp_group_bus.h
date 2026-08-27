// A byProd group bus — the mix group events are routed into (music, UI, SFX).
//
// Buses are owned by the loaded project, not by this object, so the wrapper only
// borrows the handle and never destroys anything.

#pragma once

#include "bp_api.h"

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class ByProdSoundManager;

class ByProdGroupBus : public RefCounted {
	GDCLASS(ByProdGroupBus, RefCounted)

protected:
	static void _bind_methods();

public:
	static Ref<ByProdGroupBus> wrap(const Ref<ByProdSoundManager> &p_owner, byprod::GroupBusHandle p_handle);

	ByProdGroupBus() = default;
	~ByProdGroupBus();

	void set_volume(float p_volume);
	float get_volume() const;

	bool is_valid() const { return handle != nullptr; }

private:
	// The bus belongs to the loaded project, so the manager has to outlive it.
	// Holding a reference is what enforces that: Godot frees objects in no
	// particular order, and a call into a destroyed manager crashes the runtime.
	Ref<ByProdSoundManager> owner;
	byprod::GroupBusHandle handle = nullptr;
};

} // namespace godot
