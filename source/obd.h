/** @file obd.h
 * 
 * @brief A module for interacting with a WIFI ELM327 module connected to a car.
 */ 

#ifndef OBD_H
#define OBD_H

// Socket conneted to an ELM327, returned by initOBD().
typedef int OBDSocket;

// connects to and initializes the ELM327 and WIFI, returns a socket or -1 if failure.
OBDSocket initConnection();


// initializes the ELM327 to the format i want
int initELM(OBDSocket socket_fd);

void testELM(OBDSocket socket_fd, char* buffer);

// buffer for storing input, static because ds stack is very short.
#define BUF_LEN 1024

// this buffer is used for constructing requests and recieving requests
static char OBD_BUF[BUF_LEN];

#endif