#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Bluetongue.h"
#include <ros/ros.h>

using namespace std;

#define MESSAGE_MAGIC 0x55AA
// All types are multiples of 16 bits, due to how XC16 optimises for memory access
// All structs are multiples of 32 bits, so this works on x86
struct toControlMsg {
    uint16_t magic;
    int16_t lSpeed;
    int16_t rSpeed;
    int16_t armRotate;
    int16_t armTop;
    int16_t armBottom;
} __attribute__((packed));

struct toNUCMsg {
    uint16_t magic;
    uint16_t vbat;
    GPSData gpsData;
    MagData magData;
    IMUData imuData;
} __attribute__((packed));

Bluetongue::Bluetongue(const char* port) {
	// Open serial port
	port_fd = open(port, O_RDWR | O_NOCTTY);
	if (port_fd == -1) {
		ROS_INFO("Error in open uart port");
        abort();
	} else {
		ROS_INFO("Opened uart port");
	}
    // Set up stuff for select so we can timeout on reads
    FD_ZERO(&uart_set); /* clear the set */
    FD_SET(port_fd, &uart_set); /* add our file descriptor to the set */
	timeout.tv_sec = 1;
    timeout.tv_usec = 0;

	// Set parameters
	struct termios tty;
	memset(&tty, 0, sizeof tty);

	// Error Handling 
	if (tcgetattr(Bluetongue::port_fd, &tty) != 0) {
		ROS_INFO("Error %d from tcgetattr: %s", errno, strerror(errno));
	}
	ROS_INFO("Setting up uart");

	// Set Baud Rate 
	cfsetospeed(&tty, (speed_t)B19200);
	cfsetispeed(&tty, (speed_t)B19200);

	// Setting other Port Stuff 
	tty.c_cflag &= ~PARENB; // Make 8n1
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;

	tty.c_cflag &= ~CRTSCTS; // no flow control
	tty.c_cc[VMIN] = 1; // read doesn't block
	tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout
	tty.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines
	ROS_INFO("About to make raw");

	// Make raw 
	cfmakeraw(&tty);

	// Flush Port, then applies attributes
	tcflush(port_fd, TCIFLUSH);
	if (tcsetattr(port_fd, TCSANOW, &tty) != 0) {
		ROS_INFO("Error %d from tcsetattr: %s" ,errno, strerror(errno));
        abort();
	}
		ROS_INFO("Finished initalizing bluetongue");
}

Bluetongue::~Bluetongue(void) {
	close(Bluetongue::port_fd);
}

void Bluetongue::comm(bool forBattery, void *message, int message_len, 
    void *resp, int resp_len) {
	ROS_INFO("Writing message: ");
	for (int i = 0; i < message_len; i++) {
		ROS_INFO("%d: %02x\n", i, *((char *) message + i));
	}
	int written = 0;
	do {
		written += write(port_fd, (int8_t*)message + written, message_len - written);
	} while (written < message_len);
	ROS_INFO("Written packet, expecting to read %d", resp_len);
	tcflush(port_fd, TCIOFLUSH); 
	int readCount = 0;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
	do {
        int rv = select(port_fd + 1, &uart_set, NULL, NULL, &timeout);
        if(rv == -1) {
            ROS_ERROR("select"); /* an error accured */
        } else if(rv == 0) {
            ROS_INFO("timeout"); /* a timeout occured */
            break;
        } else {
		  readCount += read(port_fd, (int8_t*)resp + readCount, resp_len - readCount);
		}
        ROS_INFO("reading... %d", readCount);
	} while (readCount < resp_len);
	ROS_INFO("Read packet");
}

struct status Bluetongue::update(double leftMotor, double rightMotor, int armTop, 
    int armBottom, double armRotate) {
	struct toControlMsg mesg;
	struct toNUCMsg resp;
	mesg.magic = MESSAGE_MAGIC;
	mesg.lSpeed = (leftMotor * 500) + 1500; // Scale to 16bit int
	mesg.rSpeed = (rightMotor * 500) + 1500;
    mesg.armRotate = (armRotate * 500) + 1500;
    mesg.armTop = armTop;
    mesg.armBottom = armBottom;
	ROS_INFO("Speeds %d %d", mesg.lSpeed, mesg.rSpeed);
	ROS_INFO("Writing %d bytes.", (int) sizeof(struct toControlMsg));
	ROS_INFO("Arm top %d bottom %d rotate %d", mesg.armTop, mesg.armBottom, mesg.armRotate);
	comm(false, &mesg, sizeof(struct toControlMsg), &resp, 
            sizeof(struct toNUCMsg));
    struct status stat;
	if (resp.magic != MESSAGE_MAGIC) {
		 ROS_INFO("Update Bluetongue had a error");
        stat.roverOk = false;    
	} else {
        stat.roverOk = true;
    }
    stat.batteryVoltage = resp.vbat;// / (1 << 15);
    stat.gpsData = resp.gpsData;
    stat.magData = resp.magData;
    return stat;
}
