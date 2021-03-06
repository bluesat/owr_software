# owr_positioning 
package for managing positioning of BLUEsat Rovers

## provides the following:

### launch files
- `laser.launch` launch file for the LIDAR used by the BLUEtongue Rover
- `lidar_sweep.launch` launches a node that sweeps the lidar gimbal on BLUEtongue
- `optical_ekf.launch` launches ROS's inbuilt ekf system 
- `optical_localization.launch` launches the optical flow node and associated cameras. **Requires the optical flow udev rule**
- `slam.tf` produces tf frames required for hector slam
- `cse_graphslam2d.launch` - graph slam with pure optical flow 
- `cse`

### nodes
- `optical_localization` - takes in four camera outputs and produces  

(and some other old stuff)

# Graph SLAM Setup

This uses the implementation of Graph SLAM provided by CSE UNSW at (http://robolab.cse.unsw.edu.au:4443/rescue/crosbot/wikis/package/crosbot_graphslam)
As well as our own optical flow code for the tracking frame.

## frames

| Frame           | Provided by                  | Used By         | Purpose |
|-----------------|------------------------------|-----------------|---------|
| /slam           | Graph SLAM                   | RViz/GUI        | Mapping |
| /base_footprint | Robot State Publisher (URDF) | graph_slam, ekf | Base of the Robot in the URDF model. Used to define relative position of sensors |
| /odom           | EKF                          | graph_slam      | tracking frame used for motion of the rover |
| /icp (optional) | crosbot_ogmbicp              | graph_slam      | Can be used instead of odom add laser tracking, may be better |


## Using the Package

### Running SLAM

In tmuxprocon run: graphslam2d_ogmbicp for the easy way

NOTE: this assumes a sweeping lidar. If you don't want the lidar to sweep then you need to publish the lidar's position control topic with a value of 0.0 to ensure
the transforms are correct.

Uses the frame at the top of the transform tree (should be above `odom`)

### Running just optical flow

In tmuxprocom run: localisation_base.

This will start the basic steering systems + optical flow and its ekf.

The rover will move relative to the `odom` frame. If you are watching in rviz switch to that.

### Calibrating the optical flow

To calibrate the optical flow run the `optical_localization.launch` and `optical_ekf.launch` launch files.

Drive the rover forward along a ruler and look at the position value published by the ekf on `/odometry/filtered` compared to the one on the ruler. Adjust the pixel/m ratio in `src/owr_positioning/include/optical_localization.h` to match.


