#include "main.hpp"

Config* config;

std::vector<Meter*> meters;
std::vector<Manageable*> manageables;

std::vector<Energy_Manager*> energy_managers;

std::string condense_string(std::string s); //<- todo besseren Platz finden

int main(int c, char** v)
{   

	//load configuration:
	config = new Config("energy_server_config.json");
	
	//init devices:
	for(Device_Config device_config : config->get_devices())init_device(device_config);

	//prepare and start energy managers:
	for(Energy_Manager_Config energy_manager_config : config->get_energy_managers())init_manager(energy_manager_config);

	//run http service:
	std::cerr << "\nRunning Server on port: " << config->get_http_port();
	serve_forever(std::to_string((uint16_t)config->get_http_port()).c_str());
	
    return 0;
}

void init_device(Device_Config device_configuration){
	
	const nlohmann::json* device_config = &(device_configuration.json);
	
	std::string device_type = device_config->at("type");
	Device* new_device;
	
	if(device_type=="BnE_modbus"){
		Modbus_Connection_RTU* modbus = new Modbus_Connection_RTU(device_config->at("path"));
		modbus->set_reading_retries(device_config->at("reading_retries"));
		for(nlohmann::json slave_config : device_config->at("slaves")){
			append_meter_to_list((Meter*)(new_device = new BnE_Meter(modbus, slave_config.at("address"))));
			new_device->set_meta_fields(slave_config.at("id"), slave_config.at("description"));
		}
		return;
	}
	else if(device_type=="Keba_P30"){
		Keba_P30* new_keba_p30 = new Keba_P30(device_config->at("host"), device_config->at("port"));
		Meter* new_meter_keba_p30 = new_keba_p30->retrieve_meter();
		new_meter_keba_p30->set_meta_fields(device_config->at("id"), device_config->at("description"));
		append_meter_to_list(new_meter_keba_p30);
		Manageable* new_manageable_keba_p30 = new_keba_p30->retrieve_manageable();
		new_manageable_keba_p30->set_meta_fields(device_config->at("id"), device_config->at("description"));
		append_manageable_to_list(new_manageable_keba_p30);
		return;
	}
	else if(device_type=="virtual_ale3_device"){
		append_manageable_to_list((Manageable*)(new_device = new Virtual_ALE3_Meter(device_config->at("path"))));
	}
	else if(device_type=="sml_interface"){
		append_meter_to_list((Meter*)(new_device = new SML_Meter(device_config->at("path"))));
	}
	else if(device_type=="Fronius_Gen24_Meter"){
		append_meter_to_list((Meter*)(new_device = new Fronius_Gen24_Meter(device_config->at("host"), device_config->at("port"))));
	}
	else if(device_type=="Virtual_Fronius_Meter"){
		append_manageable_to_list((Manageable*)(new_device = new Virtual_Fronius_Meter(device_config->at("port"))));
	}
	else{
		fprintf(stderr, "\nDevice-Type \"%s\" could not be parsed.",device_type.c_str());
		return;
	}
	new_device->set_meta_fields(device_config->at("id"), device_config->at("description"));
		 
}

void init_manager(Energy_Manager_Config energy_manager_config){
	
	const nlohmann::json* em_config = &(energy_manager_config.json);
	
	if(0 == em_config->at("interval")){
		std::cerr << "\nEnergymanager inaktiv (interval=0): " << em_config->at("id");
		return;
	}

	Manageable* target_device = find_manageable(em_config->at("target"));
	if(target_device==NULL){
		std::cerr << "\nZiel-Device wurde nicht initialisiert: " << em_config->at("id");
		return;
	}
	
	Calculation_Term* calculation_rule = Calculation_Term::create_by_string(condense_string(em_config->at("calculation_rule")), &meters);
	if(target_device==NULL){
		std::cerr << "\nCalculation rule could not be parsed: " << em_config->at("id");
		return;
	}	
	
	Energy_Manager* energy_manager = new Energy_Manager(target_device,
															em_config->at("id"),
															em_config->at("interval"),
															calculation_rule,
															em_config->at("regulation_type"),
															em_config->at("regulation_value") );
															
	energy_managers.push_back(energy_manager);
	
}

void append_meter_to_list(Meter* meter){
	meters.push_back(meter);
}

void append_manageable_to_list(Manageable* manageable){
	manageables.push_back(manageable);
}

Meter* find_meter(std::string meter_id){
	for(Meter* meter:meters)if(meter->get_id() == meter_id)return meter;
	return NULL;
}

Meter* find_meter_by_serial(std::string meter_serial){
	for(Meter* meter:meters)if(meter->get_serial() == meter_serial)return meter;
	return NULL;
}

Manageable* find_manageable(std::string manageable_id){
	for(Manageable* manageable:manageables)if(manageable->get_id() == manageable_id)return manageable;
	return NULL;
}

void route(int file, struct http_req_s *http_req){
	
	struct header_s* parameter_list = get_parameter_list(http_req->qs);
	
	if(get_param_value(parameter_list, "managed_device_action"))handle_http_managed_device_action(file, parameter_list);
	else if(get_param_value(parameter_list, "meter_id"))handle_meter_request(file,get_param_value(parameter_list, "meter_id"));
	else if(get_param_value(parameter_list, "meter_serial"))handle_meter_request(file,"",get_param_value(parameter_list, "meter_serial"));
	else handle_meter_request(file, "");
		
	destruct_query_parameter(parameter_list);
	                                                                    
}

void handle_meter_request(int file, std::string meter_id, std::string meter_serial){
	
	//generate http header:
	dprintf(file, "HTTP/1.1 200 OK\r\n");
	dprintf(file, "Content-Type: application/json;\r\n");
	dprintf(file, "Connection: close\r\n");
	dprintf(file, "\r\n");
		
	//get data of matching meters and build json content:
	dprintf(file, "{\"meterdata\":[");
	Meter* meter = NULL;
	if(meter_id!="")meter = find_meter(meter_id);		
	else if(meter_serial!="")meter = find_meter_by_serial(meter_serial);
	if(meter)print_meter_data_as_json(file, meter);
	else{
		int n=0;
		for(Meter* meter:meters){
			if(n>0)dprintf(file, ",");
			print_meter_data_as_json(file, meter);
			++n;
		}
	}
	dprintf(file, "] }");
		
}

void print_meter_data_as_json(int file, Meter* meter){
	Meter_Data meter_data = meter->get_meter_data();
	dprintf(file, "{ \"id\":\"%s\", \"description\":\"%s\", \"serial\":\"%s\", \"last_time_read\":%d, \"register_grid\":%d, \"register_generation\":%d, \"current_power\":%d, \"register_resetable\":%d }",
		meter->get_id().c_str(), meter->get_description().c_str(), meter_data.get_serial().c_str(), meter_data.last_time_read, meter_data.register_grid, meter_data.register_generation, meter_data.current_power, meter_data.register_resetable);
}

void handle_http_managed_device_action(int file, struct header_s* parameter_list){

	char action = ((std::string)get_param_value(parameter_list, "managed_device_action")).at(0);
	char* c_device_id = get_param_value(parameter_list, "device_id");
	std::string device_id = "";
	if(c_device_id)device_id = c_device_id;
	switch(action){
		case '0': //get
			if(device_id!=""){
				for(Energy_Manager* em : energy_managers){
					if(em->get_id() == device_id){
						Energymanager_Options cur_device = em->get_device_data();
						respond_http(file, "200 OK", "");
						dprintf(file, "{\"device_id\":\"%s\", \"minimal_consumption_possible\":%i, \"maximal_consumption_possible\":%i, \"regulation_type\":%i, \"regulation_value\":%i }" , 
						em->get_id().c_str(), cur_device.minimal_consumption_possible, cur_device.maximal_consumption_possible, cur_device.regulation_type, cur_device.regulation_value);
						return;
					}
				}
			}
			respond_http(file, "404 NOK", "{\"message\":\"No Manager with matching ID found.\"}");
			break;
		case '1'://set
			std::string regulation_type = get_param_value(parameter_list, "regulation_type");
			std::string regulation_value = get_param_value(parameter_list, "regulation_value");
			if(regulation_type!="" && regulation_value!=""){
			enum REGULATION_TYPE new_regulation_type = (enum REGULATION_TYPE)std::stoi(regulation_type);
			int new_regulation_value = std::stoi(regulation_value);
				if(device_id!="" && (new_regulation_type == 0 || new_regulation_type == 1)){
					for(Energy_Manager* em : energy_managers){
						if(em->get_id() == device_id){
							if(em->set_regulation(new_regulation_type, new_regulation_value)){							
								config->set_energy_manager_regulation(device_id, new_regulation_type, new_regulation_value);
								if(config->save()==0)respond_http(file, "201 OK", "{\"message\":\"Successfully set\"}");
								else respond_http(file, "500 NOK", "{\"message\":\"Error saving configuration.\"}");
								return;
							}
							else respond_http(file, "500 NOK", "{\"message\":\"Could not be set.\"}");
						}
					}
				}
			}
	}
	
	respond_http(file, "500 NOK", "{\"message\":\"Failed\"}");

}

std::string condense_string(std::string s){
	std::string result;
	for(char c:s)if(c!=' ')result+=c;
	return result;
}