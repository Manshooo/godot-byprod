// One playing (or about to play) byProd event.
//
// Instances are explicitly owned: the runtime keeps them alive until released,
// so the destructor releases the handle rather than leaking a voice when the
// last GDScript reference goes away. release_when_finished() hands that
// ownership back to the runtime for fire-and-forget one-shots.

#pragma once

#include "bp_api.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

class ByProdSoundManager;

class ByProdEventInstance : public RefCounted {
	GDCLASS(ByProdEventInstance, RefCounted)

protected:
	static void _bind_methods();

public:
	enum State {
		STATE_STOPPED = byprod::EVENT_INSTANCE_STOPPED,
		STATE_PLAYING = byprod::EVENT_INSTANCE_PLAYING,
		STATE_PAUSED = byprod::EVENT_INSTANCE_PAUSED,
		STATE_FINISHED = byprod::EVENT_INSTANCE_FINISHED,
	};

	static Ref<ByProdEventInstance> wrap(const Ref<ByProdSoundManager> &p_owner, byprod::EventInstanceHandle p_handle);

	ByProdEventInstance() = default;
	~ByProdEventInstance();

	void start();
	void stop();
	void pause();
	void unpause();
	void release();
	void release_when_finished();

	void fade_in(float p_duration);
	void release_after_fade_out(float p_duration);

	State get_state() const;
	float get_time() const;

	void set_parameter(const String &p_name, float p_value);
	void set_parameter_by_index(int p_index, float p_value);
	float get_parameter_by_index(int p_index) const;
	void send_signal(const String &p_signal);

	void set_volume_multiplier(float p_volume);
	float get_volume_multiplier() const;

	// Subscribes the instance to the manager's tick level: it pauses at p_level and
	// below, and resumes above it. Off by default, so ByProdSoundManager's
	// set_tick_level() moves nothing until instances opt in here. Must be called
	// before release_when_finished(), which drops the handle this needs.
	void set_auto_pause(bool p_enabled, int p_level);

	void set_3d_attributes(const Vector3 &p_position, const Vector3 &p_velocity);

	bool is_valid() const { return handle != nullptr; }

private:
	// Releasing an instance calls into the manager that created it, so the manager
	// must not be freed first — which Godot gives no ordering guarantee about.
	Ref<ByProdSoundManager> owner;
	byprod::EventInstanceHandle handle = nullptr;
};

} // namespace godot

VARIANT_ENUM_CAST(ByProdEventInstance::State);
