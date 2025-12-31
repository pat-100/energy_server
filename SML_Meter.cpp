#include "SML_Meter.hpp"

SML_Meter::SML_Meter(std::string device_path){
	device = device_path;
	pthread_mutex_init(&data_lock, NULL);
	int err = pthread_create(&thread, NULL, &handle_thread, (void*)this);
}

SML_Meter::~SML_Meter(){
	thread = 0;
}

Meter_Data SML_Meter::get_meter_data(){
	pthread_mutex_lock(&data_lock);
	Meter_Data meter_data_copy = meter_data;
	pthread_mutex_unlock(&data_lock);
	return meter_data_copy;
}

std::string SML_Meter::get_serial(){
	pthread_mutex_lock(&data_lock);
	std::string serial = meter_data.get_serial();
	pthread_mutex_unlock(&data_lock);
	return serial;
}

void* SML_Meter::handle_thread(void* object){
	((SML_Meter*)object)->handle_thread();
	return NULL;
}

int SML_Meter::get_register_grid(){
	pthread_mutex_lock(&data_lock);
	auto register_grid = meter_data.register_grid;
	pthread_mutex_unlock(&data_lock);
	return register_grid;
}

int SML_Meter::get_register_generation(){
	pthread_mutex_lock(&data_lock);
	auto register_generation = meter_data.register_generation;
	pthread_mutex_unlock(&data_lock);
	return register_generation;
}
	
int SML_Meter::get_current_power(){
	pthread_mutex_lock(&data_lock);
	auto current_power = meter_data.current_power;
	pthread_mutex_unlock(&data_lock);
	return current_power;
}

void* SML_Meter::handle_thread(){
    
	//init data and COM port:
	int fd = serial_port_open(device.c_str());
	if(fd<0)fprintf(stderr, "\nPort could not be opened: %s", device.c_str());
	else fprintf(stderr, "\nPort opened: %s", device.c_str());
	sleep(5);
	while(thread){
		
		class Meter_Data meter_update;
		
		//bytes_received = 0;
		if(fd>=0)meter_update = get_sml_meter_data(fd);
		
		if(meter_update.get_serial()!=""){
			
			//apply recently fetched data to global data:
			pthread_mutex_lock(&(data_lock));
			meter_data = meter_update;
			pthread_mutex_unlock(&(data_lock));
		}
		else{
			//try to reinit COM port:
			if(serial_port_close(fd))fprintf(stderr, "\nPort could not be closed: %s", device.c_str());
			sleep(10);
			fd = serial_port_open(device.c_str());
			if(fd<0)fprintf(stderr, "\nPort could not be reopened: %s", device.c_str());
			else fprintf(stderr, "\nPort reopened: %s", device.c_str());
		}
			
	}
	
	pthread_mutex_destroy(&data_lock);
	
	return NULL;
		
}

time_t SML_Meter::get_last_time_read(){
	return meter_data.last_time_read;
}

Meter_Data get_sml_meter_data(int fd){
	
	class Meter_Data data;
	std::string serial = "";
	size_t bytes = read_sml_values(fd, &serial, &data.register_grid, &data.register_generation, &data.current_power);
	data.set_serial(serial);
	data.last_time_read = time(NULL);
	return data;
	
}

size_t read_sml_values(int fd, std::string* serial, int* reg_1_0_1_8_0,int* reg_1_0_2_8_0, int* reg_power){

	#define MC_SML_BUFFER_LEN 8096
	unsigned char buffer[MC_SML_BUFFER_LEN];

	size_t bytes = sml_transport_read(fd, buffer, MC_SML_BUFFER_LEN);
	
	//fprintf(stderr, "\nTestio:\n%i %i\n",fd, bytes);
	//for(int i=0;i<bytes;++i)fprintf(stderr, "%X ", buffer[i]);
	//return 0;
	
	
	if(bytes==0)return bytes;
	
	sml_file* file = sml_file_parse(buffer + 8, bytes - 16);
	
	if(file->messages_len == 0)fprintf(stderr, "sml_file ohne messages");

	for (int i = 0; i < file->messages_len; i++) {
		sml_message *message = file->messages[i];

		if (*message->message_body->tag == SML_MESSAGE_GET_LIST_RESPONSE) {
			sml_list *entry;
			sml_get_list_response *body;
			body = (sml_get_list_response *) message->message_body->data;

			for (entry = body->val_list; entry != NULL; entry = entry->next) {
				if (!entry->value) { // do not crash on null value
						fprintf(stderr, "Error in data stream. entry->value should not be NULL. Skipping this.\n");
						continue;
					}
				
				if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) || ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED)){					
					if(entry->obj_name->str[0]==0x01 && entry->obj_name->str[1]==0x00 && entry->obj_name->str[2] == 0x01 && entry->obj_name->str[3]==0x08 && entry->obj_name->str[4]==0x00){
						*reg_1_0_1_8_0 = get_value_of_entry(entry);
						
					}
					else if(entry->obj_name->str[0]==0x01 && entry->obj_name->str[1]==0x00 && entry->obj_name->str[2] == 0x02 && entry->obj_name->str[3]==8 && entry->obj_name->str[4]==0){
						*reg_1_0_2_8_0 = get_value_of_entry(entry);
					}
					else if(entry->obj_name->str[0]==0x01 && entry->obj_name->str[1]==0x00 && entry->obj_name->str[2] == 0x10 && entry->obj_name->str[3]==0x07 && entry->obj_name->str[4]==0x00){
						*reg_power = get_value_of_entry(entry);
					}
					else if(entry->obj_name->str[0]==0x01 && entry->obj_name->str[1]==0x00 && entry->obj_name->str[2] == 0x0F && entry->obj_name->str[3]==0x07 && entry->obj_name->str[4]==0x00){
						*reg_power = get_value_of_entry(entry);
					}
					/*else{
						char *str;
						printf("%d-%d:%d.%d.%d*%d#%s#\n",
						entry->obj_name->str[0], entry->obj_name->str[1],
						entry->obj_name->str[2], entry->obj_name->str[3],
						entry->obj_name->str[4],* entry->obj_name->str[5],
						sml_value_to_strhex(entry->value, &str, true));
					}*/
				}
				if(entry->value->type==SML_TYPE_OCTET_STRING){
					if((entry->obj_name->str[0]==0x01 && entry->obj_name->str[1]==0x00 && entry->obj_name->str[2] == 0x00 && entry->obj_name->str[3]==0x00 && entry->obj_name->str[4]==0x09) || 
					   (entry->obj_name->str[0]==0x01 && entry->obj_name->str[1]==0x00 && entry->obj_name->str[2] == 0x60 && entry->obj_name->str[3]==0x01 && entry->obj_name->str[4]==0x00) ){
						char* temp;
						*serial = "";
						sml_value_to_strhex(entry->value, &temp, true);
						char* pos = temp;
						while(*pos){
							*pos = toupper(*pos);
							if(*pos != ' ')(*serial) += *pos;
							++pos;
						}
						*pos='\0';
						free(temp);
					}
				}
				
				
				/*if (entry->value->type == SML_TYPE_OCTET_STRING) {
					char *str;
					printf("%d-%d:%d.%d.%d*%d#%s#\n",
						entry->obj_name->str[0], entry->obj_name->str[1],
						entry->obj_name->str[2], entry->obj_name->str[3],
						entry->obj_name->str[4], entry->obj_name->str[5],
						sml_value_to_strhex(entry->value, &str, true));
					free(str);
				} else if (entry->value->type == SML_TYPE_BOOLEAN) {
					printf("%d-%d:%d.%d.%d*%d#%s#\n",
						entry->obj_name->str[0], entry->obj_name->str[1],
						entry->obj_name->str[2], entry->obj_name->str[3],
						entry->obj_name->str[4], entry->obj_name->str[5],
						entry->value->data.boolean ? "true" : "false");
				} else if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
						((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED)) {
					double value = sml_value_to_double(entry->value);
					int scaler = (entry->scaler) ? *entry->scaler : 0;
					int prec = -scaler;
					if (prec < 0)
						prec = 0;
					value = value * pow_int(10, scaler);
					printf("%d-%d:%d.%d.%d*%d#%.*f#",
						entry->obj_name->str[0], entry->obj_name->str[1],
						entry->obj_name->str[2], entry->obj_name->str[3],
						entry->obj_name->str[4], entry->obj_name->str[5], prec, value);
					const char *unit = NULL;
					if (entry->unit &&  // do not crash on null (unit is optional)
						(unit = dlms_get_unit((unsigned char) *entry->unit)) != NULL)
						printf("%s", unit);
					printf("\n");
					// flush the stdout puffer, that pipes work without waiting
					fflush(stdout);
				}*/
			}
		
		}
	}
	
	sml_file_free(file);
	
	//*end_read = time(NULL);
	
	return bytes;
}
	
double get_value_of_entry(sml_list *entry){
	if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) || ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED)){
		double value = sml_value_to_double(entry->value);
		int scaler = (entry->scaler) ? *entry->scaler : 0;
		int prec = -scaler;
		if (prec < 0)prec = 0;
		return value * pow_int(10, scaler);
	}
	return 0;
}

double pow_int(int base, int exponent){
	double result = 1;
	if(exponent >= 0)for(;exponent; --exponent)result *= base;
	else for(;exponent; ++exponent)result /= base;
	return result;
}

int serial_port_open(const char* device) {
	int bits;
	struct termios config;
	memset(&config, 0, sizeof(config));

	if (!strcmp(device, "-"))
		return 0; // read stdin when "-" is given for the device

#ifdef O_NONBLOCK
	//int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	int fd = open(device, O_RDONLY | O_NOCTTY | O_NONBLOCK);
#else
	//int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	int fd = open(device, O_RDONLY | O_NOCTTY | O_NDELAY);
#endif
	if (fd < 0) {
		fprintf(stderr, "error: open(%s): %s\n", device, strerror(errno));
		return -1;
	}

	// set RTS
	ioctl(fd, TIOCMGET, &bits);
	bits |= TIOCM_RTS;
	ioctl(fd, TIOCMSET, &bits);

	tcgetattr(fd, &config);

	// set 8-N-1
	config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR
			| ICRNL | IXON);
	config.c_oflag &= ~OPOST;
	config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	config.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB);
	config.c_cflag |= CS8;

	// set speed to 9600 baud
	cfsetispeed(&config, B9600);
	cfsetospeed(&config, B9600);

	tcsetattr(fd, TCSANOW, &config);
	return fd;
}

int serial_port_close(int fd){
	return close(fd);
}