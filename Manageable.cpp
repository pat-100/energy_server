#include "Manageable.hpp"

bool Manageable::set_target_power(int new_target_power){
	//todo:mutex 
	//std::cerr << "testi";
	target_power = new_target_power;
	return true;
}

int Manageable::get_target_power(){
	//todo:mutex 
	return target_power;
}

int Manageable::get_power(){ //sinnvoll?
	return 0;
}