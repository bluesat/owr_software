/*
 * GPSLogger Node
 * Logs GPS input to KML
 * By Harry J.E Day for Bluesat OWR
 * Date: 31/05/2014
 */
#include "GpsGUI.h"
#include <fstream>

GPSGUI::GPSGUI(OwrGui * ogui) {
	ROS_INFO("initlising GPSLogger");
	gui = ogui;
	//a nodehandler is used to communiate with the rest of ros
	ros::NodeHandle n("~");
	
	battery = 5;
	signal = 5;
	tiltX = 30;
	tiltY = 30;
	ultrasonic = 0;
	
	// pass the function that is called when a message is recived
	gpsSub = n.subscribe("/gps/fix", 1000, &GPSGUI::receiveGpsMsg, this); // GPS related data
	batterySub = n.subscribe("/status/battery", 1000, &GPSGUI::receiveBatteryMsg, this); // Power left on the battery
	videoSub = n.subscribe("/camera/image_raw", 1000, &GPSGUI::receiveVideoMsg, this); // Frames of video from camera
}

void GPSGUI::spin() {
	ros::spin();
}

void GPSGUI::receiveGpsMsg(const sensor_msgs::NavSatFix::ConstPtr& msg) {
	assert(msg);
	
	//ROS_INFO("received a message");
	//ROS_INFO("long %lf, lat %lf, alt %lf", msg->longitude, msg->latitude, msg->altitude);
		
	//create a new node
	ListNode l = (ListNode)malloc(sizeof(vector2D));
	l->y = msg->latitude;
	l->x = msg->longitude;
	gui->updateConstants(battery, signal, ultrasonic, l, target, NULL);
}


void GPSGUI::receiveBatteryMsg(const bluesat_owr_protobuf::battery_ros::ConstPtr& msg) {
	assert(msg);
	
	//ROS_INFO("received a message");
	//ROS_INFO("voltage %f", msg->voltage);
	battery = msg->voltage;
	
	gui->updateConstants(battery, signal, ultrasonic, NULL, target, NULL);
}

void GPSGUI::receiveVideoMsg(const sensor_msgs::Image::ConstPtr& msg) {
	assert(msg);
	
	//ROS_INFO("received video frame");
	
	gui->updateConstants(battery, signal, ultrasonic, NULL, target, (unsigned char *)msg->data.data());
}
