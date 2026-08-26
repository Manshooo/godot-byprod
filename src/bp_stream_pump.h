// Pushes a host-mixed byProd manager through Godot's own audio buses.
//
// Only needed when byProd must not own the audio device — routing through an
// AudioStreamPlayer means Godot's buses, effects and volume settings apply to
// byProd's output. Give the node an AudioStreamPlayer whose stream is an
// AudioStreamGenerator, and a manager created with create_host_mixed() at the
// generator's mix rate.
//
// The plain create() path needs none of this: byProd then renders on its own
// thread and never hands Godot a sample.

#pragma once

#include "bp_sound_manager.h"

#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/node.hpp>

namespace godot {

class ByProdStreamPump : public Node {
	GDCLASS(ByProdStreamPump, Node)

protected:
	static void _bind_methods();

public:
	void _ready() override;
	void _process(double p_delta) override;

	void set_sound_manager(const Ref<ByProdSoundManager> &p_manager);
	Ref<ByProdSoundManager> get_sound_manager() const;

	void set_player_path(const NodePath &p_path);
	NodePath get_player_path() const;

	// Frames pushed in one go. Smaller values react faster, larger ones survive
	// frame spikes; the pump fills whatever the generator reports as free anyway.
	void set_max_chunk_frames(int p_frames);
	int get_max_chunk_frames() const;

private:
	void resolve_playback();

	Ref<ByProdSoundManager> sound_manager;
	NodePath player_path;
	Ref<AudioStreamGeneratorPlayback> playback;
	int max_chunk_frames = 2048;
};

} // namespace godot
