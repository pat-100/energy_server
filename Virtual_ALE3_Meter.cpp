#include "Virtual_ALE3_Meter.hpp"

Virtual_ALE3_Meter::Virtual_ALE3_Meter(std::string device){
	
	Virtual_ALE3_Meter::device = device;
	mapping = NULL;
	init_response_message();
		
	fprintf(stderr, "\nALE3: Starte...");	
	pthread_create(&thread, NULL, handle_thread, this);

}

Virtual_ALE3_Meter::~Virtual_ALE3_Meter(){
	thread = 0;
	disconnect();
	modbus_mapping_free(mapping);
}

void* Virtual_ALE3_Meter::handle_thread(void* object){
	((Virtual_ALE3_Meter*)object)->manage_request();
	return NULL;
}

void Virtual_ALE3_Meter::manage_request(){
		
	while(thread){	
		uint8_t request[MODBUS_RTU_MAX_ADU_LENGTH];// request buffer
		int len;// length of the request/response
		int target_power = get_target_power();
		mapping->tab_registers[37] = target_power / 30;
		mapping->tab_registers[42] = target_power / 30;	
		mapping->tab_registers[47] = target_power / 30;
		init_response_message();
		len = modbus_receive(modbus_ctx, request);
		
		if (len == -1){
			if(errno != Virtual_ALE3_Meter::INVALID_CRC && errno != Virtual_ALE3_Meter::CONNECTION_TIMEOUT && errno!=104 && errno != 11)fprintf(stderr,"\nALE3: Error on receive: %i %s\n",errno, modbus_strerror(errno));
			reconnect();
		}
		else {
			if( len != 0){
				len = modbus_reply(modbus_ctx, request, len, mapping);
			}
		}
	}

}

void Virtual_ALE3_Meter::init_response_message(){
	
	if(mapping)modbus_mapping_free(mapping);
	mapping = modbus_mapping_new(0, 0, 52, 0);
	if (!(mapping)) {
		printf("Failed to allocate the mapping: %s\n", modbus_strerror(errno));
		exit(1);
	}
	int power = get_target_power();
	mapping->tab_registers[0] = 10; //Firmware
	mapping->tab_registers[21] = 0; //Comm ok
	mapping->tab_registers[24] = 0; //Metering ok
	mapping->tab_registers[27] = 1; //Counter 1 Total high (in 10 Wh)
	mapping->tab_registers[28] = 2; //Counter 1 Total low (in 10 Wh)
	mapping->tab_registers[29] = 3; //Counter 1 Partial high (in 10 Wh)
	mapping->tab_registers[30] = 4; //Counter 1 Partial low (in 10 Wh)
	mapping->tab_registers[31] = 5; //Counter 2 Total high (in 10 Wh)
	mapping->tab_registers[32] = 6; //Counter 2 Total low (in 10 Wh)
	mapping->tab_registers[33] = 9; //Counter 2 Partial high (in 10 Wh)
	mapping->tab_registers[34] = 12; //Counter 2 Partial low (in 10 Wh)
	mapping->tab_registers[37] = power / 30; //Power Phase 1 (in 10W)
	mapping->tab_registers[42] = power / 30; //Power Phase 2 (in 10W)
	mapping->tab_registers[47] = power / 30; //Power Phase 3 (in 10W)
	
}

void Virtual_ALE3_Meter::reconnect(){
	disconnect();
	connect();
}

void Virtual_ALE3_Meter::connect(){
	
	modbus_ctx = modbus_new_rtu(device.c_str(), 19200, 'E', 8, 1);
	
	if (!modbus_ctx) {
		printf("Failed to create the context: %s\n", modbus_strerror(errno));
		exit(1);
	}

	//Set the Modbus address of this slave (to 3)
	modbus_set_slave(modbus_ctx, 60);

	if (modbus_connect(modbus_ctx) == -1) {
		printf("Unable to connect: %s\n", modbus_strerror(errno));
		modbus_free(modbus_ctx);
		modbus_ctx = NULL;
		sleep(60);
		return;
	}
	
}

void Virtual_ALE3_Meter::disconnect(){
	
	modbus_close(modbus_ctx);
	modbus_free(modbus_ctx);
	
}

int Virtual_ALE3_Meter::get_minimal_consumption(){
	return 0;
}
int Virtual_ALE3_Meter::get_maximal_consumption(){
	return 2 * 3 * 230 * 16;
}