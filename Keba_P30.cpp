#include "Keba_P30.hpp"

Keba_P30::Keba_P30(std::string host, uint16_t port){
	modbus = new Modbus_Connection_TCP(host, port);
	modbus->connect();
}

Keba_P30::~Keba_P30(){
	delete modbus;
}

int Keba_P30::Manageable_Keba_P30::apply_new_power(int new_power){
	
	int number_of_phases = keba_p30->get_number_of_phases();
	//fprintf(stderr, "\nNP:%i", number_of_phases);
	if(minimal_consumption_possible != get_minimal_consumption()){
		minimal_consumption_possible = get_minimal_consumption();
		is_enabled = 0;
		keba_p30->set_station_enabled(is_enabled);
		fprintf(stderr, "\nNumber of phases just changed.");
		return 0;
	}
	
	int target_power;
	int charging_state = keba_p30->get_charging_state();
	if(charging_state != Keba_P30::CHARGING && charging_state != Keba_P30::INTERUPTED){
		target_power = minimal_consumption_possible;//230 * 6 * get_number_of_phases(devices[i].modbus_ctx);
		fprintf(stderr, "\nNo charging. State: %i, Cable: %i", charging_state, keba_p30->get_cable_state());
	}
	else {
		int maximal_consumption_possible = get_maximal_consumption();
		target_power = new_power <= maximal_consumption_possible ? new_power : maximal_consumption_possible;
		//fprintf(stderr, "\nCharging: %i", charging_state);
	}
	
	if((target_power != 0 && is_enabled == 0) || (target_power == 0 && is_enabled == 1) || is_enabled == -1){
		is_enabled = target_power != 0 ? 1 : 0;
		keba_p30->set_station_enabled(is_enabled);
		//fprintf(stderr, "\nStation enabled: %i", is_enabled);
	}
	int rc = keba_p30->set_charging_current(1000 * target_power / (230*number_of_phases));
	//fprintf(stderr, "\nRC: %i", rc);
	//uint16_t I = 1000 * target_power / (230*number_of_phases);
	//fprintf(stderr, "\nP30: Neue Sollleistung: %iW (Sollstrom: %u)",target_power, I);
	return target_power;

}

std::string Keba_P30::Meter_Keba_P30::get_serial(){
	return "P30@" + keba_p30->modbus->get_host() + ":" + std::to_string(keba_p30->modbus->get_port());
};

int Keba_P30::Meter_Keba_P30::get_register_grid(){
	return keba_p30->get_total_consumption();
}

int Keba_P30::Meter_Keba_P30::get_register_generation(){
	return -1;
}

int Keba_P30::Meter_Keba_P30::get_register_resetable(){
	return keba_p30->get_current_consumption();
}

int Keba_P30::Meter_Keba_P30::get_current_power(){
	return keba_p30->get_active_power();
}

Meter_Data Keba_P30::Meter_Keba_P30::get_meter_data(){
	Meter_Data meter_data;
	meter_data.register_grid = keba_p30->get_total_consumption();
	meter_data.register_generation = -1;
	meter_data.register_resetable = get_register_resetable();
	meter_data.current_power = keba_p30->get_active_power();
	meter_data.serial = get_serial();
	meter_data.last_time_read = time(NULL);
	return meter_data;
}

uint32_t Keba_P30::get_charging_state(){
	return modbus->read_double_registers(1000);
}

uint32_t Keba_P30::get_cable_state(){
	return modbus->read_double_registers(1004);
}

uint32_t Keba_P30::get_error_code(){
	return modbus->read_double_registers(1006);
}

uint32_t Keba_P30::get_current_L1(){
	return modbus->read_double_registers(1008);
}

uint32_t Keba_P30::get_active_power(){
	return modbus->read_double_registers(1020)/1000;
}

uint32_t Keba_P30::get_total_consumption(){
	return modbus->read_double_registers(1036)/10;
}

uint32_t Keba_P30::get_max_current(){
	return modbus->read_double_registers(1100);
}

uint32_t Keba_P30::get_max_current_supported(){
	return modbus->read_double_registers(1110);
}

uint32_t Keba_P30::get_authentificated_rfid(){
	return modbus->read_double_registers(1500);
}

uint32_t Keba_P30::get_current_consumption(){
	return modbus->read_double_registers(1502)/10;
}

uint32_t Keba_P30::get_phase_switching_state(){
	return modbus->read_double_registers(1552);
}

uint32_t Keba_P30::get_voltage_l1(){
	return modbus->read_double_registers(1040);
}

uint32_t Keba_P30::get_voltage_l2(){
	return modbus->read_double_registers(1042);
}

uint32_t Keba_P30::get_voltage_l3(){
	return modbus->read_double_registers(1044);
}

int Keba_P30::get_number_of_phases(){
	int n_phases = 0;
	/*if(get_voltage_l1()>115)++n_phases;
	if(get_voltage_l2()>115)++n_phases;
	if(get_voltage_l3()>115)++n_phases;*/
	//fprintf(stderr,"\nV %i %i %i", get_voltage_l1(),get_voltage_l2(),get_voltage_l3());
	//int n_phases = get_phase_switching_state();
	return n_phases != 0 ? n_phases : 1;
}

int Keba_P30::set_charging_current(uint16_t value){
	return modbus->write_single_register(5004, value);
}

int Keba_P30::set_station_enabled(uint16_t value){
	return modbus->write_single_register(5014, value);
}

int Keba_P30::set_phase_switch_toggle(uint16_t value){
	return modbus->write_single_register(5050, value);
}

int Keba_P30::set_phase_switch(uint16_t value){
	return modbus->write_single_register(5052, value);
}

void Keba_P30::Manageable_Keba_P30::init(){
	keba_p30->set_phase_switch_toggle(3);
	keba_p30->set_phase_switch(0);
}

time_t Keba_P30::Meter_Keba_P30::get_last_time_read(){
	return time(NULL);
}

bool Keba_P30::Manageable_Keba_P30::set_target_power(int new_target_power){
	
	/*int* x = NULL;
	*x = 0;*/
	
	//std::cerr << "testo";
	//fprintf(stderr, "\nTarget: %i", new_target_power);
	//target_power = new_target_power;//((Keba_P30*)this)->
	int new_actual_power = apply_new_power(new_target_power);
	//int number_of_phases = get_number_of_phases();
	//if(new_target_power >= 0 && new_target_power <= 230*number_of_phases){
	//set_charging_current(1000 * new_target_power / (230*number_of_phases));
	//fprintf(stderr, "\n%i W Leistung gesetzt.", actual_power);
	
	return Manageable::set_target_power(new_actual_power);
	//}
	//else return false;
}

int Keba_P30::Manageable_Keba_P30::get_power(){
	return keba_p30->get_active_power();
}

int Keba_P30::Manageable_Keba_P30::get_minimal_consumption(){
	return keba_p30->get_number_of_phases() * 230 * 6;
}
int Keba_P30::Manageable_Keba_P30::get_maximal_consumption(){
	return 3 * 230 * 16;
}

Meter* Keba_P30::retrieve_meter(){
	return new Meter_Keba_P30(this);
}

Manageable* Keba_P30::retrieve_manageable(){
	return new Manageable_Keba_P30(this);
}


Keba_P30::Meter_Keba_P30::Meter_Keba_P30(Keba_P30* keba_p30){
	Meter_Keba_P30::keba_p30 = keba_p30;
}

Keba_P30::Meter_Keba_P30::~Meter_Keba_P30(){
}

Keba_P30::Manageable_Keba_P30::Manageable_Keba_P30(Keba_P30* keba_p30){
	Manageable_Keba_P30::keba_p30 = keba_p30;
	is_enabled = -1;
	minimal_consumption_possible = 0;
}

Keba_P30::Manageable_Keba_P30::~Manageable_Keba_P30(){
}