/*
 * Main class for pbuff relays
 * Author: Harry J.E Day for Bluesat OWR
 * Date: 13/12/14
 */
 

#include "bluesat_owr_protobuf/TiltRelay.h"
 
#include <iostream>
 
 
int main(int argc, char ** argv) {
    //required to make sure protobuf will work correctly
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    //init ros
    ros::init(argc, argv, "ROStoPBuffSignalRelay");
    
    TiltRelay relay(TOPIC);
    tilt.spin();
    
    return EXIT_SUCCESS;   
}

