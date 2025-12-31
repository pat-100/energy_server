#include "Calculation_Term.hpp"

/*Calculation_Term* Calculation_Term::create_constant(int constant){
	Calculation_Term* result = new Calculation_Term();
	result->type = CONSTANT;
	result->constant = constant;
	return result;
}

Calculation_Term* Calculation_Term::create_negation(Calculation_Term* operand_a){
	Calculation_Term* result = new Calculation_Term();
	result->type = NEGATION;
	result->operand_a = operand_a;
	return result;
}

Calculation_Term* Calculation_Term::create_addition(Calculation_Term* operand_a, Calculation_Term* operand_b){
	Calculation_Term* result = new Calculation_Term();
	result->type = ADDITION;
	result->operand_a = operand_a;
	result->operand_b = operand_b;
	return result;
}

Calculation_Term* Calculation_Term::create_substraction(Calculation_Term* operand_a, Calculation_Term* operand_b){
	Calculation_Term* result = new Calculation_Term();
	result->type = SUBSTRACTION;
	result->operand_a = operand_a;
	result->operand_b = operand_b;
	return result;
}

Calculation_Term* Calculation_Term::create_meter_read(Meter* meter){
	Calculation_Term* result = new Calculation_Term();
	result->type = METER;
	result->meter = meter;
	return result;
}*/

Calculation_Term* Calculation_Term::create_by_string(const std::string rule, std::vector<Meter*>* meters){
	
	Calculation_Term* result = new Calculation_Term;
	
	result->operand_a = NULL;
	result->operand_b = NULL;
	
	int bracket_level = 0;
	int bracket_start;
	
	for(int i=0; i < rule.length(); ++i){
	
			switch(rule[i]){
				case '(': 
					++bracket_level;
					if(bracket_level==1)bracket_start = i+1;
					break;
				case ')':
					--bracket_level;
					if(bracket_level<0)return NULL; //syntax error
					if(bracket_level==0)result->operand_a = create_by_string(rule.substr(bracket_start, i - bracket_start), meters);
					break;
			}
		
		if(bracket_level==0){ //if not within a pair of brackets
			switch(rule[i]){
				case '+':
					result->type = ADDITION;
					if(result->operand_a==NULL)result->operand_a = create_by_string(rule.substr(0,i), meters);
					result->operand_b = create_by_string(rule.substr(i+1), meters);
					return result;
				case '-':
					result->type = SUBSTRACTION;
					if(result->operand_a==NULL)result->operand_a = create_by_string(rule.substr(0,i), meters);
					result->operand_b = create_by_string(rule.substr(i+1), meters);
					return result;
			}
		}
	}	

	if(!(result->operand_a)){
		if(rule==""){
			delete result;
			return NULL;
		}
		if(is_numeric(rule)){
			result->type = CONSTANT;
			result->constant = atoi(rule.c_str());
			return result;
		}
		else{
			result->type = METER;
			result->meter = find_meter(rule, meters);
			return result;
		}
	}
	
	delete result;
	return NULL;

}

bool Calculation_Term::is_numeric(std::string s){
	for(char c:s)if(c<'0' || c>'9')return false;
	return true;
}

Meter* Calculation_Term::find_meter(std::string meter_id, std::vector<Meter*>* meters){
	for(Meter* meter:(*meters))if(meter->get_id() == meter_id)return meter;
	return NULL;
}

int Calculation_Term::get_result(){
	
	switch(type){
		case CONSTANT:
			return constant;
		case NEGATION:
			return - ( operand_a->get_result() );
		case ADDITION:
			return operand_a->get_result() + operand_b->get_result();
		case SUBSTRACTION:
			return operand_a->get_result() - operand_b->get_result();
		case METER:
			return meter->get_current_power();
	}
	return 0;
	
}

/*Calculation_Term::Calculation_Term(){
}*/