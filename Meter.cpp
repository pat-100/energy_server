#include "Meter.hpp"

Meter_Data::Meter_Data(){
	serial = "";
	register_grid = -1;
	register_generation = -1;
	register_resetable = -1;
	current_power = 0;
	last_time_read = 0;
}

Meter_Data::~Meter_Data(){
}

void Meter_Data::set_serial(std::string new_serial){
	serial = new_serial;
}

std::string Meter_Data::get_serial(){
	return serial;
}

std::string Meter::get_id(){
	if(Device::get_id() != "")return Device::get_id();
	else return "SN:" + get_serial();
}

int Meter::get_register_resetable(){
	return -1;
}

void Meter::uint16_t_to_hex(uint16_t source, char* destination){
		destination[0] = value_to_hexchar(source>>12 & 0x0F);
		destination[1] = value_to_hexchar(source>>8 & 0x0F);
		destination[2] = value_to_hexchar(source>>4 & 0x0F);
		destination[3] = value_to_hexchar(source & 0x0F);
}

char Meter::value_to_hexchar(char value){
	if(value<10)return '0' + value;
	else return 'A' + value - 0xA;	
}