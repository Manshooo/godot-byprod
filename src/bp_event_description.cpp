#include "bp_event_description.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ByProdEventDescription::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_instance"), &ByProdEventDescription::create_instance);
	ClassDB::bind_method(D_METHOD("get_parameter_index", "name"), &ByProdEventDescription::get_parameter_index);
	ClassDB::bind_method(D_METHOD("get_parameter_count"), &ByProdEventDescription::get_parameter_count);
	ClassDB::bind_method(D_METHOD("get_length"), &ByProdEventDescription::get_length);
	ClassDB::bind_method(D_METHOD("is_valid"), &ByProdEventDescription::is_valid);
}

Ref<ByProdEventDescription> ByProdEventDescription::wrap(byprod::EventDescriptionHandle p_handle) {
	Ref<ByProdEventDescription> description;
	if (p_handle == nullptr) {
		return description;
	}
	description.instantiate();
	description->handle = p_handle;
	return description;
}

Ref<ByProdEventInstance> ByProdEventDescription::create_instance() {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return Ref<ByProdEventInstance>();
	}
	return ByProdEventInstance::wrap(api->bpdEventDescriptionCreateInstance(handle));
}

int ByProdEventDescription::get_parameter_index(const String &p_name) const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return -1;
	}
	const uint32_t index = api->bpdEventDescriptionGetParameterIndex(handle, p_name.utf8().get_data());
	// The C API reports "no such parameter" as an all-ones index, which would read
	// as a huge valid index on the GDScript side.
	return index == byprod::INVALID_PARAMETER_INDEX ? -1 : static_cast<int>(index);
}

int ByProdEventDescription::get_parameter_count() const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return 0;
	}
	return static_cast<int>(api->bpdEventDescriptionGetParameterCount(handle));
}

float ByProdEventDescription::get_length() const {
	const byprod::Api *api = byprod::api();
	if (api == nullptr || handle == nullptr) {
		return 0.0f;
	}
	return api->bpdEventDescriptionGetLength(handle);
}
