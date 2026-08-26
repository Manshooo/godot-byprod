#include "bp_api.h"

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/string.hpp>

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace godot;

namespace byprod {

namespace {

Api g_api = {};
bool g_attempted = false;
bool g_loaded = false;
std::string g_error;

// The casing matters on Linux, where the runtime resolves to libbyProd.so exactly.
#ifdef _WIN32
constexpr const char *LIBRARY_FILE_NAME = "byProd.dll";
#elif defined(__APPLE__)
constexpr const char *LIBRARY_FILE_NAME = "libbyProd.dylib";
#else
constexpr const char *LIBRARY_FILE_NAME = "libbyProd.so";
#endif

void *open_library(const char *p_path) {
#ifdef _WIN32
	return reinterpret_cast<void *>(LoadLibraryA(p_path));
#else
	return dlopen(p_path, RTLD_NOW | RTLD_LOCAL);
#endif
}

void close_library(void *p_handle) {
#ifdef _WIN32
	FreeLibrary(reinterpret_cast<HMODULE>(p_handle));
#else
	dlclose(p_handle);
#endif
}

void *load_symbol(void *p_handle, const char *p_name) {
#ifdef _WIN32
	return reinterpret_cast<void *>(GetProcAddress(reinterpret_cast<HMODULE>(p_handle), p_name));
#else
	return dlsym(p_handle, p_name);
#endif
}

// Exported games keep the runtime next to the extension binary, but a developer
// may also have it on the system search path, so both are tried before giving up.
void *open_library_anywhere(std::string &r_tried) {
	const String bundled = String("res://addons/byprod/bin/") + String(LIBRARY_FILE_NAME);
	const String globalized = ProjectSettings::get_singleton()->globalize_path(bundled);

	const std::string candidates[] = {
		std::string(globalized.utf8().get_data()),
		std::string(LIBRARY_FILE_NAME),
	};

	for (const std::string &candidate : candidates) {
		void *handle = open_library(candidate.c_str());
		if (handle != nullptr) {
			return handle;
		}
		if (!r_tried.empty()) {
			r_tried += ", ";
		}
		r_tried += candidate;
	}

	return nullptr;
}

void load_api() {
	g_attempted = true;

	std::string tried;
	void *handle = open_library_anywhere(tried);
	if (handle == nullptr) {
		g_error = "byProd runtime not found. Tried: " + tried;
		return;
	}

	const char *missing = nullptr;

#define BYPROD_LOAD_FUNCTION(m_ret, m_name, m_params)                                    \
	g_api.m_name = reinterpret_cast<m_ret(*) m_params>(load_symbol(handle, #m_name));    \
	if (g_api.m_name == nullptr && missing == nullptr) {                                 \
		missing = #m_name;                                                               \
	}

	BYPROD_API_FUNCTIONS(BYPROD_LOAD_FUNCTION)

#undef BYPROD_LOAD_FUNCTION

	// A partially resolved table would crash on the first call to a null slot, so
	// one missing symbol invalidates the whole load: it means the runtime is a
	// different version than this wrapper was written against.
	if (missing != nullptr) {
		g_api = {};
		close_library(handle);
		g_error = std::string("byProd runtime is missing the symbol ") + missing +
				" — the installed version does not match this binding.";
		return;
	}

	// The library is deliberately never closed: the sound manager may outlive any
	// single scene, and unloading it under a live mixer thread is not worth the risk.
	g_loaded = true;
	g_error.clear();
}

} // namespace

const Api *api() {
	if (!g_attempted) {
		load_api();
	}
	return g_loaded ? &g_api : nullptr;
}

const char *load_error() {
	return g_error.c_str();
}

} // namespace byprod
