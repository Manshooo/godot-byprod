#include "bp_stream_pump.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ByProdStreamPump::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_sound_manager", "manager"), &ByProdStreamPump::set_sound_manager);
	ClassDB::bind_method(D_METHOD("get_sound_manager"), &ByProdStreamPump::get_sound_manager);
	ClassDB::bind_method(D_METHOD("set_player_path", "path"), &ByProdStreamPump::set_player_path);
	ClassDB::bind_method(D_METHOD("get_player_path"), &ByProdStreamPump::get_player_path);
	ClassDB::bind_method(D_METHOD("set_max_chunk_frames", "frames"), &ByProdStreamPump::set_max_chunk_frames);
	ClassDB::bind_method(D_METHOD("get_max_chunk_frames"), &ByProdStreamPump::get_max_chunk_frames);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "player_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AudioStreamPlayer"),
			"set_player_path", "get_player_path");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_chunk_frames", PROPERTY_HINT_RANGE, "64,8192,1"),
			"set_max_chunk_frames", "get_max_chunk_frames");
}

void ByProdStreamPump::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		set_process(false);
		return;
	}
	resolve_playback();
	set_process(true);
}

void ByProdStreamPump::resolve_playback() {
	playback.unref();

	if (player_path.is_empty()) {
		return;
	}

	AudioStreamPlayer *player = Object::cast_to<AudioStreamPlayer>(get_node_or_null(player_path));
	if (player == nullptr) {
		UtilityFunctions::push_warning("ByProdStreamPump: player_path does not point at an AudioStreamPlayer.");
		return;
	}

	// The playback object only exists once the player is running, so the stream is
	// started here rather than expecting the scene to have done it.
	if (!player->is_playing()) {
		player->play();
	}

	playback = Ref<AudioStreamGeneratorPlayback>(
			Object::cast_to<AudioStreamGeneratorPlayback>(player->get_stream_playback().ptr()));
	if (playback.is_null()) {
		UtilityFunctions::push_warning("ByProdStreamPump: the player's stream is not an AudioStreamGenerator.");
	}
}

void ByProdStreamPump::_process(double p_delta) {
	if (sound_manager.is_null() || !sound_manager->is_valid()) {
		return;
	}

	sound_manager->update();

	if (playback.is_null()) {
		return;
	}

	int available = playback->get_frames_available();
	if (available <= 0) {
		return;
	}
	if (available > max_chunk_frames) {
		available = max_chunk_frames;
	}

	const PackedVector2Array frames = sound_manager->mix(available);
	if (frames.is_empty()) {
		return;
	}
	playback->push_buffer(frames);
}

void ByProdStreamPump::set_sound_manager(const Ref<ByProdSoundManager> &p_manager) {
	sound_manager = p_manager;
}

Ref<ByProdSoundManager> ByProdStreamPump::get_sound_manager() const {
	return sound_manager;
}

void ByProdStreamPump::set_player_path(const NodePath &p_path) {
	player_path = p_path;
	if (is_inside_tree() && !Engine::get_singleton()->is_editor_hint()) {
		resolve_playback();
	}
}

NodePath ByProdStreamPump::get_player_path() const {
	return player_path;
}

void ByProdStreamPump::set_max_chunk_frames(int p_frames) {
	max_chunk_frames = p_frames < 64 ? 64 : p_frames;
}

int ByProdStreamPump::get_max_chunk_frames() const {
	return max_chunk_frames;
}
