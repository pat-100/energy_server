#ifndef _Device_H___
#define _Device_H___

#include <string>

class Device{
	private:
		std::string id;
		std::string description;
	
	public:
		virtual void set_meta_fields(std::string id, std::string description);
		virtual void set_id(std::string id);
		virtual std::string get_description();
		virtual std::string get_id();
};

#endif