#ifndef blob_H_INCLUDED
#define blob_H_INCLUDED

struct sample_loc {
	float angle_from_robot=0; //degrees for all angles
	float distance=0; //meters
	bool sample_not_found=0;
	bool sample_angle_conf=0;
	bool whiteSample=true;
};

void blob_main(sample_loc &s_loc);

#endif
