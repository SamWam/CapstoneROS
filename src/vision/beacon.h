#ifndef beacon_H_INCLUDED
#define beacon_H_INCLUDED


struct beacon_loc {
	float angle_from_robot; //degrees for all angles
	float distance; //meters
	float angle_from_beacon;
	bool only_bottom;
	bool beacon_not_found;
	bool beacon_angle_conf;
};

struct hsvParams{
  int hL, sL, vL, hH, sH, vH;
};

Mat getPic(VideoCapture cap);

beacon_loc beacon_main(float min, float max);

#endif
