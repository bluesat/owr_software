/*
	Navigation Node
	Handles updates to the Navigation GUI
	By Harry J.E Day for Bluesat OWR
	Date: 31/05/2014
	
	
	Updated 30/5/15 by Simon Ireland to detect and pass to gui the active/inactive/offline cameras
*/

#include "NavigationNode.h"
#include <fstream>

// Include for the image_trasport pkg which will allow us to use compressed
// images through magic ros stuff :)
#include <image_transport/image_transport.h>

NavigationNode::NavigationNode(NavigationGUI *newgui) {
	ROS_INFO("Starting Navigation Node");
	gui = newgui;
	//a nodehandler is used to communiate with the rest of ros
	ros::NodeHandle n("~");
    image_transport::ImageTransport imgTrans(n);
	
	//Initialise all the information to be used in by the gui
	battery = 6;
	signal = 6;
	tiltX = 30;
	tiltY = 30;
	ultrasonic = 0;
	lidar = 0;
	
	//Initialise the feeds array
	for(int i = 0; i < TOTAL_FEEDS; i++)
		feeds[i] = FEED_OFFLINE;
	
	// 
	// Subscribe to all relevant topics for information used by the gui
	// pass the function that is called when a message is received into the subscribe function
	// 
	
	gpsSub = n.subscribe("/gps/fix", 1000, &NavigationNode::receiveGpsMsg, this); // GPS related data
	batterySub = n.subscribe("/status/battery", 1000, &NavigationNode::receiveBatteryMsg, this); // Power left on the battery
	feedsSub = n.subscribe("/owr/control/availableFeeds", 1000, &NavigationNode::receiveFeedsStatus, this);
	voltSub = n.subscribe("/owr/voltmeter", 1000, &NavigationNode::receiveVoltageMsg, this);
	adcSub = n.subscribe("/owr/adc", 1000, &NavigationNode::receiveClawMsg, this);
	lidarModeSub = n.subscribe("/owr/lidar_gimble_mode", 1000, &NavigationNode::receiveLidarMsg, this);
	
	// Subscribe to all topics that will be published to by cameras, if the topic hasnt been
	// created yet, will wait til it has w/o doing anything

	//ros::TransportHints transportHints = ros::TransportHints().tcpNoDelay();
	//image_transport::TransportHints transportHints = ros::TransportHints().tcpNoDelay();

        // Frames of video from camera
	videoSub[0] = imgTrans.subscribe("/cam0", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));
	videoSub[1] = imgTrans.subscribe("/cam1", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));
	videoSub[2] = imgTrans.subscribe("/cam2", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));
	videoSub[3] = imgTrans.subscribe("/cam3", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));
	videoSub[4] = imgTrans.subscribe("/cam4", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));
	videoSub[5] = imgTrans.subscribe("/cam5", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));
	videoSub[6] = imgTrans.subscribe("/cam6", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));
	videoSub[7] = imgTrans.subscribe("/cam7", 1, &NavigationNode::receiveVideoMsg, this, image_transport::TransportHints("compressed"));

}

// Spin to wait until a message is received
void NavigationNode::spin() {
	ros::spin();
}

// Called when a message from availableFeeds topic appears, updates with a list of connected cameras:
// FEED_OFFLINE means camera not conencted
// FEED_ACTIVE means its currently streaming
// FEED_INACTIVE means connected but not streaming.
// See gui/src/owr_messages/msg/activeCameras.msg and stream.msg to understand the input message
//
// Simon Ireland: 30/5/15

void NavigationNode::receiveFeedsStatus(const owr_messages::activeCameras::ConstPtr &msg) {
	assert(msg);
	
	//ROS_INFO("finding active feeds");
	
	// Reset feeds array
	for(int i = 0; i < TOTAL_FEEDS; i++)
		feeds[i] = FEED_OFFLINE;
	
	// There isnt gaurenteed to be 4 streams in msg, so only update the ones that do appear
	for(int i = 0; i < msg->num; i++) {
		// Get the actual camera number from msg
		int feed = msg->cameras[i].stream;
		
		// If on, then it is streaming, oterwise its only connected 
		if(msg->cameras[i].on)
			feeds[feed] = FEED_ACTIVE;
		else
			feeds[feed] = FEED_INACTIVE;
	}
	
	// Update the gui
	gui->updateFeedsStatus(feeds, msg->num);
}

void NavigationNode::receiveGpsMsg(const sensor_msgs::NavSatFix::ConstPtr& msg) {
	assert(msg);
	
	//ROS_INFO("received a message");
	//ROS_INFO("long %lf, lat %lf, alt %lf", msg->longitude, msg->latitude, msg->altitude);
	
	//create a new node
	ListNode l = (ListNode)malloc(sizeof(vector3D));
	l->lat = msg->latitude;
	l->lon = msg->longitude;
	l->alt = msg->altitude;
	gui->updateInfo(battery, signal, ultrasonic, l, target, lidar);
}


void NavigationNode::receiveBatteryMsg(const owr_messages::status::ConstPtr& msg) {
	assert(msg);
	
	//ROS_INFO("received a message");
	//ROS_INFO("voltage %f", msg->voltage);
	signal = msg->signal;
	battery = msg->battery;
	gui->updateInfo(battery, signal, ultrasonic, NULL, target, lidar);
}

void NavigationNode::receiveVideoMsg(const sensor_msgs::Image::ConstPtr& msg) {
	assert(msg);
	
	//ROS_INFO("received video frame");
	
	gui->updateVideo((unsigned char *)msg->data.data(), msg->width, msg->height);
}

void NavigationNode::receiveVoltageMsg(const std_msgs::Float32::ConstPtr& msg) {
	assert(msg); // 
	
	//ROS_INFO("received a message");
	//ROS_INFO("voltage %f", msg->voltage);
	voltage = msg->data;
	gui->updateVoltage(voltage);
    ROS_INFO("voltage %f", voltage);
}

void NavigationNode::receiveClawMsg(const owr_messages::adc::ConstPtr& msg) {

	assert(msg); 
	pot_vals = msg->pot;
	pot_names = msg->potFrame;

	int effort_index;
	int target_index;
	int actual_index;

	effort_index = find(pot_names.begin(), pot_names.end(), "clawEffort") - pot_names.begin();
	target_index = find(pot_names.begin(), pot_names.end(), "clawGrip") - pot_names.begin();
	actual_index = find(pot_names.begin(), pot_names.end(), "clawActual") - pot_names.begin();

	float effort = pot_vals[effort_index];
	float actual = pot_vals[actual_index];
	float target = pot_vals[target_index];

	ROS_INFO("EFFORT: %f", effort);
	ROS_INFO("ACTUAL: %f", actual);
	ROS_INFO("TARGET: %f", target);

	ROS_INFO("EFFORT INDEX: %d", effort_index);
	ROS_INFO("ACTUAL INDEX: %d", actual_index);
	ROS_INFO("TARGET INDEX: %d", target_index);

	gui->updateADC(effort, actual, target);

}

void NavigationNode::receiveLidarMsg(const std_msgs::Int16::ConstPtr& msg) {
	assert(msg);
	
	//ROS_INFO("received video frame");
	lidar = msg->data;
	gui->updateInfo(battery, signal, ultrasonic, NULL, target, lidar);
}
