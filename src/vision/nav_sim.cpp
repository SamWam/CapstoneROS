#include "ros/ros.h"
#include "robot/BeaconResponse.h"
#include "robot/BeaconRequest.h"
#include <iostream>
#include <string>

bool waiting_on_vision = false;
void beaconCallback(const robot::BeaconResponse::ConstPtr& msg){
	ROS_INFO("Response Recieved:\n
		Beacon Found: %s\n
		Distance: %fm\n
		Angle from beacon: %f degrees\n
		Angle from robot: %f degrees\n
		Only bottom: %s\n
		Beacon Angle Confidence: %s",
		msg->beacon_not_found? "false":"true",
		msg->distance,
		msg->angle_from_beacon,
		msg->angle_from_robot
		msg->only_bottom,
		msg->beacon_angle_conf);
	waiting_on_vision = false;
}

int main(int argc, char **argv) {
	ROS_INFO("Initializing...");
	
	//Initialize ROS
	ros::init(arc, argv, "nav_sim");
	
	// Main access point to ROS. Makes a ROS node, which will shut down when this is destructed
  	ros::NodeHandle n;
	
  	// Publish a topic "BeaconRequest", used to ask for a scan for the Beacon
  	ros::Publisher beacon_request_pub = n.advertise<robot::BeaconRequest>("BeaconRequest", 1000);
	
  	// Publish a topic "BeaconRequest", used to ask for a scan for the Beacon
  	ros::Publisher command_pub = n.advertise<robot::Commands>("Commands", 1000);
  	
  	// Subscribe to the "BeaconResponse" return from a beacon scan
  	ros::Subscriber beacon_response_msg = n.subscribe("BeaconResponse", 1000, beaconCallback);
  	
  	// Defines a maximum loop rate (10 Hz)
  	ros::Rate loop_rate(10); // This should be fast enough for us, since at 2 m/s this would be .2 meters an update at worst.
	
  	// Make a beacon request
  	robot::BeaconRequest b_msg; // Defined in msg directory
	ROS_INFO("Initializing Complete");
  
  	while (ros::ok()) {
		b_msg.angle_min = -150;
      		b_msg.angle_max = 150;
      		while(beacon_request_pub.getNumSubscribers()<1){ //Wait for vision to subcribe to us
			loop_rate.sleep();
		}
		beacon_request_pub.publish(b_msg);
		waiting_on_vision = true;
		ROS_INFO("Request Published");
		while(waiting_on_vision) {
			loop_rate.sleep();
			ros::spinOnce();
      		}
		ROS_INFO("Press enter to make another request");
		std::string str;
		std::cin>>str;
	}
}