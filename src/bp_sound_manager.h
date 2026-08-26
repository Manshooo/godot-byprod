// The byProd runtime instance: owns the loaded project, the listener and the mix.
//
// Two creation modes. create() lets byProd open its own audio device and run its
// own mixer thread — the simple path, where Godot never sees the samples.
// create_host_mixed() opens no device and renders only when Mix() is called,
// which is what ByProdStreamPump uses to push audio through Godot's own buses.

#pragma once

#include "bp_api.h"
#include "bp_event_description.h"
#include "bp_group_bus.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

class ByProdSoundManager : public RefCounted {
	GDCLASS(ByProdSoundManager, RefCounted)

protected:
	static void _bind_methods();

public:
	// How much of the world is being simulated; events can auto-pause per level.
	enum TickLevel {
		TICK_LEVEL_NONE = byprod::TICK_LEVEL_NONE,
		TICK_LEVEL_PARTIAL = byprod::TICK_LEVEL_PARTIAL,
		TICK_LEVEL_FULL = byprod::TICK_LEVEL_FULL,
	};

	static Ref<ByProdSoundManager> create();
	static Ref<ByProdSoundManager> create_host_mixed(int p_sample_rate);

	// Empty while the last create() succeeded; otherwise why it did not.
	static String get_last_error();

	// True when the native runtime could be found and fully resolved.
	static bool is_runtime_available();
	static int get_runtime_version();

	ByProdSoundManager() = default;
	~ByProdSoundManager();

	// Takes the bytes of a project compiled by the byProd editor.
	bool load_project(const PackedByteArray &p_bytes);

	Ref<ByProdEventDescription> get_event_description(const String &p_path);

	// Must be called once per frame from the game thread.
	void update();

	// Host-mixed only: renders p_frame_count stereo frames.
	PackedVector2Array mix(int p_frame_count);

	void set_listener_transform(const Vector3 &p_position, const Vector3 &p_forward, const Vector3 &p_up);

	void set_global_volume(float p_volume);
	float get_global_volume() const;

	void set_tick_level(int p_tick_level);

	Ref<ByProdGroupBus> get_master_group_bus();
	Ref<ByProdGroupBus> get_group_bus(const String &p_path);

	bool is_valid() const { return handle != nullptr; }

	byprod::SoundManagerHandle get_handle() const { return handle; }

private:
	static Ref<ByProdSoundManager> create_with_flags(uint32_t p_flags, uint32_t p_sample_rate);

	byprod::SoundManagerHandle handle = nullptr;
};

} // namespace godot

VARIANT_ENUM_CAST(ByProdSoundManager::TickLevel);
