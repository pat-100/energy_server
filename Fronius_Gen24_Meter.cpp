#include "Fronius_Gen24_Meter.hpp"

Fronius_Gen24_Meter::Fronius_Gen24_Meter(std::string host, uint16_t port){
	Fronius_Gen24_Meter::host = host;
	Fronius_Gen24_Meter::port = port;
	try_connecting();
}

Fronius_Gen24_Meter::~Fronius_Gen24_Meter(){
	modbus_free(ctx);
}

std::string Fronius_Gen24_Meter::get_serial(){
	return read_string(40052, 16);
};

std::string Fronius_Gen24_Meter::read_string(int start_register, int max_len){
	
	uint16_t* buffer = (uint16_t*)malloc(sizeof(uint16_t) * max_len);
	int rc = modbus_read_registers(ctx, start_register, max_len, buffer);
	if (rc == -1) {
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return std::string("");
	}

	char* s = (char*)malloc(sizeof(uint16_t) * max_len);
	for(int i=0; i<rc; ++i){
		s[i*2+0]=MODBUS_GET_HIGH_BYTE(buffer[i]);
		s[i*2+1]=MODBUS_GET_LOW_BYTE(buffer[i]);
	}
	
	std::string string = std::string(s);
	free(buffer);
	free(s);
	return string;
	
}

int Fronius_Gen24_Meter::scale(const int16_t value, const int16_t exp){
	int result = value;
	if(exp>0)for(int i=exp; i; --i)result *= 10;
	else if(exp<0)for(int i=exp; i; ++i)result /= 10;
	return result;
}

float Fronius_Gen24_Meter::read_float(int reg){
	
	uint16_t buffer[2];
	int rc = modbus_read_registers(ctx, reg, 2, buffer);
	if(rc != 2){
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return -1;
	}
	
	return modbus_get_float_dcba(buffer);
}

int Fronius_Gen24_Meter::get_register_grid(){
	return -1;
}

int Fronius_Gen24_Meter::get_register_generation(){
	return read_single_register(40093);
}

int Fronius_Gen24_Meter::get_current_power(){
	//int16_t scale = read_single_register(40084);
	
	//fprintf(stderr, "\nscale:%i",scale_W_SF);
	//fprintf(stderr, "\nmax:%i",read_single_register(40171));
	int16_t power = read_single_register_signed(40083);
	
	return scale(power, scale_W_SF);
}

Meter_Data Fronius_Gen24_Meter::get_meter_data(){
	Meter_Data meter_data;
	meter_data.register_grid = get_register_grid();
	meter_data.register_generation = get_register_generation();
	meter_data.register_resetable = -1;
	meter_data.current_power = get_current_power();
	meter_data.serial = get_serial();
	meter_data.last_time_read = time(NULL);
	return meter_data;
}

int Fronius_Gen24_Meter::write_double_registers(int address, uint32_t value){
	uint16_t buffer[2];
	buffer[0] = value>>16;
	buffer[1] = value;	
    int returned_n = modbus_write_registers(ctx, address, 2, buffer);
	if(returned_n==2)return 0;
	else{
		try_connecting();
		return returned_n;
	}
}

int Fronius_Gen24_Meter::write_single_register(int address, uint16_t value){
    int returned_n = modbus_write_register(ctx, address, value);
	if(returned_n==1)return 0;
	else{
		try_connecting();
		return -1;
	}
}

uint32_t Fronius_Gen24_Meter::read_double_registers(int address){
	uint16_t buffer[2];
	int returned_n = modbus_read_registers(ctx, address, 2, buffer);
	if(returned_n==2)return (buffer[0]<<16)+buffer[1];
	else{
		try_connecting();
		return -1;
	}
}

uint16_t Fronius_Gen24_Meter::read_single_register(int address){
	uint16_t buffer;
	int returned_n = modbus_read_registers(ctx, address, 1, &buffer);
	if(returned_n==1)return buffer;
	else{
		try_connecting();
		return -1;
	}
}

int16_t Fronius_Gen24_Meter::read_single_register_signed(int address){
	int16_t buffer;
	int returned_n = modbus_read_registers(ctx, address, 1, (uint16_t*)&buffer);
	if(returned_n==1)return buffer;
	else{
		try_connecting();
		return -1;
	}
}

void* Fronius_Gen24_Meter::alloc_and_read_registers(int address, int n){
	uint16_t* buffer = (uint16_t *)malloc(n * sizeof(uint16_t));
	int returned_n = modbus_read_registers(ctx, address, n, buffer);
	if(returned_n==n)return buffer;
	else{
		try_connecting();
		free(buffer);
		return NULL;
	}
}

bool Fronius_Gen24_Meter::try_connecting(){
	ctx = NULL;
	for(int i=0;i<1&&ctx==NULL;++i)ctx = create_tcp_mobus_connection(host.c_str(), port);
		
	init();
	return ctx ? true : false;
}

void Fronius_Gen24_Meter::init(){
	modbus_set_slave(ctx, 1); //inverter
	//modbus_set_response_timeout(ctx, 5, 0);
	scale_W_SF = read_single_register_signed(40084);
	//fprintf(stderr, "\nlimp:%i", read_single_register_signed(40248));
	//fprintf(stderr, "\nlim:%i", read_single_register_signed(40249));
}

modbus_t* Fronius_Gen24_Meter::create_tcp_mobus_connection(std::string host, uint16_t port){
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(host.c_str(), port);
	
	if (ctx == NULL) {
      printf( "Unable to allocate libmodbus context\n");
      return NULL;
    }
	
	//modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));
		
	uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;	
	
	modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);
	
    if (modbus_connect(ctx) == -1) {
        printf( "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return NULL;
    }
		
	return ctx;
}

time_t Fronius_Gen24_Meter::get_last_time_read(){
	return time(NULL);
}