#include "Config.hpp"

Config::Config(std::string file_path){
	file_name = file_path;
	load_config_file();
}

void Config::load_config_file(){
	std::ifstream fJson(file_name);
    std::stringstream buffer;
    buffer << fJson.rdbuf();
    json = nlohmann::json::parse(buffer.str());
}

int Config::get_http_port(){
	return json["http_port"];
}

std::vector<Device_Config> Config::get_devices(){
	std::vector<Device_Config> result;
	for(nlohmann::json device : json["devices"]){
		Device_Config new_devices_config;
		new_devices_config.json = device;
		result.push_back(new_devices_config);
	}
	return result;
}

std::vector<Energy_Manager_Config> Config::get_energy_managers(){
	std::vector<Energy_Manager_Config> result;
	for(nlohmann::json em_json : json["energy_managers"]){
		Energy_Manager_Config new_em_config;
		new_em_config.json = em_json;
		result.push_back(new_em_config);
	}
	return result;
}

bool Config::set_energy_manager_regulation(std::string device_id, int regulation_type, int regulation_value){
	
	auto& energy_managers = json.at("energy_managers");
	for (auto&& energy_manager : energy_managers){
		if(energy_manager.at("id") == device_id){
			energy_manager["regulation_type"] = regulation_type;
			energy_manager["regulation_value"] = regulation_value;
			return true;
		}
	}
	
	return false;
	
}

int Config::save(){
	
    std::ofstream file(file_name.c_str());
	file << json << std::endl;
	file.close();
	
	return 0;

}