#include "ros/ros.h"
#include "robot/BeaconRequest.h"
#include "robot/BeaconResponse.h"
#include "robot/SampleRequest.h"
#include "robot/SampleResponse.h"
#include "beacon.h"
#include "blob.h"
#include <signal.h>

//create beacon publisher
ros::Publisher becn_rsp;
//create beacon subscriber
ros::Subscriber becn_req;
//create sample publisher
ros::Publisher samp_rsp;
//create sample subscriber
ros::Subscriber samp_req;

void sigintHand(int sig) {
	ros::shutdown();
}

//handler for beacon request
void BeaconRequest_hand(const robot::BeaconRequest::ConstPtr& msg) {

	//look for the beacon in the requested area
	beacon_loc loc = beacon_main(msg->angle_min, msg->angle_max);
	/*loc.angle_from_robot = 10;
	loc.angle_from_beacon = 15;
	loc.distance = 10;
	loc.only_bottom = false;
	loc.beacon_not_found = false;
	loc.beacon_angle_conf = true;
	*/
	//report findings
	robot::BeaconResponse rsp;
	rsp.angle_from_robot = loc.angle_from_robot;
	rsp.distance = loc.distance;
	rsp.angle_from_beacon = loc.angle_from_beacon;
	rsp.only_bottom = loc.only_bottom;
	rsp.beacon_not_found = loc.beacon_not_found;
	rsp.beacon_angle_conf = loc.beacon_angle_conf;

	becn_rsp.publish(rsp);

}
 
//handler for sample request
void SampleRequest_hand(const robot::SampleRequest::ConstPtr& msg) {

	//look for the sample in the requested area
	sample_loc loc = blob_main(msg->angle_min, msg->angle_max);
	robot::SampleResponse rsp;
	rsp.angle_from_robot = loc.angle_from_robot;
	rsp.distance = loc.distance;
	rsp.sample_not_found = loc.sample_not_found;
	rsp.sample_angle_conf = loc.sample_angle_conf;

	samp_rsp.publish(rsp);

}

int main(int argc, char **argv) {
	//initialize ros with "vision" node
	ros::init(argc, argv, "vision");

	//create handle for node
	ros::NodeHandle n;

	//advertise response	
	becn_rsp = n.advertise<robot::BeaconResponse>("BeaconResponse", 1000);
	samp_rsp = n.advertise<robot::SampleResponse>("SampleResponse", 1000);

	//subscribe to beacon request
	becn_req = n.subscribe("BeaconRequest", 1000, BeaconRequest_hand);
	//subscribe to sample request
	samp_req = n.subscribe("SampleRequest", 1000, SampleRequest_hand);

	//set loop rate 
	ros::Rate loop_rate(10);
	ROS_INFO("Started"); //logs 

	//make SIGINT handler
	signal(SIGINT, sigintHand);
	//ok() will return true as long as the node is still running and Ctrl-C hasnt been pressed
	while (ros::ok()) { 
	   //SpinOnce processes any subscriber handlers waiting on the queue
	   ros::spinOnce();
	   loop_rate.sleep();
    }

}
