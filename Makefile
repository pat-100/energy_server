UNAME := $(shell uname)

LIBSML = ./libsml-master/sml/lib/libsml.a

FLAGS = -I /usr/local/include/modbus
#FLAGS= -I /usr/local/lib
LDIR = /usr/local/lib
LIBMODBUS = -lmodbus

#ifneq ($(UNAME), Darwin)
LIBS = -luuid
#endif

all: energy_server

clean:
	@rm -rf *.o
	@rm -rf server

energy_server: main.o httpd.o Meter.o SML_Meter.o Modbus_Connection.o Modbus_Connection_RTU.o Modbus_Connection_TCP.o Virtual_Fronius_Meter.o BnE_Meter.o Config.o logging.o Calculation_Term.o Manageable.o Keba_P30.o string_ext.o Device.o Energy_Manager.o Virtual_ALE3_Meter.o Fronius_Gen24_Meter.o $(UNITY) $(OBJS) $(LIBSML) $(LIBMODBUS) $(LIBS)
	g++ -o energy_server $^ -lpthread

main.o: main.cpp main.hpp
	g++ -c -o main.o main.cpp

httpd.o: httpd.cpp httpd.hpp
	g++ -c -o httpd.o httpd.cpp
	
Meter.o: Meter.cpp Meter.hpp
	g++ -c -o Meter.o Meter.cpp
	
SML_Meter.o: SML_Meter.cpp SML_Meter.hpp
	g++ -c -o SML_Meter.o SML_Meter.cpp	

Modbus_Connection.o: Modbus_Connection.cpp Modbus_Connection.hpp
	g++ -c -o Modbus_Connection.o Modbus_Connection.cpp

Modbus_Connection_RTU.o: Modbus_Connection_RTU.cpp Modbus_Connection_RTU.hpp
	g++ -c -o Modbus_Connection_RTU.o Modbus_Connection_RTU.cpp

Modbus_Connection_TCP.o: Modbus_Connection_TCP.cpp Modbus_Connection_TCP.hpp
	g++ -c -o Modbus_Connection_TCP.o Modbus_Connection_TCP.cpp

Virtual_Fronius_Meter.o: Virtual_Fronius_Meter.cpp Virtual_Fronius_Meter.hpp
	g++ -c -o Virtual_Fronius_Meter.o Virtual_Fronius_Meter.cpp

BnE_Meter.o: BnE_Meter.cpp BnE_Meter.hpp
	g++ -c -o BnE_Meter.o BnE_Meter.cpp
	
Config.o: Config.cpp Config.hpp
	g++ -c -o Config.o Config.cpp
	
string_ext.o: string_ext.cpp string_ext.hpp
	g++ -c -o string_ext.o string_ext.cpp
	
logging.o: logging.cpp logging.hpp
	g++ -c -o logging.o logging.cpp
	
Keba_P30.o: Keba_P30.cpp $(LIBMODBUS)
	g++ -c -o Keba_P30.o Keba_P30.cpp

Device.o: Device.cpp Device.hpp
	g++ -c -o Device.o Device.cpp

Manageable.o: Manageable.cpp Manageable.hpp
	g++ -c -o Manageable.o Manageable.cpp

Energy_Manager.o: Energy_Manager.cpp Energy_Manager.hpp
	g++ -c -o Energy_Manager.o Energy_Manager.cpp

Virtual_ALE3_Meter.o: Virtual_ALE3_Meter.cpp Virtual_ALE3_Meter.hpp
	g++ -c -o Virtual_ALE3_Meter.o Virtual_ALE3_Meter.cpp

Calculation_Term.o: Calculation_Term.cpp Calculation_Term.hpp
	g++ -c -o Calculation_Term.o Calculation_Term.cpp

Fronius_Gen24_Meter.o: Fronius_Gen24_Meter.cpp Fronius_Gen24_Meter.hpp
	g++ -c -o Fronius_Gen24_Meter.o Fronius_Gen24_Meter.cpp

libsml : 
	@$(MAKE) -C ../libsml-master/sml

libmodbus:
	@$(MAKE) -C ../libmodbus-master
