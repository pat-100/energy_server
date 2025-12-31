/*The main thing*/

#ifndef _MAIN_H___
#define _MAIN_H___

#include <string>
#include "SML_Meter.hpp"
#include "BnE_Meter.hpp"
#include "Keba_P30.hpp"
#include "Virtual_ALE3_Meter.hpp"
#include "Energy_Manager.hpp"
#include "Fronius_Gen24_Meter.hpp"
#include "Virtual_Fronius_Meter.hpp"

#include "Config.hpp"
#include "httpd.hpp"

/*Handles a http request for meter data.
  Parameter <file>: a valid file handle with an open TCP connection.
  Parameter <meter_id>: A meter ID as used in config file. If a matching meter is found its values get fetched and written to socket. If "", parameter <meter_serial> is used instead.
  Parameter <meter_serial>: Only applies when <meter_id> is empty string. Tries to find a meter with this serial number to fetch and return its data. If empty data for all meters will be returned.
*/
void handle_meter_request(int file, std::string meter_id="", std::string meter_serial="");

/*Handles request to get or set properties of a manageable device.
  Parameter <file>: A valid file handle for an open TCP connection.
  Parameter <parameter_list>: Name value list of http query. A value of parameter "device_id" can be used to identify a manageable device with the id given in config file. A parameter with name
  "managed_device_action" is "0" the function will reply the properties in JSON format to the TCP socket. If "managed_device_action" is "1" and parameters "regulation_type" and "regulation_value"
  are given, the function will try to set new properties to the manageable device.*/
void handle_http_managed_device_action(int file, struct header_s* parameter_list);
  
//helper method to fetch data from a Meter <meter> and serialise it to JSON. The result gets written to <file>.:
void print_meter_data_as_json(int file, Meter* meter);

//Helper when meters are initialised:
void append_meter_to_list(Meter* meter);

//Helper when energy managers are initialised:
void append_manageable_to_list(Manageable* manageable);

//Tries to create an instance for a meter for a device config:
void init_device(Device_Config device_configuration);

//Creates an instance of Energy_Manager by an EM config:
void init_manager(Energy_Manager_Config energy_manager_config);

//Returns a manageable device which matches <manageable_id>. Returns NULL if no match found.
Manageable* find_manageable(std::string manageable_id);

//Returns a meter device which matches <meter_id>. Returns NULL if no match found.
Meter* find_meter(std::string meter_id);

//Returns a meter device whose serial matches <meter_serial>. Returns NULL if no match found.
Meter* find_meter_by_serial(std::string meter_serial);

#endif