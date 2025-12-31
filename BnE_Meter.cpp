#include "BnE_Meter.hpp"

BnE_Meter::BnE_Meter(Modbus_Connection_RTU* meter_bus, int slave_address){
	modbus = meter_bus;
	slave = slave_address;
	read_serial();
}

void BnE_Meter::read_serial(){
	uint16_t* serial_buffer = (uint16_t*)malloc(sizeof(uint16_t ) * len_reg_serial);
	if(modbus->read_slave_register(slave, addr_reg_serial, len_reg_serial, serial_buffer) != 0){
		serial ="";
		return;
	}
	char* c_serial = (char*)malloc(sizeof(char) * ( len_reg_serial * 2 + 1 ));
	for(int i=0; i<3; ++i)uint16_t_to_hex(serial_buffer[i], c_serial+(4*i));
	c_serial[12] = '\0';
	serial = c_serial;
	free(serial_buffer);
	free(c_serial);	
}

std::string BnE_Meter::get_serial(){
	if(serial == "")read_serial(); //if not yet successfully read, try again now
	return serial;
}

int BnE_Meter::get_register_grid(){
	uint16_t buffer[2];
	if(modbus->read_slave_register(slave, addr_reg_1_0_1_8_0, len_reg_1_0_1_8_0, buffer) != 0)return -1;
	return ((((uint32_t)buffer[0])<<16) + buffer[1])*10;
}

int BnE_Meter::get_register_generation(){
	uint16_t buffer[2];
	if(modbus->read_slave_register(slave, addr_reg_1_0_2_8_0, len_reg_1_0_2_8_0, (uint16_t*)(&buffer)) != 0)return -1;
	return ((((uint32_t)buffer[0])<<16) + buffer[1])*10;
}

int BnE_Meter::get_mode(){
	uint16_t buffer;
	if(modbus->read_slave_register(slave, 0x100F, 1, &buffer) != 0)return -1;
	return buffer;
}

int BnE_Meter::get_current_power(){
	uint16_t buffer[2];
	if(modbus->read_slave_register(slave, addr_reg_power, len_power, buffer) != 0)return -1;
	if(buffer[0] & 0xA000)return - (0xFFFF - buffer[1]);
	else return buffer[1];
	
}

Meter_Data BnE_Meter::get_meter_data(){
	Meter_Data meter_data;
	meter_data.register_grid = get_register_grid();
	meter_data.register_generation = get_register_generation();
	meter_data.current_power = get_current_power();
	meter_data.serial = get_serial();
	meter_data.last_time_read = get_last_time_read();
	return meter_data;
}

time_t BnE_Meter::get_last_time_read(){
	return time(NULL); //return now because meter is read whenever a get* method is called
}