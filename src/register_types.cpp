#include "register_types.h"

#include "bp_event_description.h"
#include "bp_event_instance.h"
#include "bp_group_bus.h"
#include "bp_sound_manager.h"
#include "bp_stream_pump.h"

#include <gdextension_interface.h>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_byprod_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// Instances and descriptions are registered as abstract: they are only ever
	// handed out by the runtime, never constructed from script.
	GDREGISTER_ABSTRACT_CLASS(ByProdGroupBus);
	GDREGISTER_ABSTRACT_CLASS(ByProdEventInstance);
	GDREGISTER_ABSTRACT_CLASS(ByProdEventDescription);
	GDREGISTER_ABSTRACT_CLASS(ByProdSoundManager);
	GDREGISTER_CLASS(ByProdStreamPump);
}

void uninitialize_byprod_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {

GDExtensionBool GDE_EXPORT byprod_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
		const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_byprod_module);
	init_obj.register_terminator(uninitialize_byprod_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
