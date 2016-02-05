#include "ros/ros.h"
#include "robot/Commands.h"
#include <cstdlib> 
#include <iostream>
#include <string>
#include <cmath>

// Flags
int state = 0; // 0: Startup, 1: Target seeking, 2: Returning, 3: BeaconApproach, 4: Paused 
double last_angle_to_beacon = 180;
double last_angle_to_robot = 0;
double last_distance_to_beacon = 12;
bool only_bottom_light = true;
bool beacon_found = true;
bool beacon_angle_conf = true;
bool waiting_on_command = false;
bool waiting_on_vision = false;
int command_timeout = 100; //TODO: Add command timeout

// Coordinates
int x_pos = 0;
int y_pos = 1;

void beaconCallback(const robot::BeaconResponse::ConstPtr& msg) {
  if(msg->beacon_not_found) {
    beacon_found = false;
    beacon_angle_conf = false;
  } else {
    last_angle_to_beacon = msg->angle_from_beacon;
    last_angle_to_robot = msg->angle_to_robot;
    last_distance_to_beacon = msg->distance;
    only_bottom_light = msg->only_bottom;
    beacon_found = true;
    beacon_angle_conf = msg->beacon_angle_conf; 
    if(beacon_angle_conf) {
      y_pos = last_distance_to_beacon * cos(last_angle_to_beacon * PI/180); //Vision sends deg
      x_pos = last_distance_to_beacon * sin(last_angle_to_beacon * PI/180);
    }
  }
  waiting_on_vision = false;
}

void setState(const std_msgs::int32::ConstPtr& msg) {
  state = msg->data;
}

void comandDone(const std_msgs::int32::ConstPtr& msg) {
  waiting_on_command = false;
}
  
/**
 * Early prototype of the robot navigation
 */
int main(int argc, char **argv) {
  
  // Initialize ROS, this must be done before using other ROS components
  ros::init(argc, argv, "navigation");

  // Main access point to ROS. Makes a ROS node, which will shut down when this is destructed
  ros::NodeHandle n;

  // Publish a topic "BeaconRequest", used to ask for a scan for the Beacon
  ros::Publisher beacon_request_pub = n.advertise<robot::BeaconRequest>("BeaconRequest", 1000);

  // Publish a topic "BeaconRequest", used to ask for a scan for the Beacon
  ros::Publisher command_pub = n.advertise<robot::Commands>("Commands", 1000);
  
  // Subscribe to the "BeaconResponse" return from a beacon scan
  ros::Subscriber beacon_response_msg = n.subscribe("BeaconResponse", 1000, beaconCallback);
  
  // Subscribe to the "SetState" return from a beacon scan
  ros::Subscriber set_state_msg = n.subscribe("SetState", 1000, setState);
  ros::Subscriber command_done_msg = n.subscribe("CommandDone", 1000, commandDone);

  // Defines a maximum loop rate (10 Hz)
  ros::Rate loop_rate(10); // This should be fast enough for us, since at 2 m/s this would be .2 meters an update at worst.

  // Make a beacon request
  robot::BeaconRequest b_msg; // Defined in msg directory
  robot::Commands c_msg;
  
  while (ros::ok()) {
    

    switch(state) {
    case 0: // Startup
      break;
    case 1: // Target Seeking
      break;
    case 2: // Returning
      b_msg.angle_min = -150;
      b_msg.angle_max = 150;
      beacon_request_pub.publish(b_msg); // Look for the beacon
      waiting_on_vision = true;
      while(waiting_on_vision) {
	loop_rate.sleep(); // TODO: Add a timeout here
      }
      if(beacon_found) {
	double angle = last_angle_to_beacon + (90 - last_angle_to_robot - (180/PI)*atan((y_pos - 5)/x_pos));
	double dist = sqrt(x_pos*x_pos + (y_pos-5)*(y_pos-5));
	if(dist < 30) {
	  state = 3;
	  break;
	}
	if(abs(angle)<10) {
	  c_msg.commandOrder = 1; // Driving
	  c_msg.value = dist;
	  command_pub.publish(c_msg);
	  waiting_on_command = true;
	  while(waiting_on_command){
	    loop_rate.sleep(); // TODO: Add a timeout here
	  }
	} else {
	  c_msg.commandOrder = 2; // Turning
	  c_msg.value = angle;
	  command_pub.publish(c_msg);
	  waiting_on_command = true;
	  while(waiting_on_command){
	    loop_rate.sleep(); // TODO: Add a timeout here
	  }
	}
      } else {
	c_msg.commandOrder = 2; // Turning
	c_msg.value = 180;
	command_pub.publish(c_msg);
	waiting_on_command = true;
	while(waiting_on_command){
	  loop_rate.sleep(); // TODO: Add a timeout here
	}
	
      }
      break;
    case 3: // BeaconApproach
      b_msg.angle_min = -150;
      b_msg.angle_max = 150;
      beacon_request_pub.publish(b_msg); // Look for the beacon
      waiting_on_vision = true;
      while(waiting_on_vision) {
	loop_rate.sleep(); // TODO: Add a timeout here
      }
      if(beacon_found) {
	double angle = last_angle_to_beacon;
	if(abs(angle) > 10) {
	  c_msg.commandOrder = 2; // Turning
	  c_msg.value = angle;
	  command_pub.publish(c_msg);
	  waiting_on_command = true;
	  while(waiting_on_command){
	    loop_rate.sleep(); // TODO: Add a timeout here
	  }	  
	} else {
	  c_msg.commandOrder = 1; // Drive up the ramp
	  c_msg.value = 200;
	  command_pub.publish(c_msg);
	  waiting_on_command = true;
	  while(waiting_on_command){
	    loop_rate.sleep(); // TODO: Add a timeout here
	  }
	}
      } else {
	c_msg.commandOrder = 2; // Turning
	c_msg.value = 180;
	command_pub.publish(c_msg);
	waiting_on_command = true;
	while(waiting_on_command){
	  loop_rate.sleep(); // TODO: Add a timeout here
	}	
      }
      break;
    case 4: // Paused
      break;
    } 
          
    ros::spinOnce(); // Checks for ros update
    loop_rate.sleep(); // Sleep for the period corresponding to the given frequency
  }
  
  return 0;
}
