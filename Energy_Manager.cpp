#include "Energy_Manager.hpp"

Energy_Manager::Energy_Manager(Manageable* target_device, std::string id, int interval, Calculation_Term* calculation_rule, REGULATION_TYPE regulation_type, int regulation_value){
	
	Energy_Manager::id = id;
	Energy_Manager::target_device = target_device;
	Energy_Manager::interval = interval;
	Energy_Manager::calculation_rule = calculation_rule;
	Energy_Manager::regulation_type = regulation_type;
	Energy_Manager::regulation_value = regulation_value;
	
	pthread_mutex_init(&data_lock, NULL);
	pthread_create(&thread, NULL, &handle_thread, (void*)this);
	
}

Energy_Manager::~Energy_Manager(){
	thread = 0;
}

void* Energy_Manager::handle_thread(void* object){
	((Energy_Manager*)object)->handle_energy_manager();
	return NULL;
}

void Energy_Manager::handle_energy_manager(){
	while(thread){
		manage();		//manage
		sleep(interval);//wait for next time to manage
	}
	pthread_mutex_destroy(&data_lock);
}

void Energy_Manager::manage(){

	pthread_mutex_lock(&(data_lock));

	//calculate:
	int calc_result = calculation_rule->get_result();

	int new_power;
	switch(regulation_type){
		case MAXIMAL_GRID_POWER:
			new_power = (calc_result + regulation_value) >= target_device->get_minimal_consumption() ? (calc_result + regulation_value) : 0;
		break;
		case MINIMAL_POWER: 
			new_power = (calc_result > regulation_value) ? calc_result : regulation_value;
		break;
	}
	
	fprintf(stderr, "\n%s-%s: Setting %i ...", id.c_str(), target_device->get_id().c_str(), new_power);
	
	target_device->set_target_power(new_power);
	
	pthread_mutex_unlock(&(data_lock));

}

std::string Energy_Manager::get_id(){
	return id;
}

Energymanager_Options Energy_Manager::get_device_data(){
	pthread_mutex_lock(&data_lock);
	
	Energymanager_Options device_data;	
	device_data.minimal_consumption_possible = target_device->get_minimal_consumption();
	device_data.maximal_consumption_possible = target_device->get_maximal_consumption();
	device_data.regulation_type = regulation_type;
	device_data.regulation_value = regulation_value;
	
	pthread_mutex_unlock(&data_lock);
	return device_data;
}

bool Energy_Manager::set_regulation(REGULATION_TYPE new_regulation_type, int new_regulation_value){
	pthread_mutex_lock(&(data_lock));
	if(new_regulation_type == 0 || (new_regulation_type == 1 && new_regulation_value >= target_device->get_minimal_consumption())){
		Energy_Manager::regulation_type = new_regulation_type;
		Energy_Manager::regulation_value = new_regulation_value;
		pthread_mutex_unlock(&(data_lock));
		return true;
	}
	pthread_mutex_unlock(&(data_lock));
	return false;
}