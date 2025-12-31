#include "Virtual_Fronius_Meter.hpp"

Virtual_Fronius_Meter::Virtual_Fronius_Meter(uint16_t port){
	
	modbus_ctx = modbus_new_tcp(NULL, port);
	if (modbus_ctx == NULL) {
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return;
	}
	
	modbus_set_debug(modbus_ctx, TRUE);
	mapping = NULL;
	
	/*if (modbus_connect(modbus_ctx) == -1) {
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(modbus_ctx);
		return;
	}*/
	
	init_response_message();
		
	fprintf(stderr, "\nVirtual Fronius SmartMeterIP: Starte...");	
	pthread_create(&thread, NULL, handle_thread, this);

}

Virtual_Fronius_Meter::~Virtual_Fronius_Meter(){
	thread = 0;
	disconnect();
	modbus_mapping_free(mapping);
}

void* Virtual_Fronius_Meter::handle_thread(void* object){
	((Virtual_Fronius_Meter*)object)->manage_request();
	return NULL;
}

void Virtual_Fronius_Meter::manage_request(){
		
	while(thread){
		//FD_ZERO(&refset);
		//FD_SET(server_socket, &refset);
		int server_socket = modbus_tcp_listen(modbus_ctx, 1);
		if(server_socket==-1){
			fprintf(stderr,"\nVirtual Fronius SmartMeterIP: Socket error: %i %s\n",errno, modbus_strerror(errno));
			sleep(5);
			continue;
		}
		
		int client_socket;
        while(client_socket = modbus_tcp_accept(modbus_ctx, &server_socket)){
			if(client_socket==-1){
				fprintf(stderr,"\nVirtual Fronius SmartMeterIP: Connect error: %i %s\n",errno, modbus_strerror(errno));
				sleep(5);
				continue;
			}
		
			uint8_t request[MODBUS_TCP_MAX_ADU_LENGTH];//request buffer
			int len;// length of the request/response
			int target_power = get_target_power();
			mapping->tab_registers[37] = target_power / 30;
			mapping->tab_registers[42] = target_power / 30;	
			mapping->tab_registers[47] = target_power / 30;
			//init_response_message();
			while(len = modbus_receive(modbus_ctx, request)){
				if (len == -1){
					if(errno != Virtual_Fronius_Meter::INVALID_CRC && errno != Virtual_Fronius_Meter::CONNECTION_TIMEOUT && errno!=104 && errno != 11)fprintf(stderr,"\nVirtual Fronius SmartMeterIP: Error on receive: %i %s\n",errno, modbus_strerror(errno));
					//close(client_socket);
					//continue;
					//reconnect();
					exit;
				}
				else {
					if(len != 0){
						len = modbus_reply(modbus_ctx, request, len, mapping);
						fprintf(stderr,"\nVirtual Fronius SmartMeterIP: Modbus responded: %i \n",len);
					}
				}
			}
			close(client_socket);
		}
		
		
		close(server_socket);
		modbus_free(modbus_ctx);
	}

}

void Virtual_Fronius_Meter::init_response_message(){
	
	if(mapping)modbus_mapping_free(mapping);
	//mapping = modbus_mapping_new(0, 0, 100, 0);
	mapping = modbus_mapping_new_start_address(0, 0, 0, 0, 0, 200, 0, 0);
	if (!(mapping)) {
		printf("Failed to allocate the mapping: %s\n", modbus_strerror(errno));
		exit(1);
	}
	
	int power = get_target_power();
	mapping->tab_registers[0] = 21365;
	mapping->tab_registers[1] = 28243;
	mapping->tab_registers[2] = 1;
	mapping->tab_registers[3] = 65; 
	//Manufacturer "Fronius":
	mapping->tab_registers[4] = 70;
	mapping->tab_registers[5] = 114;
	mapping->tab_registers[6] = 111;
	mapping->tab_registers[7] = 110;
	mapping->tab_registers[8] = 105;
	mapping->tab_registers[9] = 117;
	mapping->tab_registers[10] = 115;
	mapping->tab_registers[11] = 0;
	mapping->tab_registers[12] = 0;
	mapping->tab_registers[13] = 0;
	mapping->tab_registers[14] = 0;
	mapping->tab_registers[15] = 0;
	mapping->tab_registers[16] = 0;
	mapping->tab_registers[17] = 0;
	mapping->tab_registers[18] = 0;
	mapping->tab_registers[19] = 0;
	//Device Model "Smart Meter 63A":
	mapping->tab_registers[20] = 83;
	mapping->tab_registers[21] = 109;
	mapping->tab_registers[22] = 97;
	mapping->tab_registers[23] = 114;
	mapping->tab_registers[24] = 116;
	mapping->tab_registers[25] = 32;
	mapping->tab_registers[26] = 77;
	mapping->tab_registers[27] = 101;
	mapping->tab_registers[28] = 116;
	mapping->tab_registers[29] = 101;
	mapping->tab_registers[30] = 114;
	mapping->tab_registers[31] = 32;
	mapping->tab_registers[32] = 54;
	mapping->tab_registers[33] = 51;
	mapping->tab_registers[34] = 65;
	mapping->tab_registers[35] = 0;
	//Options N/A:
	mapping->tab_registers[36] = 0;
	mapping->tab_registers[37] = 0;
	mapping->tab_registers[38] = 0;
	mapping->tab_registers[39] = 0;
	mapping->tab_registers[40] = 0;
	mapping->tab_registers[41] = 0;
	mapping->tab_registers[42] = 0;
	mapping->tab_registers[43] = 0;
	//Software Version  N/A:
	mapping->tab_registers[44] = 0;
	mapping->tab_registers[45] = 0;
	mapping->tab_registers[46] = 0;
	mapping->tab_registers[47] = 0;
	mapping->tab_registers[48] = 0;
	mapping->tab_registers[49] = 0;
	mapping->tab_registers[50] = 0;
	mapping->tab_registers[51] = 0;
	//Serial Number: 00000001:
	mapping->tab_registers[52] = 48;
	mapping->tab_registers[53] = 48;
	mapping->tab_registers[54] = 48;
	mapping->tab_registers[55] = 48;
	mapping->tab_registers[56] = 48;
	mapping->tab_registers[57] = 48;
	mapping->tab_registers[58] = 48;
	mapping->tab_registers[59] = 49;
	mapping->tab_registers[60] = 0;
	mapping->tab_registers[61] = 0;
	mapping->tab_registers[62] = 0;
	mapping->tab_registers[63] = 0;
	mapping->tab_registers[64] = 0;
	mapping->tab_registers[65] = 0;
	mapping->tab_registers[66] = 0;
	mapping->tab_registers[67] = 0;
	mapping->tab_registers[68] = 240; //Modbus TCP Address: 240
	
	mapping->tab_registers[69] = 213;
	mapping->tab_registers[70] = 124;
	
	for(int i=71;i<=194;++i)mapping->tab_registers[i] = 0;
	
	mapping->tab_registers[195] = 65535;
	mapping->tab_registers[196] = 0;
	
	
	 /*       40001:  [21365, 28243], 
        40003:  [1],
        40004:  [65],
        40005:  [70,114,111,110,105,117,115,0,0,0,0,0,0,0,0,0,         #Manufacturer "Fronius"
		83,109,97,114,116,32,77,101,116,101,114,32,54,51,65,0, #Device Model "Smart Meter 63A"
		0,0,0,0,0,0,0,0,                                       #Options N/A
                0,0,0,0,0,0,0,0,                                       #Software Version  N/A
		48,48,48,48,48,48,48,49,0,0,0,0,0,0,0,0,               #Serial Number: 00000001 (49,54,50,50,48,49,49,56
                240],                                                  #Modbus TCP Address: 240
        40070: [213],
        40071: [124], 
        40072: [0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0],

                40196: [65535, 0],*/
	
}

void Virtual_Fronius_Meter::reconnect(){
	disconnect();
	connect();
}

void Virtual_Fronius_Meter::connect(){
	
	modbus_ctx = modbus_new_rtu(device.c_str(), 19200, 'E', 8, 1);
	
	if (!modbus_ctx) {
		printf("Failed to create the context: %s\n", modbus_strerror(errno));
		exit(1);
	}

	//Set the Modbus address of this slave
	modbus_set_slave(modbus_ctx, 1);

	if (modbus_connect(modbus_ctx) == -1) {
		printf("Unable to connect: %s\n", modbus_strerror(errno));
		modbus_free(modbus_ctx);
		return;
	}
	
}

void Virtual_Fronius_Meter::disconnect(){
	modbus_close(modbus_ctx);
	modbus_free(modbus_ctx);
}

int Virtual_Fronius_Meter::get_minimal_consumption(){
	return 0;
}
int Virtual_Fronius_Meter::get_maximal_consumption(){
	return 2 * 3 * 230 * 16;

}

