#ifndef _Modbus_Connection_H___
#define _Modbus_Connection_H___

#include <string>
#include "/usr/local/include/modbus/modbus.h"

class Modbus_Connection{
	
	int current_slave;
	int reading_attempts;
	
	//void modbus_init(std::string device);
	int set_slave(int slave);
	int read_register(int addr_reg, int len_reg, uint16_t* destination);
	
	protected:
	modbus_t* modbus_ctx;
	
	public:
	//Modbus_Connection(std::string device, int reading_retries = 0);
	//~Modbus_Connection();
	int read_slave_register(int slave, int addr_reg, int len_reg, uint16_t* destination);
	void set_reading_retries(int retries);
		
};

#endif