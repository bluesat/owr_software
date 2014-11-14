/*
 * Main class for pbuff relays
 * Author: Harry J.E Day for Bluesat OWR
 * Date: 14/12/14
 */
 
#include "bluesat_owr_protobuf/PBuffRelay.h"
#include "message1.pb.h"
#include "bluesat_owr_protobuf/message1_ros.h"
 
#define MESSAGE_CLASS bluesat_owr_protobuf_proto::message1
#define MESSAGE_CLASS_ROS bluesat_owr_protobuf::message1_ros
#define TOPIC "/owr_protobuf/message1"
 
 
int main(int argc, char ** argv) {
    //required to make sure protobuf will work correctly
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    //init ros
    ros::init(argc, argv, "PBuffRelay");
    
    PBuffRelay<MESSAGE_CLASS_ROS,MESSAGE_CLASS> relay(TOPIC);
    
    relay.spin();
    
    return EXIT_SUCCESS;   
 }
