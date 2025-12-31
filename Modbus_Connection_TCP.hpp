#ifndef _Modbus_Connection_TCP_H___
#define _Modbus_Connection_TCP_H___

#include "Modbus_Connection.hpp"

class Modbus_Connection_TCP:public Modbus_Connection {
	
		std::string host;
		uint16_t port;
		
		void create_tcp_mobus_connection();
	
	public:
		Modbus_Connection_TCP(std::string host, uint16_t port);
		~Modbus_Connection_TCP();
		bool connect();
		
		std::string get_host();
		uint16_t get_port();	
		
		int write_double_registers(int address, uint32_t value);
		int write_single_register(int address, uint16_t value);
		uint32_t read_double_registers(int address);
		uint16_t read_single_register(int address);
		void* alloc_and_read_registers(int address, int n);		
};

#endif