#ifndef _SML_METER_H___
#define _SML_METER_H___

#include <stdio.h>
#include <sml/sml_file.h>
#include <sml/sml_transport.h>
#include <sml/sml_value.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include "Meter.hpp"
#include "logging.hpp"
#include <string.h>
#include <pthread.h>

double pow_int(int base, int exponent);
double get_value_of_entry(sml_list *entry);
class Meter_Data get_sml_meter_data(int fd);
size_t read_sml_values(int fd, std::string* serial, int* reg_1_0_1_8_0,int* reg_1_0_2_8_0, int* reg_1_0_16_7_0);
int serial_port_open(const char* device);
int serial_port_close(int fd);

class SML_Meter:public Meter{
	private:
	pthread_t thread;
	pthread_mutex_t data_lock;
	
	std::string device;
	Meter_Data meter_data;
	
	static void* handle_thread(void* object);
	void* handle_thread();
	
	public:
	SML_Meter(std::string device_path);
	~SML_Meter();
	
	time_t get_last_time_read() override;
	std::string get_serial() override;
	int get_register_grid() override;
	int get_register_generation() override;
	int get_current_power() override;
	Meter_Data get_meter_data() override;
};

#endif
