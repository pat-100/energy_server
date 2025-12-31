#include "Device.hpp"

std::string Device::get_id(){
	return id;
}

std::string Device::get_description(){
	return description;
}

void Device::set_id(std::string id){
	Device::id = id;
}

void Device::set_meta_fields(std::string id, std::string description){
	Device::id = id;
	Device::description = description;
}