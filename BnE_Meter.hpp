/* Class BnE_Meter represent a meter of DS100*-series of manufacturer B+G e-tech
   connected via a modbus interface */

#ifndef _BnE_Meter_H___
#define _BnE_Meter_H___

#include <time.h>
#include "Meter.hpp"
#include "Modbus_Connection_RTU.hpp"

const int addr_reg_serial = 0x1000;
const int len_reg_serial = 6;
const int addr_reg_1_0_1_8_0 = 0x010E;
const int len_reg_1_0_1_8_0 = 2;
const int addr_reg_1_0_2_8_0 = 0x0118;
const int len_reg_1_0_2_8_0 = 2;
const int addr_reg_power = 0x0420;
const int len_power = 2;

class BnE_Meter:public Meter{
	
	private:
		Modbus_Connection_RTU* modbus;
		int slave;
		std::string serial;
		time_t last_time_read;
		void read_serial();
	
	public:
		/*meter_bus: a previous created and initialised modbus connection, where one or more devices bay be listing
		  slace_address the modbus slace address the device was assigned to*/  
		BnE_Meter(Modbus_Connection_RTU* meter_bus, int slave_address); 

		/*returns mode (see meter's manual)*/
		int get_mode();
		
		//for use of overridden methods, see super class
		std::string get_serial() override;
		int get_register_grid() override;
		int get_register_generation() override;
		int get_current_power() override;
		Meter_Data get_meter_data() override;
		time_t get_last_time_read()override;
			
};

#endif