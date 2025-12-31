/* Class Config represents the values of a config file. Instances can be created by a path of a config file.

   Class Device_Config represents the config part a device (such as a meter or
   something that acts like one) or a manageable device (such as a charger,
   a heat pump or a virtual meter to bring data to devies that want their data "pulled").
   
   Class Energy_Manager_Config contains the data to create a Energy_Manager instance.
   */

#ifndef _Config_H___
#define _Config_H___

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "nlohmann/json.hpp"

#define MAXLINE 128

class Device_Config{
	public:
		nlohmann::json json;
};

class Energy_Manager_Config{
	public:
		nlohmann::json json;
};

class Config{

	public:
		Config(std::string file_path);
		
		int save();
		
		int get_http_port();
		std::vector<Device_Config> get_devices();
		std::vector<Energy_Manager_Config> get_energy_managers();
		
		bool set_energy_manager_regulation(std::string device_id, int regulation_type, int regulation_value);
		
	private:
		
		std::string file_name;
		nlohmann::json json;
		void load_config_file();
		
};

#endif