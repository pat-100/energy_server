#include "Modbus_Connection.hpp"

/*Modbus_Connection::Modbus_Connection(std::string device, int reading_retries){
	Modbus_Connection::reading_attempts = 1+reading_retries;
	this->modbus_init(device);
	if(!modbus_ctx){
		fprintf(stderr, "Fehler beim Initialisieren von Modbus");
		return;
	}
}*/

/*Modbus_Connection::~Modbus_Connection(){
	modbus_close(modbus_ctx);
	modbus_free(modbus_ctx);
}*/

int Modbus_Connection::set_slave(int slave){
	if(current_slave == slave)return 0;

	fprintf(stderr, "\nModbus: setting slave to %i...", slave);
	int rc = modbus_set_slave(modbus_ctx, slave);
	if(rc != 0){
		fprintf(stderr, "modbus_set_slave failed");
		return rc;
	}
	current_slave = slave;
	//usleep(500);
	return 0;
}

int Modbus_Connection::read_slave_register(int slave, int addr_reg, int len_reg, uint16_t* destination){
	int rc = set_slave(slave);
	if(!rc)return read_register(addr_reg, len_reg, destination);	
	else return rc;
}

int Modbus_Connection::read_register(int addr_reg, int len_reg, uint16_t* destination){
	
	for(int i = reading_attempts; i; --i){
		int result = modbus_read_input_registers(modbus_ctx, addr_reg, len_reg, destination);
		if(result != -1)return 0;
		fprintf(stderr, "\nModbus: Failed reading register. %i retries left...", i);
	}
	return -1;
	
}

void Modbus_Connection::set_reading_retries(int retries){
	reading_attempts = 1 + retries;
}

/*void Modbus_Connection::modbus_init(std::string device){
	fprintf(stderr, "\nModbus: Try connecting ...");
	modbus_ctx = modbus_new_rtu(device.c_str(), 9600, 'N', 8, 1);
	int rc_connect = modbus_connect(modbus_ctx);
	if(rc_connect == -1)fprintf(stderr, "failed");
	else fprintf(stderr, "connected");
}*/