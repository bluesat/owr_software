#!/usr/bin/env python
"""
Python subscriber to guess the position of joints based on their mode

Original Authors: Sajid, Harry
Editors:
This code is released under the MIT License. Copyright BLUEsat UNSW, 2018
"""
import rospy

from std_msgs.msg import Float64, Int16
from sensor_msgs.msg import JointState

LOOP_RATE_HZ = 10.0

pub = None
sub = None

joint_delay = None
joint_name = None

def joint_callback(msg):
    """
    Handles the joint control command callback

    @param msg: the message
    """
    out = JointState()
    curr_time = rospy.get_rostime()
    out.header.stamp = curr_time + joint_delay
    out.name = [joint_name]
    out.position = [msg.data]
    pub.publish(out)

def listener():
    rospy.init_node('joint_guesser')
    global pub
    pub = rospy.Publisher("/joint_states", JointState, queue_size=0)

    global joint_name
    joint_name = rospy.get_param("~joint_name", None)


    joint_delay_ns = rospy.get_param("~/joint_guesser/joint_delay_ns", 0.0)
    global joint_delay
    joint_delay = rospy.Duration.from_sec(joint_delay_ns)

    # this needs to be last so we don't get callbacks before everything else is initialised
    global sub
    sub = rospy.Subscriber(
        '/input/command',
        Float64,
        joint_callback,
        queue_size=1
    )

    rospy.spin()

if __name__ == '__main__':
    listener()
