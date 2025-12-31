/* Class Calculation_Term represents a calculation term of a formula
   which can be used to calc a target value for a managable device.
   Each instance is defined by a operator and 1 or 2 operands. Depending
   on the operator an operand can be a instance of Calculation_Term,
   a Meter instance or a constant value.
   
   Operator CONSTANT: calculation just returns int value given to constructor
   Operator NEGATION: lets operand a calculate and then returns its value * -1
   Operator ADDITION: let both operands calculate and returns the sum of their values
   Operator ADDITION: let both operands calculate and returns the sum of their values
   Operator SUBSTRACTION: let both operands calculate and returns the result of
		operand a substracted by the result of operand b
   Operator METER: fetches the current power of the meter given to constructor 
*/   
   
#ifndef _Calculation_Term_H___
#define _Calculation_Term_H___

#include <vector>
#include "Meter.hpp"

class Calculation_Term{
	
	enum CALCULATION_TYPE{
		CONSTANT,
		NEGATION,
		ADDITION,
		SUBSTRACTION,
		METER
	};
	
	private:
		//Calculation_Term();
		
		int constant;
		CALCULATION_TYPE type;
		Calculation_Term* operand_a;
		Calculation_Term* operand_b;
		Meter* meter;
		
		static bool is_numeric(std::string s); //TODO: besseren Ort finden
		static Meter* find_meter(std::string meter_id, std::vector<Meter*>* meters);

		/*static Calculation_Term* create_constant(int constant);
		static Calculation_Term* create_negation(Calculation_Term* operand_a);
		static Calculation_Term* create_addition(Calculation_Term* operand_a, Calculation_Term* operand_b);
		static Calculation_Term* create_substraction(Calculation_Term* operand_a, Calculation_Term* operand_b);
		static Calculation_Term* create_meter_read(Meter* meter);*/
	
	public:

		//parses a rule and returns a calculation term object; if rule contains any meters, parameter meters must contain a list with the meter refererred to:
		static Calculation_Term* create_by_string(const std::string rule, std::vector<Meter*>* meters = NULL);

		//calculates the value of the term and returns it:
		int get_result(); 
		
};

#endif