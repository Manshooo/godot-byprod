#include "bp_sound_manager.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <cstring>

#include <string>
#include <vector>

using namespace godot;

namespace {

// Set by the last failed create(), read back through get_last_error().
//
// Deliberately a std::string and not a godot::String: a global of a Godot type is
// constructed while the DLL is being loaded, before the extension has been handed
// the engine's function pointers, and calling into them there fails the library's
// initialization outright (Windows error 1114).
std::string g_last_error;

} // namespace

void ByProdSoundManager::_bind_methods() {
	ClassDB::bind_static_method("ByProdSoundManager", D_METHOD("create"), &ByProdSoundManager::create);
	ClassDB::bind_static_method("ByProdSoundManager", D_METHOD("create_host_mixed", "sample_rate"), &ByProdSoundManager::create_host_mixed);
	ClassDB::bind_static_method("ByProdSoundManager", D_METHOD("get_last_error"), &ByProdSoundManager::get_last_error);
	ClassDB::bind_static_method("ByProdSoundManager", D_METHOD("is_runtime_available"), &ByProdSoundManager::is_runtime_available);
	ClassDB::bind_static_method("ByProdSoundManager", D_METHOD("get_runtime_version"), &ByProdSoundManager::get_runtime_version);

	ClassDB::bind_method(D_METHOD("load_project", "bytes"), &ByProdSoundManager::load_project);
	ClassDB::bind_method(D_METHOD("set_bank_directory", "directory"), &ByProdSoundManager::set_bank_directory);
	ClassDB::bind_method(D_METHOD("get_bank_directory"), &ByProdSoundManager::get_bank_directory);
	ClassDB::bind_method(D_METHOD("preload_bank", "name"), &ByProdSoundManager::preload_bank);
	ClassDB::bind_method(D_METHOD("get_event_description", "path"), &ByProdSoundManager::get_event_description);
	ClassDB::bind_method(D_METHOD("update"), &ByProdSoundManager::update);
	ClassDB::bind_method(D_METHOD("mix", "frame_count"), &ByProdSoundManager::mix);
	ClassDB::bind_method(D_METHOD("set_listener_transform", "position", "forward", "up"), &ByProdSoundManager::set_listener_transform);
	ClassDB::bind_method(D_METHOD("set_global_volume", "volume"), &ByProdSoundManager::set_global_volume);
	ClassDB::bind_method(D_METHOD("get_global_volume"), &ByProdSoundManager::get_global_volume);
	ClassDB::bind_method(D_METHOD("set_tick_level", "tick_level"), &ByProdSoundManager::set_tick_level);
	ClassDB::bind_method(D_METHOD("get_master_group_bus"), &ByProdSoundManager::get_master_group_bus);
	ClassDB::bind_method(D_METHOD("get_group_bus", "path"), &ByProdSoundManager::get_group_bus);
	ClassDB::bind_method(D_METHOD("is_valid"), &ByProdSoundManager::is_valid);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "global_volume"), "set_global_volume", "get_global_volume");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "bank_directory", PROPERTY_HINT_DIR), "set_bank_directory", "get_bank_directory");

	BIND_ENUM_CONSTANT(TICK_LEVEL_NONE);
	BIND_ENUM_CONSTANT(TICK_LEVEL_PARTIAL);
	BIND_ENUM_CONSTANT(TICK_LEVEL_FULL);
}

Ref<ByProdSoundManager> ByProdSoundManager::create_with_flags(uint32_t p_flags, uint32_t p_sample_rate) {
	Ref<ByProdSoundManager> manager;

	const byprod::Api *api = byprod::api();
	if (api == nullptr) {
		g_last_error = byprod::load_error();
		return manager;
	}

	// The settings struct is filled by the runtime into an over-sized zeroed
	// buffer: an SDK that grew the struct writes into the slack instead of past
	// the end of the version this wrapper was compiled against.
	alignas(8) uint8_t settings_storage[256] = {};
	api->bpdSoundManagerSettingsInit(settings_storage);

	byprod::SoundManagerSettings *settings = reinterpret_cast<byprod::SoundManagerSettings *>(settings_storage);

	// Godot's 3D space is right-handed; byProd defaults to left-handed, which would
	// silently mirror every panned sound.
	settings->flags = p_flags | byprod::SOUND_MANAGER_RIGHT_HANDED_3D;
	settings->environment = Engine::get_singleton()->is_editor_hint()
			? byprod::RUNTIME_ENVIRONMENT_PREVIEW_IN_EDITOR
			: byprod::RUNTIME_ENVIRONMENT_APP;
	if (p_sample_rate > 0) {
		settings->sample_rate = p_sample_rate;
	}

	byprod::SoundManagerHandle created = api->bpdSoundManagerCreate(settings_storage);
	if (created == nullptr) {
		g_last_error = "byProd refused to create a sound manager with these settings.";
		return manager;
	}

	manager.instantiate();
	manager->handle = created;

	// Installed up front rather than from set_bank_directory(): the runtime only
	// asks for banks it needs, and an unset directory answers "no such bank" —
	// which is a readable byProd error instead of a silent project with no audio.
	api->bpdSoundManagerSetSoundBankCallbacks(created,
			&ByProdSoundManager::bank_get_callback,
			&ByProdSoundManager::bank_release_callback,
			manager.ptr());

	g_last_error.clear();
	return manager;
}

Ref<ByProdSoundManager> ByProdSoundManager::create() {
	return create_with_flags(byprod::SOUND_MANAGER_NONE, 0);
}

Ref<ByProdSoundManager> ByProdSoundManager::create_host_mixed(int p_sample_rate) {
	// Host-mixed has no device to ask for a rate, so a real one is mandatory here.
	const uint32_t rate = p_sample_rate > 0 ? static_cast<uint32_t>(p_sample_rate) : 44100u;
	return create_with_flags(byprod::SOUND_MANAGER_HOST_MIXED, rate);
}

String ByProdSoundManager::get_last_error() {
	// A failed create() is the interesting error only once the library is up; while
	// it is not, the loader's own message (what was tried, or which symbol is
	// missing) is what the caller actually needs to see.
	if (byprod::api() == nullptr) {
		return String(byprod::load_error());
	}
	return String(g_last_error.c_str());
}

bool ByProdSoundManager::is_runtime_available() {
	return byprod::api() != nullptr;
}

int ByProdSoundManager::get_runtime_version() {
	const byprod::Api *api = byprod::api();
	return api == nullptr ? 0 : static_cast<int>(api->bpdVersion());
}

ByProdSoundManager::~ByProdSoundManager() {
	const byprod::Api *api = byprod::api();
	if (api != nullptr && handle != nullptr) {
		api->bpdSoundManagerDestroy(handle);
	}
	handle = nullptr;
}

bool ByProdSoundManager::load_project(const PackedByteArray &p_bytes) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr || p_bytes.is_empty()) {
		return false;
	}
	return api->bpdSoundManagerLoadProject(handle, p_bytes.ptr(), static_cast<size_t>(p_bytes.size())) != 0;
}

int32_t ByProdSoundManager::bank_get_callback(const char *p_name, byprod::SoundBankData *r_data, void *p_user) {
	ByProdSoundManager *self = static_cast<ByProdSoundManager *>(p_user);
	if (self == nullptr || self->bank_directory.is_empty()) {
		return 0;
	}

	const String path = self->bank_directory.path_join(String(p_name) + ".bybank");
	if (!FileAccess::file_exists(path)) {
		return 0;
	}

	const PackedByteArray bytes = FileAccess::get_file_as_bytes(path);
	if (bytes.is_empty()) {
		return 0;
	}

	// Handed over without a copy (copy_data = 0), so the buffer has to outlive the
	// call and is owned here until the matching release. A PackedByteArray cannot
	// do that safely across threads, hence the raw array.
	uint8_t *owned = new uint8_t[bytes.size()];
	memcpy(owned, bytes.ptr(), bytes.size());

	r_data->bytes = owned;
	r_data->length = static_cast<uint32_t>(bytes.size());
	r_data->copy_data = 0;
	return 1;
}

void ByProdSoundManager::bank_release_callback(const char *p_name, const byprod::SoundBankData *p_data, void *p_user) {
	(void)p_name;
	(void)p_user;
	delete[] static_cast<const uint8_t *>(p_data->bytes);
}

void ByProdSoundManager::set_bank_directory(const String &p_directory) {
	bank_directory = p_directory;
}

String ByProdSoundManager::get_bank_directory() const {
	return bank_directory;
}

bool ByProdSoundManager::preload_bank(const String &p_name) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return false;
	}
	return api->bpdSoundManagerPreloadSoundBank(handle, p_name.utf8().get_data()) != 0;
}

Ref<ByProdEventDescription> ByProdSoundManager::get_event_description(const String &p_path) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return Ref<ByProdEventDescription>();
	}
	return ByProdEventDescription::wrap(api->bpdSoundManagerGetEventDescription(handle, p_path.utf8().get_data()));
}

void ByProdSoundManager::update() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdSoundManagerUpdate(handle);
}

PackedVector2Array ByProdSoundManager::mix(int p_frame_count) {
	PackedVector2Array frames;

	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr || p_frame_count <= 0) {
		return frames;
	}

	// byProd renders interleaved stereo floats; Godot's generator playback wants
	// one Vector2 per frame, so the conversion happens here rather than in script.
	std::vector<float> interleaved(static_cast<size_t>(p_frame_count) * 2, 0.0f);
	api->bpdSoundManagerMix(handle, interleaved.data(), static_cast<uint32_t>(p_frame_count));

	frames.resize(p_frame_count);
	Vector2 *out = frames.ptrw();
	for (int i = 0; i < p_frame_count; i++) {
		out[i] = Vector2(interleaved[i * 2], interleaved[i * 2 + 1]);
	}
	return frames;
}

void ByProdSoundManager::set_listener_transform(const Vector3 &p_position, const Vector3 &p_forward, const Vector3 &p_up) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdSoundManagerSetListenerTransform(handle,
			p_position.x, p_position.y, p_position.z,
			p_forward.x, p_forward.y, p_forward.z,
			p_up.x, p_up.y, p_up.z);
}

void ByProdSoundManager::set_global_volume(float p_volume) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdSoundManagerSetGlobalVolume(handle, p_volume);
}

float ByProdSoundManager::get_global_volume() const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return 0.0f;
	}
	return api->bpdSoundManagerGetGlobalVolume(handle);
}

void ByProdSoundManager::set_tick_level(int p_tick_level) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdSoundManagerSetTickLevel(handle, static_cast<uint32_t>(p_tick_level));
}

Ref<ByProdGroupBus> ByProdSoundManager::get_master_group_bus() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return Ref<ByProdGroupBus>();
	}
	return ByProdGroupBus::wrap(api->bpdSoundManagerGetMasterGroupBus(handle));
}

Ref<ByProdGroupBus> ByProdSoundManager::get_group_bus(const String &p_path) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return Ref<ByProdGroupBus>();
	}
	return ByProdGroupBus::wrap(api->bpdSoundManagerGetGroupBus(handle, p_path.utf8().get_data()));
}
