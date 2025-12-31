/* Class Keba_P30 represent charger of the Keba P30 series. The class
   can be used to read values from the internal meter as well as it
   can be used to control the maximum charging power.
   
   Might work for similar devices too.*/

#ifndef _Keba_P30_H_
#define _Keba_P30_H_

#include "Modbus_Connection_TCP.hpp"
#include "Meter.hpp"
#include "Manageable.hpp"
#include <time.h>
#include <string>

modbus_t* create_tcp_mobus_connection(std::string host, uint16_t port);

class Keba_P30{
	private:
	
		Modbus_Connection_TCP* modbus;
		
		uint32_t get_charging_state();
		uint32_t get_cable_state();
		uint32_t get_error_code();
		uint32_t get_active_power();
		uint32_t get_total_consumption();
		uint32_t get_max_current();
		uint32_t get_max_current_supported();
		uint32_t get_authentificated_rfid();
		uint32_t get_current_consumption();
		uint32_t get_current_L1();
		uint32_t get_phase_switching_state();
		int get_number_of_phases();
		int set_charging_current(uint16_t value);
		int set_station_enabled(uint16_t value);
		uint32_t get_voltage_l1();
		uint32_t get_voltage_l2();
		uint32_t get_voltage_l3();	
		int set_phase_switch(uint16_t value);
		int set_phase_switch_toggle(uint16_t value);
		
	class Meter_Keba_P30: public Meter{
		private:
			Keba_P30* keba_p30;
		public:
			Meter_Keba_P30(Keba_P30* keba_p30);
			~Meter_Keba_P30();
			time_t get_last_time_read()override;
			std::string get_serial() override;
			int get_register_grid() override;
			int get_register_generation() override;
			int get_register_resetable() override;
			int get_current_power() override;
			Meter_Data get_meter_data() override;
	};
	
	class Manageable_Keba_P30: public Manageable{
		private:
			Keba_P30* keba_p30;
			int apply_new_power(int new_power);
			void init();
			bool is_enabled;
			int minimal_consumption_possible;
		public:
			Manageable_Keba_P30(Keba_P30* keba_p30);
			~Manageable_Keba_P30();
			virtual bool set_target_power(int new_target_power)override;
			int get_minimal_consumption()override;
			int get_maximal_consumption()override;			
			int get_power()override;
	};
		
	public:
	
		enum charging_state{
			START_UP = 0,
			NOT_READY = 1, //The charging station is not connected to an electric vehicle, it is locked by the authorization function or another mechanism.
			WAITING = 2, //waits for a reaction from the electric vehicle.
			CHARGING = 3, 
			ERROR = 4,
			INTERUPTED = 5 //temperature is too high or the wallbox is in suspended mode
		}; 

		enum cable_state{
			NO_CABLE = 0,
			CONNECTED_TO_CHARGER_ONLY = 1,
			CONNECTED_AND_LOCKED_TO_CHARGER_ONLY,
			CONNECTED_TO_BOTH = 5,
			CONNECTED_AND_LOCKED_TO_BOTH = 7 //charging
		};
		
		Keba_P30(std::string host, uint16_t port);
		~Keba_P30();
		
		Meter* retrieve_meter();
		Manageable* retrieve_manageable();
		
};

#endif