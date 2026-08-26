// Runtime loader for the native byProd shared library.
//
// Every symbol is resolved with LoadLibrary/dlopen instead of linking against
// byProd's import library. That keeps this repository buildable — and its CI
// green — without redistributing or downloading Madrigal's proprietary SDK, and
// it turns a missing runtime into one readable error instead of the whole
// GDExtension failing to load.
//
// Signatures and constants are taken from bindings/csharp/ByProd.Native.cs and
// ByProd.cs in MadrigalGames/byProdResources, which are documented there as
// being one to one with byprod.h.

#pragma once

#include <cstddef>
#include <cstdint>

namespace byprod {

// Opaque handles owned by the runtime.
using SoundManagerHandle = void *;
using EventDescriptionHandle = void *;
using EventInstanceHandle = void *;
using GroupBusHandle = void *;

enum RuntimeEnvironment : int32_t {
	RUNTIME_ENVIRONMENT_APP = 0,
	RUNTIME_ENVIRONMENT_PREVIEW_IN_EDITOR = 1,
};

// Combined bitwise into SoundManagerSettings::flags.
enum SoundManagerFlags : uint32_t {
	SOUND_MANAGER_NONE = 0x0u,

	// byProd places 3D audio in a left-handed space by default; Godot is
	// right-handed, so the wrapper always sets this.
	SOUND_MANAGER_RIGHT_HANDED_3D = 0x1u,

	// No audio device and no thread of its own: audio is only rendered when the
	// host calls Mix(). Needs a real sample rate, since there is no device to ask.
	SOUND_MANAGER_HOST_MIXED = 0x2u,
};

enum TickLevel : uint32_t {
	TICK_LEVEL_NONE = 0,
	TICK_LEVEL_PARTIAL = 1,
	TICK_LEVEL_FULL = 2,
};

enum EventInstanceState : uint32_t {
	EVENT_INSTANCE_STOPPED = 0,
	EVENT_INSTANCE_PLAYING = 1,
	EVENT_INSTANCE_PAUSED = 2,
	EVENT_INSTANCE_FINISHED = 3,
};

constexpr uint32_t INVALID_PARAMETER_INDEX = 0xFFFFFFFFu;

// Mirrors SoundManagerSettings in ByProd.cs. Never fill this by hand: hand a
// zeroed, over-sized buffer to bpdSoundManagerSettingsInit() and cast it (see
// bp_sound_manager.cpp), so a future SDK that grows the struct writes into slack
// space instead of past the end of ours.
struct SoundManagerSettings {
	int32_t environment;
	uint32_t flags;
	uint32_t sample_rate;
	float global_volume_multiplier;
	float post_clip_scaler;
	uint32_t max_active_voice_count;
};

static_assert(sizeof(SoundManagerSettings) == 24, "byProd settings layout drifted from the C# binding");

// The subset of the C API this wrapper needs. Sound banks, the job scheduler,
// bulk audio and the debug statistics are deliberately left out of the first
// pass — they are additive and can be appended here without touching callers.
#define BYPROD_API_FUNCTIONS(X)                                                                                          \
	X(uint32_t, bpdVersion, ())                                                                                          \
	X(uint32_t, bpdHashString, (const char *))                                                                           \
	X(void, bpdSoundManagerSettingsInit, (void *))                                                                       \
	X(void *, bpdSoundManagerCreate, (void *))                                                                           \
	X(void, bpdSoundManagerDestroy, (void *))                                                                            \
	X(int32_t, bpdSoundManagerLoadProject, (void *, const uint8_t *, size_t))                                            \
	X(void *, bpdSoundManagerGetEventDescription, (void *, const char *))                                                \
	X(void, bpdSoundManagerUpdate, (void *))                                                                             \
	X(void, bpdSoundManagerMix, (void *, float *, uint32_t))                                                             \
	X(void, bpdSoundManagerSetTickLevel, (void *, uint32_t))                                                             \
	X(void, bpdSoundManagerSetGlobalVolume, (void *, float))                                                             \
	X(float, bpdSoundManagerGetGlobalVolume, (void *))                                                                   \
	X(void, bpdSoundManagerSetListenerTransform, (void *, float, float, float, float, float, float, float, float, float)) \
	X(void *, bpdSoundManagerGetMasterGroupBus, (void *))                                                                \
	X(void *, bpdSoundManagerGetGroupBus, (void *, const char *))                                                        \
	X(float, bpdGroupBusGetVolume, (void *))                                                                             \
	X(void, bpdGroupBusSetVolume, (void *, float))                                                                       \
	X(uint32_t, bpdEventDescriptionGetParameterIndex, (void *, const char *))                                            \
	X(uint32_t, bpdEventDescriptionGetParameterCount, (void *))                                                          \
	X(float, bpdEventDescriptionGetLength, (void *))                                                                     \
	X(void *, bpdEventDescriptionCreateInstance, (void *))                                                               \
	X(void, bpdEventInstanceStart, (void *))                                                                             \
	X(void, bpdEventInstanceStop, (void *))                                                                              \
	X(void, bpdEventInstancePause, (void *))                                                                             \
	X(void, bpdEventInstanceUnpause, (void *))                                                                           \
	X(void, bpdEventInstanceRelease, (void *))                                                                           \
	X(void, bpdEventInstanceReleaseWhenFinished, (void *))                                                               \
	X(void, bpdEventInstanceFadeIn, (void *, float))                                                                     \
	X(void, bpdEventInstanceReleaseAfterFadeOut, (void *, float))                                                        \
	X(uint32_t, bpdEventInstanceGetState, (void *))                                                                      \
	X(float, bpdEventInstanceGetTime, (void *))                                                                          \
	X(void, bpdEventInstanceSetParameterByIndex, (void *, uint32_t, float))                                              \
	X(void, bpdEventInstanceSetParameterByName, (void *, const char *, float))                                           \
	X(float, bpdEventInstanceGetParameterByIndex, (void *, uint32_t))                                                    \
	X(void, bpdEventInstanceSendSignal, (void *, const char *))                                                          \
	X(void, bpdEventInstanceSetVolumeMultiplier, (void *, float))                                                        \
	X(float, bpdEventInstanceGetVolumeMultiplier, (void *))                                                              \
	X(void, bpdEventInstanceSet3DAttributes, (void *, float, float, float, float, float, float))

struct Api {
#define BYPROD_DECLARE_FUNCTION(m_ret, m_name, m_params) m_ret(*m_name) m_params;
	BYPROD_API_FUNCTIONS(BYPROD_DECLARE_FUNCTION)
#undef BYPROD_DECLARE_FUNCTION
};

// The resolved API, or nullptr when the library could not be loaded. The first
// call performs the load; every later call is a pointer read.
const Api *api();

// Why the last load attempt failed. Empty while the library is loaded.
const char *load_error();

} // namespace byprod
