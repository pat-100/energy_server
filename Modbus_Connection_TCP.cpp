#include "Modbus_Connection_TCP.hpp"

Modbus_Connection_TCP::Modbus_Connection_TCP(std::string host, uint16_t port){
	
	Modbus_Connection_TCP::host = host;
	Modbus_Connection_TCP::port = port;

	connect();	
	
	if(!modbus_ctx){
		fprintf(stderr, "Fehler beim Initialisieren von Modbus");
		return;
	}
}

Modbus_Connection_TCP::~Modbus_Connection_TCP(){
	modbus_close(modbus_ctx);
	modbus_free(modbus_ctx);
}

bool Modbus_Connection_TCP::connect(){
	modbus_ctx = NULL;
	for(int i=0;i<1&&modbus_ctx==NULL;++i)create_tcp_mobus_connection();
	return modbus_ctx ? true : false;
}

std::string Modbus_Connection_TCP::get_host(){
	return host;
}

uint16_t Modbus_Connection_TCP::get_port(){
	return port;
}

void Modbus_Connection_TCP::create_tcp_mobus_connection(){
	
	modbus_ctx = NULL;
	modbus_ctx = modbus_new_tcp(host.c_str(), port);
	
	if (modbus_ctx == NULL) {
      printf( "Unable to allocate libmodbus context\n");
    }
	
	//modbus_set_debug(TRUE);
    modbus_set_error_recovery(modbus_ctx, (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));
		
	uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;	
	
	modbus_get_response_timeout(modbus_ctx, &old_response_to_sec, &old_response_to_usec);
	
    if (modbus_connect(modbus_ctx) == -1) {
        printf( "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(modbus_ctx);
        modbus_ctx = NULL;
    }
		
}

int Modbus_Connection_TCP::write_double_registers(int address, uint32_t value){
	uint16_t buffer[2];
	buffer[0] = value>>16;
	buffer[1] = value;	
    int returned_n = modbus_write_registers(modbus_ctx, address, 2, buffer);
	if(returned_n==2)return 0;
	else{
		connect();
		return returned_n;
	}
}

int Modbus_Connection_TCP::write_single_register(int address, uint16_t value){
    int returned_n = modbus_write_register(modbus_ctx, address, value);
	if(returned_n==1)return 0;
	else{
		connect();
		return -1;//returned_n;
	}
}

uint32_t Modbus_Connection_TCP::read_double_registers(int address){
	uint16_t buffer[2];
	int returned_n = modbus_read_registers(modbus_ctx, address, 2, buffer);
	if(returned_n==2)return (buffer[0]<<16)+buffer[1];
	else{
		connect();
		return -1;
	}
}

uint16_t Modbus_Connection_TCP::read_single_register(int address){
	uint16_t buffer[1];
	int returned_n = modbus_read_registers(modbus_ctx, address,1,  buffer);
	if(returned_n==2)return (buffer[0]);
	else{
		connect();
		return -1;
	}
}

void* Modbus_Connection_TCP::alloc_and_read_registers(int address, int n){
	uint16_t* buffer = (uint16_t *)malloc(n * sizeof(uint16_t));
	int returned_n = modbus_read_registers(modbus_ctx, address, n, buffer);
	if(returned_n==n)return buffer;
	else{
		connect();
		free(buffer);
		return NULL;
	}
}