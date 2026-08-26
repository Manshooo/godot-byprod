#include "bp_group_bus.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ByProdGroupBus::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_volume", "volume"), &ByProdGroupBus::set_volume);
	ClassDB::bind_method(D_METHOD("get_volume"), &ByProdGroupBus::get_volume);
	ClassDB::bind_method(D_METHOD("is_valid"), &ByProdGroupBus::is_valid);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "volume"), "set_volume", "get_volume");
}

Ref<ByProdGroupBus> ByProdGroupBus::wrap(byprod::GroupBusHandle p_handle) {
	Ref<ByProdGroupBus> bus;
	if (p_handle == nullptr) {
		return bus;
	}
	bus.instantiate();
	bus->handle = p_handle;
	return bus;
}

void ByProdGroupBus::set_volume(float p_volume) {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return;
	}
	api->bpdGroupBusSetVolume(handle, p_volume);
}

float ByProdGroupBus::get_volume() const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return 0.0f;
	}
	return api->bpdGroupBusGetVolume(handle);
}
