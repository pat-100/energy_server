#ifndef _METER_H___
#define _METER_H___

#include <string>
#include <stdint.h>
#include "Device.hpp"

class Meter_Data{
	public:
	
		int register_grid;
		int register_generation;
		int register_resetable;
		int current_power;
		std::string serial;
		time_t last_time_read;
		
		Meter_Data();
		~Meter_Data();
		void set_serial(std::string new_serial);
		std::string get_serial();

};

class Meter:public Device{
	
	public:
		
		virtual time_t get_last_time_read()=0;
		virtual std::string get_serial() = 0;
		virtual int get_register_grid() = 0;
		virtual int get_register_generation() = 0;
		virtual int get_register_resetable();
		virtual int get_current_power() = 0;
		virtual Meter_Data get_meter_data() = 0;
		
		virtual std::string get_id()override;
		
		static void uint16_t_to_hex(uint16_t source, char* destination);
		static char value_to_hexchar(char value);
};

#endif
