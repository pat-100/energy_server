#include "Modbus_Connection_RTU.hpp"

Modbus_Connection_RTU::Modbus_Connection_RTU(std::string device){
	//Modbus_Connection_RTU::reading_attempts = 1+reading_retries;
	modbus_init(device);
	if(!modbus_ctx){
		fprintf(stderr, "Fehler beim Initialisieren von Modbus");
		return;
	}
}

Modbus_Connection_RTU::~Modbus_Connection_RTU(){
	modbus_close(modbus_ctx);
	modbus_free(modbus_ctx);
}

/*int Modbus_Connection_RTU::read_slave_register(int slave, int addr_reg, int len_reg, uint16_t* destination){
	int rc = set_slave(slave);
	if(!rc)return read_register(addr_reg, len_reg, destination);	
	else return rc;
}*/
/*
int Modbus_Connection_RTU::read_register(int addr_reg, int len_reg, uint16_t* destination){
	
	for(int i = reading_attempts; i; --i){
		int result = modbus_read_input_registers(modbus_ctx, addr_reg, len_reg, destination);
		if(result != -1)return 0;
		fprintf(stderr, "\nModbus: Failed reading register. %i retries left...", i);
	}
	return -1;
	
}*/

void Modbus_Connection_RTU::modbus_init(std::string device){
	fprintf(stderr, "\nModbus: Try connecting ...");
	modbus_ctx = modbus_new_rtu(device.c_str(), 9600, 'N', 8, 1);
	int rc_connect = modbus_connect(modbus_ctx);
	if(rc_connect == -1)fprintf(stderr, "failed");
	else fprintf(stderr, "connected");
}