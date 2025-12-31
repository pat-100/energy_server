/* An instance of class Energy_Manager asynchronously manages a manageable device
   (see class Manageable). In an given interval it calculates a given formula and
   put the result to the manageable device (i. e. a car charger)
   
   TODO: describe how regulation types work and public stuff
   */

#ifndef _ENERGY_MANAGER_H___
#define _ENERGY_MANAGER_H___

#include "Manageable.hpp"
#include "Calculation_Term.hpp"
#include <unistd.h>
#include <pthread.h>
#include <string.h>

enum REGULATION_TYPE{MAXIMAL_GRID_POWER = 0, MINIMAL_POWER = 1};

class Energymanager_Options{
	public:
		int minimal_consumption_possible;
		int maximal_consumption_possible;
		REGULATION_TYPE regulation_type;
		int regulation_value;
};

class Energy_Manager{
	
		private:
		std::string id;
		pthread_t thread;
		pthread_mutex_t data_lock;
		
		int interval;
		
		Manageable* target_device;
		Calculation_Term* calculation_rule;
		REGULATION_TYPE regulation_type;
		int regulation_value; //in watts
		
		static void* handle_thread(void* object);
		void handle_energy_manager();
		void manage();
		
	public:
		Energy_Manager(Manageable* target_device, std::string id, int interval, Calculation_Term* calculation_rule, REGULATION_TYPE regulation_type = MAXIMAL_GRID_POWER, int regulation_value=0);
		~Energy_Manager();
		bool set_regulation(REGULATION_TYPE regulation_type, int regulation_value);
		Energymanager_Options get_device_data();
		std::string get_id();

};

#endif