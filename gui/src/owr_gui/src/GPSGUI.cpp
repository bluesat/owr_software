/*
 * GPSLogger Node
 * Logs GPS input to KML
 * By Harry J.E Day for Bluesat OWR
 * Date: 31/05/2014
 */
#include "GpsGUI.h"
#include <fstream>

GPSGUI::GPSGUI() {
    ROS_INFO("initlising GPSLogger");
    //a nodehandler is used to communiate with the rest of ros
    ros::NodeHandle n("~");

    //pass the function that is called when a message is recived
    gpsSub = n.subscribe("/gps/fix", 1000, &GPSGUI::reciveGpsMsg, this);
    list = NULL;
    end = NULL;
}

void GPSGUI::spin() {
  
  ros::spin();

}



void GPSGUI::reciveGpsMsg(const sensor_msgs::NavSatFix::ConstPtr& msg) {
    assert(msg);
    
    ROS_INFO("recived a message");
    ROS_INFO("long %lf, lat %lf, alt %lf", msg->longitude, msg->latitude, msg->altitude);
        
    //create a new node
    ListNode l = (ListNode) malloc(sizeof(vector2D));
    l->x = msg->longitude; 
    l->y = msg->latitude;
    l->next = NULL;
    
    if(end == NULL) {
        list = l;
        end = l;
    } else {
        end->next = l;
        end = l;
    }
}


