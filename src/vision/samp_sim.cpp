#include "ros/ros.h"
#include "robot/SampleResponse.h"
#include "robot/SampleRequest.h"
#include <iostream>
#include <string>

bool waiting_on_vision = false;
void sampleCallback(const robot::SampleResponse::ConstPtr& msg){
	ROS_INFO("Response Recieved:\nSample Found: %s\nDistance: %fm\nAngle from robot: %f degrees\nSample Angle Confidence: %s", 
		msg->sample_not_found? "false":"true", 
		msg->distance,
		msg->angle_from_robot,
		msg->sample_angle_conf? "true":"false");
	waiting_on_vision = false;
}

int main(int argc, char **argv) {
	ROS_INFO("Initializing...");
	
	//Initialize ROS
	ros::init(argc, argv, "samp_sim");
	
	// Main access point to ROS. Makes a ROS node, which will shut down when this is destructed
  	ros::NodeHandle n;
	
  	// Publish a topic "SampleRequest", used to ask for a scan for the Sample
  	ros::Publisher sample_request_pub = n.advertise<robot::SampleRequest>("SampleRequest", 1000);
  	
  	// Subscribe to the "SampleResponse" return from a sample scan
  	ros::Subscriber sample_response_msg = n.subscribe("SampleResponse", 1000, sampleCallback);
  	
  	// Defines a maximum loop rate (10 Hz)
  	ros::Rate loop_rate(10); // This should be fast enough for us, since at 2 m/s this would be .2 meters an update at worst.
	
  	// Make a sample request
  	robot::SampleRequest s_msg; // Defined in msg directory
	ROS_INFO("Initializing Complete");
  
  	while (ros::ok()) {
		s_msg.angle_min = -150;
      		s_msg.angle_max = 150;
		s_msg.whiteSample=false;
		//s_msg.whiteSample=false;
      		while(sample_request_pub.getNumSubscribers()<1){ //Wait for vision to subcribe to us
			loop_rate.sleep();
		}
		sample_request_pub.publish(s_msg);
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
