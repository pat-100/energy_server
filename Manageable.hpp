#ifndef _Manageable_H___
#define _Manageable_H___

#include "Device.hpp"

#include <iostream>

class Manageable:public Device{
	protected:
		int target_power;
	
	public:
		virtual bool set_target_power(int new_target_power); //set target power in watts
		virtual int get_target_power(); //returns target power in watts
		virtual int get_power(); //return actually used power (if supported by device)
		virtual int get_minimal_consumption()=0; //returns minimal consumption of a device, may change during runtime (i. e. every type 2 charger's minimal power is 1380W in 1 phase mode and 4140W when switch to 3 phase mode)
		virtual int get_maximal_consumption()=0; //returns maximal consumption of a device in watts
	
};

#endif