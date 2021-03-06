/*
 * Converts joystick comands to velocity, etc
 * Author: Harry J.E Day for BlueSat OWR
 * Start: 7/02/15
 */
 
 #include "armConverter.h"
 #include <assert.h>
 #include <ros/ros.h>



int main(int argc, char ** argv) {
    ros::init(argc, argv, "owr_telop1");
    ClawControl ClawControl;
    ClawControl.run();
    
}

ClawControl::ClawControl() {

    //init button sates
    cam0Button = 0;
    cam1Button = 0;
    cam2Button = 0;
    cam3Button = 0;
    fd = fopen(TTY, "w");
    assert(fd != NULL);
    //subscribe to joy stick
    //TODO: at some point we will need to handle two joysticks
    joySubscriber = nh.subscribe<sensor_msgs::Joy>("arm_joy", 1, &ClawControl::joyCallback, this);
    clawState = STOP;
        
}

void ClawControl::run() {
    ros::Rate r(1000);
    while(ros::ok()) {
        sendMessage();
        ros::spinOnce();
        r.sleep();
    }
}

void ClawControl::sendMessage() {
    if(fd) {
        fprintf(fd,"%d %d\n", CLAW_SERVO,clawState);
        fprintf(fd,"%d %d\n", ROT_SERVO, rotState);
        float buffer;
        //fscanf(fd, "%f", &buffer);
        //ROS_INFO("%f", buffer);
        //fsync((int)fd);
        //fflush(fd);
    } else {
        ROS_INFO("unsucesfull");
    }    
    ROS_INFO("Claw:%d\tRot:%d",clawState, rotState);
}

void ClawControl::joyCallback(const sensor_msgs::Joy::ConstPtr& joy) {
    #define MAX_IN 1.5
    #define DIFF 0.25
    
    if(joy->buttons[BUTTON_LB]) {
        clawState = OPEN;
    } else if (joy->buttons[BUTTON_RB]) {
        clawState = CLOSE;
    } else {
        clawState = STOP;
    }

    if(joy->buttons[BUTTON_A]) {
        rotState = OPEN;
    } else if (joy->buttons[BUTTON_B]) {
        rotState = CLOSE;
    } else {
        rotState = STOP;
    }
    
    //float top = joy->axes[STICK_R_UD] ;//* 0.2;
    //float bottom = (-joy->axes[STICK_L_UD]) ;//* 0.2;
    
    //float leftDrive  = 1.0f;
    //float rightDrive = 1.0f;
    
 
    //topDrive = ((top / MAX_IN) * 500) + 1500.0  ;
    //bottomDrive = ((bottom / MAX_IN) * 500) + 1500.0  ;
    //sendMessage(topDrive,bottomDrive);    
    

   
   
    //TODO: camera on/off
    //check if the camera button states have changes
    //switchFeed(&cam0Button,joy->buttons[CAM_FEED_0],0);
    //TODO: camera rotation
    //TODO: take photo
    //TODO: map zoom in/out
}


