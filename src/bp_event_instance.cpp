#include "bp_event_instance.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ByProdEventInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start"), &ByProdEventInstance::start);
	ClassDB::bind_method(D_METHOD("stop"), &ByProdEventInstance::stop);
	ClassDB::bind_method(D_METHOD("pause"), &ByProdEventInstance::pause);
	ClassDB::bind_method(D_METHOD("unpause"), &ByProdEventInstance::unpause);
	ClassDB::bind_method(D_METHOD("release"), &ByProdEventInstance::release);
	ClassDB::bind_method(D_METHOD("release_when_finished"), &ByProdEventInstance::release_when_finished);

	ClassDB::bind_method(D_METHOD("fade_in", "duration"), &ByProdEventInstance::fade_in);
	ClassDB::bind_method(D_METHOD("release_after_fade_out", "duration"), &ByProdEventInstance::release_after_fade_out);

	ClassDB::bind_method(D_METHOD("get_state"), &ByProdEventInstance::get_state);
	ClassDB::bind_method(D_METHOD("get_time"), &ByProdEventInstance::get_time);

	ClassDB::bind_method(D_METHOD("set_parameter", "name", "value"), &ByProdEventInstance::set_parameter);
	ClassDB::bind_method(D_METHOD("set_parameter_by_index", "index", "value"), &ByProdEventInstance::set_parameter_by_index);
	ClassDB::bind_method(D_METHOD("get_parameter_by_index", "index"), &ByProdEventInstance::get_parameter_by_index);
	ClassDB::bind_method(D_METHOD("send_signal", "signal_name"), &ByProdEventInstance::send_signal);

	ClassDB::bind_method(D_METHOD("set_volume_multiplier", "volume"), &ByProdEventInstance::set_volume_multiplier);
	ClassDB::bind_method(D_METHOD("get_volume_multiplier"), &ByProdEventInstance::get_volume_multiplier);

	ClassDB::bind_method(D_METHOD("set_3d_attributes", "position", "velocity"), &ByProdEventInstance::set_3d_attributes);
	ClassDB::bind_method(D_METHOD("is_valid"), &ByProdEventInstance::is_valid);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "volume_multiplier"), "set_volume_multiplier", "get_volume_multiplier");

	BIND_ENUM_CONSTANT(STATE_STOPPED);
	BIND_ENUM_CONSTANT(STATE_PLAYING);
	BIND_ENUM_CONSTANT(STATE_PAUSED);
	BIND_ENUM_CONSTANT(STATE_FINISHED);
}

Ref<ByProdEventInstance> ByProdEventInstance::wrap(byprod::EventInstanceHandle p_handle) {
	Ref<ByProdEventInstance> instance;
	if (p_handle == nullptr) {
		return instance;
	}
	instance.instantiate();
	instance->handle = p_handle;
	return instance;
}

ByProdEventInstance::~ByProdEventInstance() {
	release();
}

void ByProdEventInstance::start() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceStart(handle);
}

void ByProdEventInstance::stop() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceStop(handle);
}

void ByProdEventInstance::pause() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstancePause(handle);
}

void ByProdEventInstance::unpause() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceUnpause(handle);
}

void ByProdEventInstance::release() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		handle = nullptr;
		return;
	}
	api->bpdEventInstanceRelease(handle);
	handle = nullptr;
}

void ByProdEventInstance::release_when_finished() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		handle = nullptr;
		return;
	}
	// Ownership moves to the runtime, so the handle is dropped here without a
	// second release — the destructor must not touch it again.
	api->bpdEventInstanceReleaseWhenFinished(handle);
	handle = nullptr;
}

void ByProdEventInstance::fade_in(float p_duration) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceFadeIn(handle, p_duration);
}

void ByProdEventInstance::release_after_fade_out(float p_duration) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		handle = nullptr;
		return;
	}
	api->bpdEventInstanceReleaseAfterFadeOut(handle, p_duration);
	handle = nullptr;
}

ByProdEventInstance::State ByProdEventInstance::get_state() const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return STATE_STOPPED;
	}
	return static_cast<State>(api->bpdEventInstanceGetState(handle));
}

float ByProdEventInstance::get_time() const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return 0.0f;
	}
	return api->bpdEventInstanceGetTime(handle);
}

void ByProdEventInstance::set_parameter(const String &p_name, float p_value) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceSetParameterByName(handle, p_name.utf8().get_data(), p_value);
}

void ByProdEventInstance::set_parameter_by_index(int p_index, float p_value) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr || p_index < 0) {
		return;
	}
	api->bpdEventInstanceSetParameterByIndex(handle, static_cast<uint32_t>(p_index), p_value);
}

float ByProdEventInstance::get_parameter_by_index(int p_index) const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr || p_index < 0) {
		return 0.0f;
	}
	return api->bpdEventInstanceGetParameterByIndex(handle, static_cast<uint32_t>(p_index));
}

void ByProdEventInstance::send_signal(const String &p_signal) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceSendSignal(handle, p_signal.utf8().get_data());
}

void ByProdEventInstance::set_volume_multiplier(float p_volume) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceSetVolumeMultiplier(handle, p_volume);
}

float ByProdEventInstance::get_volume_multiplier() const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return 1.0f;
	}
	return api->bpdEventInstanceGetVolumeMultiplier(handle);
}

void ByProdEventInstance::set_3d_attributes(const Vector3 &p_position, const Vector3 &p_velocity) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdEventInstanceSet3DAttributes(handle,
			p_position.x, p_position.y, p_position.z,
			p_velocity.x, p_velocity.y, p_velocity.z);
}
