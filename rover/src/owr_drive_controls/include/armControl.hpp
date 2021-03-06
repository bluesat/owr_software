/*
 * Original Author: Elliott Smith
 * Editors:
 * Date Started: 2/6/18
 * Purpose: Convert arm joystick message into arm joint control
 * @copyright: This code is released under the MIT [GPL for embeded] License. Copyright BLUEsat UNSW, 2017, 2018
 */

#ifndef ARM_CONTROL_H
#define ARM_CONTROL_H


#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>

#define MAX_JOYSTICK_Y_AXIS 36787.0
#define DEADZONE 5000.0
#define ARM_STICK_UPPER_UD 1
#define ARM_STICK_LOWER_UD 4
#define DPAD_LR 6
#define A_BUTTON 0
#define B_BUTTON 1
#define LEFT_SHOULDER 4
#define RIGHT_SHOULDER 5

#define PI 3.14159265359

typedef struct _armJointVel {
    double armUpperActuator;
    double armLowerActuator;
    double armBaseRotate;
    double clawGrip;
    double clawTwist;
} armJointVel;

class ArmControl {
    public:
        armJointVel convertJoystickMessageToJoints(const sensor_msgs::Joy::ConstPtr& joy);

    private:
        double clawGripAngle = 0.0;
};
#endif //ARM_CONTROL_H
