/*
 * Orignal Author: Harry J.E Day
 * Editors:
 * Date Started: 29/06/2016
 * Purpose: Class to convert pot values to a position 
 */

#include "PotPositionTracker.hpp"
#include <cmath>
#include <limits>
#include <ros/ros.h>

#define TURN_SAFTEY_MARGIN 1.0

/*PotPositionTracker::PotPositionTracker(int maxV, int minV, int turns) : maxValue(maxV), minValue(minV), turns(turns) {
    center = (maxV + minV)/2;
}*/

PotPositionTracker::PotPositionTracker(float negLimit, float posLimit, float singleRotationRange, int turns, float center) :
    negLimit(negLimit), posLimit(posLimit), singleRotationRange(singleRotationRange), 
    turns(turns), center(center) {

}


void PotPositionTracker::updatePos(double potValue) {
    updatePos(potValue,ros::Time::now());
}

void PotPositionTracker::updatePos(double potValue, ros::Time current) {
    //put the pot value in the range -PI*turns to PI*turns
    //TODO: check that this works when the center value is not the center, I'm pretty sure it does
    double pos = ((potValue - center) / (singleRotationRange*turns)) * (2.0 * M_PI * turns);
    ROS_INFO("POT VALUE: %f", potValue); 
    //check we haven't overshot
    if( potValue < negLimit) {
        ROS_ERROR("Too many turns! Too negative. Position is %f %% 2PI, pot value is %f", pos, potValue);
        pos = -1.0* std::numeric_limits<double>::infinity();
    } else if(potValue > posLimit) {
        ROS_ERROR("Too many turns! Too positive. Position is %f %% 2PI, pot value is %f", pos, potValue);
        pos = std::numeric_limits<double>::infinity();
    }
    
    
    //make sure the pos is in our 'nice' range (between M_PI and -M_PI)
    //second statement is for when we have an invalid pot value
    /*while (pos > M_PI && !std::isinf(pos)) {
        pos = -(2*M_PI - pos);
    } 
    while (pos < -M_PI && !std::isinf(pos)) {
        pos = 2*M_PI + pos;
    }*/
    //TODO: calc velocity
    
    updateLists(pos, 0.0, current,true);
    
}


void PotPositionTracker::resetPos() {
    mutext.lock();
    if(positions.size() > 0) {
        //TODO: account for velocity
        center = positions.back();
    } else {
        center = (negLimit + posLimit) / 2.0;
    }
    mutext.unlock();
    PositionTracker::resetPos();
}

double PotPositionTracker::getMaxAngle() {
    return ((posLimit - center) / (singleRotationRange*turns)) * (2.0 * M_PI * turns);
}

double PotPositionTracker::getMinAngle() {
   return ((negLimit - center) / (singleRotationRange*turns)) * (2.0 * M_PI * turns);
}

