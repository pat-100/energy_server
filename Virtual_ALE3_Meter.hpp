#ifndef _METER_ALE3_H___
#define _METER_ALE3_H___

#include <pthread.h>
#include <string>
#include "Manageable.hpp"

#include "/usr/local/include/modbus/modbus.h"



class Virtual_ALE3_Meter:public Manageable{
	
	pthread_t thread;
	std::string device;
	modbus_t* modbus_ctx;
	
	modbus_mapping_t *mapping;
		
	static void* handle_thread(void *lpParam);
	void manage_request();
	
	void connect();
	void disconnect();
	void reconnect();
	
	void init_response_message();
	
	public:
		Virtual_ALE3_Meter(std::string device);
		~Virtual_ALE3_Meter();
		int get_minimal_consumption()override;
		int get_maximal_consumption()override;
	
		enum error{
			INVALID_CRC = 112345690,
			CONNECTION_TIMEOUT = 110
		};
	
};

#endif