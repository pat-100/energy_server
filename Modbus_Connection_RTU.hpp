#ifndef _Modbus_Connection_RTU_H___
#define _Modbus_Connection_RTU_H___

#include "Modbus_Connection.hpp"

class Modbus_Connection_RTU:public Modbus_Connection {
	
	int i;
	public:
	void get_i();
	/*modbus_t* modbus_ctx;
	int current_slave;*/
	//int reading_attempts;
	
	void modbus_init(std::string device);
	/*int set_slave(int slave);
	int read_register(int addr_reg, int len_reg, uint16_t* destination);
	
	*/
	public:
	Modbus_Connection_RTU(std::string device);
	~Modbus_Connection_RTU();
	//int read_slave_register(int slave, int addr_reg, int len_reg, uint16_t* destination);*/
		
};

#endif
