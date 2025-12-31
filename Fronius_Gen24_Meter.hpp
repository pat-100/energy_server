/*Class Fronius_Gen24_Meter fetches all the data via modbus TCP from a 
  Fronius Gen24 inverter which can be reasonably represented as
  values of a meter. Might work for similar devices too.*/

#ifndef _Fronius_Gen24_H_
#define _Fronius_Gen24_H_
#include <stdio.h>
#include <errno.h>
#include "/usr/local/include/modbus/modbus.h"
#include "Meter.hpp"
#include <time.h>
#include <string>

class Fronius_Gen24_Meter: public Meter{
	private:
		std::string host;
		uint16_t port;	
		modbus_t* ctx;
		uint32_t read_double_registers(int address);
		int write_single_register(int address, uint16_t value);
		int write_double_registers(int address, uint32_t value);
		void* alloc_and_read_registers(int address, int n);
		uint16_t read_single_register(int address);
		int16_t read_single_register_signed(int address);
		std::string read_string(int start_register, int len);
		float read_float(int reg);
		void init();
		static modbus_t* create_tcp_mobus_connection(std::string, uint16_t port);
		
		int16_t scale_W_SF;
		
		static int scale(const int16_t value, const int16_t exp);
				
	public:
	
		Fronius_Gen24_Meter(std::string host, uint16_t port);
		~Fronius_Gen24_Meter();
		bool try_connecting();
		time_t get_last_time_read();
		std::string get_serial() override;
		int get_register_grid() override;
		int get_register_generation() override;
		//int get_register_resetable() override;
		int get_current_power() override;
		Meter_Data get_meter_data() override;

};

#endif