/** @file obd.h
 * 
 * @brief A module for interacting with a WIFI ELM327 module connected to a car.
 */ 

#ifndef OBD_H
#define OBD_H

// Socket conneted to an ELM327, returned by initOBD()
typedef int OBDSocket;

// connects to and initializes the ELM327, returns a socket.
OBDSocket initOBD();

// Gets the speed of the connected vehicle, returns -1 if failure
int getSpeed(OBDSocket s);

// Gets the RPM of the connected vehicle's engine, returns -1 if failure
int getRPM(OBDSocket s);

// Gets the coolant temperature, returns -50 on error
int getTemp(OBDSocket s);


#endif