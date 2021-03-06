#include "functions.h"

//takes a picture and returns that picture with the largest blob drawn
Mat findBiggestBlob(Mat src)
{
    int largest_area = 0;
    int largest_contour_index = 0;

    Mat temp(src.rows,src.cols,CV_8UC1);
    Mat dst(src.rows,src.cols,CV_8UC1,Scalar::all(0));
    src.copyTo(temp);

    vector<vector<Point> > contours; // storing contour
    vector<Vec4i> hierarchy;

    findContours( temp, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

    for( int i = 0; i < contours.size(); i++ ) // iterate
    {
        double a = contourArea( contours[i],false);  //Find the largest area of contour

        if(a > largest_area)
        {
            largest_area = a;
            largest_contour_index = i;
        }
    }

    drawContours(dst, contours,largest_contour_index, Scalar(255,0,0), CV_FILLED, 8, hierarchy);
    // Draw the largest contour
    return dst;
}

//finds the continuous region of grass in a picture, along with everything inside of it
void findGrass(Mat src, Mat HSV) //this should be separated into a few more readable functions
{
    int iLowH = 30;
    int iHighH = 70;

    int iLowS = 60;
    int iHighS = 255;

    int iLowV = 0;
    int iHighV = 255;

    Mat imgThresholded;
    Mat temp;
    src.copyTo(temp);

    inRange(HSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
    //morphological closing (fill small holes in the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    imgThresholded = findBiggestBlob(imgThresholded);

    for(int i = 0; i < imgThresholded.rows - 1; i++)
    {
        for(int j = 0; j < imgThresholded.cols - 1; j++)
        {
            if(imgThresholded.at<uchar>(i,j) == 0)
            {
                temp.at<Vec3b>(i,j) = (0,0,0);
            }
        }
    }

    for(int i = 0; i < temp.cols - 1; i++)
    {
        int minRow=0;
        while(minRow<temp.rows-1 && temp.at<Vec3b>(minRow,i)[0]==0 && temp.at<Vec3b>(minRow,i)[1]==0 && temp.at<Vec3b>(minRow,i)[2] == 0)
        {
            src.at<Vec3b>(minRow,i) = (0,0,0);
            minRow++;
        }
        if(minRow != temp.rows - 1)
        {
            int maxRow=temp.rows-1;
            while(maxRow > maxRow && temp.at<Vec3b>(maxRow,i)[0]==0 && temp.at<Vec3b>(maxRow,i)[1]==0 && temp.at<Vec3b>(maxRow,i)[2]==0)
            {
                src.at<Vec3b>(maxRow,i) = (0,0,0);
                maxRow--;
            }
        }
    }
}

//removes noise from a picture
void removenoise(Mat image)
{
    //Morphologial opening
    erode(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
    dilate(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
    //Morphological closing
    dilate(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
    erode(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
}

//get robot dist from sample
void tilt_turn_degrees(Mat img, int object_rows, int object_cols, sample_loc* orientation){
    double camera_height = .61;     // height of camera from ground in meters
    double camera_angle = 90;	    //angle of camera
    int camera_diagonal_angle = 69; // diagonal angle of view for camera in degrees
                                    // logitech c525 fov is 69 degrees, Samsung Galaxy S5 is 90 degrees

    int rows = img.rows; // height of camera image in pixels
    int cols = img.cols; // width of camera image in pixels
    //cout << "Rows: " << rows << "\n" << "Cols: " << cols << endl;

    //logitech c525 fov is 69 degrees, Samsung Galaxy S5 is 90 degrees
    double camera_diagonal = 69; // the angle of the cameras diagonal in degrees
    double pixel_diagonal = sqrt(rows * rows + cols * cols); // (pythagorean) diagonal length of image in pixels
    double degrees_per_pixel = camera_diagonal / pixel_diagonal; // ratio of real world degrees to pixels in the image

    int center_rows = rows / 2; // the center height is half of the total height
    int center_cols = cols / 2; // the center width is half of the total width
    //cout << "Center Rows: " << center_rows << "\n" << "Center Cols: " << center_cols << endl;

    int diff_rows = center_rows - object_rows; // difference between center and object rows
    int diff_cols = center_cols - object_cols; // difference between center and object cols
    //cout << "Diff Rows: " << diff_rows << "\n" << "Diff Cols: " << diff_cols << endl;

    double turn_robot_x_degrees = diff_cols * degrees_per_pixel; // positive -> turn left, negative -> turn right
    double tilt_camera_x_degrees = diff_rows * degrees_per_pixel; // positive -> tilt up, negative -> tilt down
    cout << "Turn robot " << turn_robot_x_degrees << " degrees.\n" << "Tilt camera " << tilt_camera_x_degrees << " degrees." << endl;

    double tilted_degrees = 90 + tilt_camera_x_degrees; // assuming camera is parallel to ground (90 degrees)

    double tilted_radians = tilted_degrees * M_PI / 180.0; // c++ tan() function uses radians
    double camera_radians = camera_angle * M_PI / 180.0;

    double height = camera_height; // height of camera from the ground in meters

    double distance = height * tan(tilted_radians); // triangle formula for finding distance

    cout << "Distance is " << distance << " meters" << endl;
    orientation->distance=distance;
}

//get angle from camera to keypoint
void robot_angle(sample_loc *orientation, Mat img, int object_cols)
{
    double cols = img.cols;                         // width of camera image in pixels

    double center_cols = cols / 2.0;                // the center width is half of the total width

    double diff_cols = center_cols - object_cols;   // difference between center and object cols

    double turn_robot_x_degrees_other = .0758 * diff_cols - .2716;

//    cout << "Turn robot " << turn_robot_x_degrees_other << " degrees." << endl;
    orientation->angle_from_robot = turn_robot_x_degrees_other;
}

//get angle from camera to keypoint
void robot_angle(beacon_loc *orientation, Mat img, int object_cols)
{
    double cols = img.cols;                         // width of camera image in pixels

    double center_cols = cols / 2.0;                // the center width is half of the total width

    double diff_cols = center_cols - object_cols;   // difference between center and object cols

    double turn_robot_x_degrees_other = .0758 * diff_cols - .2716;

//    cout << "Turn robot " << turn_robot_x_degrees_other << " degrees." << endl;
    orientation->angle_from_robot = turn_robot_x_degrees_other;
}

//takes vector of (hopefully 4) keypoints and finds the center between the leftmost and rightmost keypoints
Point findCenterPoint(vector<KeyPoint> keypoints)
{
    KeyPoint leftPoint = getLeftKeyPoint(keypoints);
    KeyPoint rightPoint = getRightKeyPoint(keypoints);

    int left = leftPoint.pt.x;
    int right = rightPoint.pt.x;
    int top = leftPoint.pt.y;
    int bot = rightPoint.pt.y;

    int xcent = (right+left)/2;
    int ycent = (top+bot)/2;

    return Point(xcent,ycent);
}

//returns the leftmost keypoint in a vector
KeyPoint getLeftKeyPoint(vector<KeyPoint> keypoints)
{
    int left = keypoints[0].pt.x;
    KeyPoint leftPoint = keypoints[0];

    for(int i = 1; i < keypoints.size(); i++)
    {
        if(keypoints[i].pt.x < left)
        {
            leftPoint = keypoints[i];
            left = leftPoint.pt.x;
        }
    }
    return leftPoint;
}

//returns the rightmost keypoint in a vector
KeyPoint getRightKeyPoint(vector<KeyPoint> keypoints)
{
    int right = keypoints[0].pt.x;
    KeyPoint rightPoint = keypoints[0];

    for(int i = 1; i < keypoints.size(); i++)
    {
        if(keypoints[i].pt.x > right)
        {
            rightPoint = keypoints[i];
            right = rightPoint.pt.x;
        }
    }
    return rightPoint;
}

//returns the bottom most keypoint in a vector
KeyPoint getBottomKeyPoint(vector<KeyPoint> keypoints)
{
    int bot = keypoints[0].pt.y;
    KeyPoint botPoint = keypoints[0];

    for(int i = 1; i < keypoints.size(); i++)
    {
        if(keypoints[i].pt.y > bot)
        {
            botPoint = keypoints[i];
            bot = botPoint.pt.y;
        }
    }
    return botPoint;
}

//returns the topmost keypoint in a vector
KeyPoint getTopKeyPoint(vector<KeyPoint> keypoints)
{
    int top = keypoints[0].pt.y;
    KeyPoint topPoint = keypoints[0];

    for(int i = 1; i < keypoints.size(); i++)
    {
        if(keypoints[i].pt.y < top)
        {
            topPoint = keypoints[i];
            top = topPoint.pt.y;
        }
    }
    return topPoint;
}

/*
//calculates the camera's distance from the beacon
void printDistanceFromLights(vector<KeyPoint> keypoints, beacon_loc* orientation)
{
    int top = getTopKeyPoint(keypoints).pt.y;
    int bot = getBottomKeyPoint(keypoints).pt.y;

    double height = bot - top;
//        cout << "top = " << top << endl;
//	cout << "bot = " << bot << endl;
//	 cout << "height = " << height << endl;
    int dist=40051*pow(height,-.997);

    orientation->distance = dist * 0.0254; //convert inches to meters

//    cout << "Distance is " << dist << endl;
}

//calculates the orientation of the beacon
void getBeaconOrientation(vector<KeyPoint> keypoints, beacon_loc* orientation)
{
    KeyPoint topPoint = getTopKeyPoint(keypoints);
    KeyPoint botPoint = getBottomKeyPoint(keypoints);
    KeyPoint leftPoint = getLeftKeyPoint(keypoints);
    KeyPoint rightPoint = getRightKeyPoint(keypoints);

    int top = topPoint.pt.y;
    int bot = botPoint.pt.y;
    int left = getLeftKeyPoint(keypoints).pt.x;
    int right = getRightKeyPoint(keypoints).pt.x;

//	cout << "top: (" << topPoint.pt.x << "," << topPoint.pt.y << ")" << endl;
//	cout << "bot: (" << botPoint.pt.x << "," << botPoint.pt.y << ")" << endl;
//	cout << "left: (" << leftPoint.pt.x << "," << leftPoint.pt.y << ")" << endl;
//	cout << "right: (" << rightPoint.pt.x << "," << rightPoint.pt.y << ")" << endl;




    double centerLine = (topPoint.pt.x + botPoint.pt.x) / 2.0;
    double width = right - left;

    int leftSeparation = centerLine - left;
    int rightSeparation = right - centerLine;

//    cout <<"leftSeparation = " << leftSeparation << endl;
//    cout << "rightSeparation = " << rightSeparation << endl;

    double ratio = (double)rightSeparation / (double)leftSeparation;

//    cout << "ratio = " << ratio << endl;

    if ((rightSeparation) > (leftSeparation)) //if left light appears closer to center of beacon
    {
        orientation->angle_from_beacon *= -1;
		ratio = 1.0/ratio;
    }
    int leftx = getLeftKeyPoint(keypoints).pt.x;
	int lefty=getLeftKeyPoint(keypoints).pt.y;
    int rightx = getRightKeyPoint(keypoints).pt.x;
    int righty = getRightKeyPoint(keypoints).pt.y;
    double centerx = (topPoint.pt.x + botPoint.pt.x) / 2.0;
    double centery = (topPoint.pt.y + botPoint.pt.y) / 2.0;
	double leftlen=sqrt(pow((centerx-leftx),2)+pow((centery-lefty),2));
	double rightlen=sqrt(pow((centerx-rightx),2)+pow((centery-righty),2));
//cout<<"ratio of right/left: "<<rightlen/leftlen<<endl;
double rat=rightlen/leftlen;
double test=-110*rat*rat + 387.4*rat - 276.18;
//cout<<"angle= "<<test<<endl;
    orientation->angle_from_beacon = 887.52*ratio*ratio - 1877.7*ratio + 990.86;
 //   cout << "beacon orientation is " << orientation->angle_from_beacon << endl;
}
*/

//returns a picture taken with the given camera
Mat getPic(VideoCapture cap)
{
    Mat img;
    struct timeval tv1,tv2;
    double duration = 0;
    double timer = .2;

    cout << "Taking pic in " << timer << " s"<<endl;

    gettimeofday(&tv1, NULL);
    duration=0;

    while(duration<timer)
    {
        cap >> img;
        gettimeofday(&tv2, NULL);
        duration = ((double)((tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec)))/1000000.00;
    }

    cout << "Taking pic" << endl;
    cap >> img;

    return img;
}

//creates viewable windows (view using showWindows())
void createWindows()
{
    namedWindow("Original 1", WINDOW_NORMAL);
    namedWindow("Original 2", WINDOW_NORMAL);
    namedWindow("Original 3", WINDOW_NORMAL);
    namedWindow("Original 4", WINDOW_NORMAL);
    namedWindow("Diff1", WINDOW_NORMAL);
    namedWindow("Diff2", WINDOW_NORMAL);
    namedWindow("bit_and", WINDOW_NORMAL);
}

//show the windows previously created with createWindows()
void showWindows(Mat img1, Mat img2, Mat img3, Mat img4, Mat diff1, Mat diff2, Mat out)
{
    imshow("Original 1", img1);
    imshow("Original 2", img2);
    imshow("Original 3", img3);
    imshow("Original 4", img4);
    imshow("Diff1", diff1 );
    imshow("Diff2", diff2 );
    imshow("bit_and", out);
    waitKey(20);
}

//sets up the parameters for SimpleBlobDetector to find beacon(just to clean up the main code)
SimpleBlobDetector::Params setupBeaconBlobParams()
{
    SimpleBlobDetector::Params params;

    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = true;
    params.filterByConvexity = false;
    params.filterByColor = false;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minThreshold = 100;
    params.maxThreshold = 255;
    params.thresholdStep = 1;
    params.minArea = 0;
    params.minConvexity = 0.50;
    params.minInertiaRatio = 0.50;
    params.maxArea = 2000;
    params.maxConvexity = 10;

    return params;
}

//sets up the parameters for SimpleBlobDetector to find object (just to clean up the main code)
SimpleBlobDetector::Params setupObjectBlobParams()
{
    SimpleBlobDetector::Params params;

    params.minDistBetweenBlobs = 0.0f;
    params.filterByInertia = true;
    params.filterByConvexity = false;
    params.filterByColor = false;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minThreshold = 100;
    params.maxThreshold = 250;
    params.thresholdStep = 1;
    params.minArea = 10;
    params.minConvexity = 0.3;
    params.minInertiaRatio = 0.15;
    params.maxArea = 8000;
    params.maxConvexity = 10;

    return params;
}

//returns an image with the keypoints drawn on the given picture
Mat drawAndCircleKeypoints(vector<KeyPoint> keypoints, Mat binDiff)
{
    Mat img;
    drawKeypoints(binDiff, keypoints, img, CV_RGB(0,0,0), DrawMatchesFlags::DEFAULT);

    //Circle blobs
    for(int i = 0; i < keypoints.size(); i++)
    {
        if(keypoints[i].size > 0)
        {
            circle(img, keypoints[i].pt, 1.5*keypoints[i].size, CV_RGB(0,0,255), 5, 8);
        }
    }
    return img;
}

//routine repeatedly used to detect beacon
void threshDilateDetect(Mat grayDiff, Mat binDiff, double thresh, SimpleBlobDetector::Params params, vector<KeyPoint> &keypoints)
{
    threshold(grayDiff, binDiff, thresh, 255, THRESH_BINARY);
    dilate(binDiff, binDiff, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    Ptr<SimpleBlobDetector> blobDetect = SimpleBlobDetector::create(params);
    blobDetect->detect(binDiff, keypoints);
}

//sets up the camera
void initializePointAndShoot()
{
    system("killall PTPCamera");
    system("gphoto2 --set-config zoom=0");
}

//zooms the camera all the way in
void zoomInFull()
{
    system("gphoto2 --set-config zoom=8");
}

//zooms the camera all the way out
void zoomOutFull()
{
    system("gphoto2 --set-config zoom=0");
}

//takes a picture and saves it
void shootPic()
{
    system("yes | gphoto2 --capture-image-and-download --filename \"newPic.jpg\"");
}

//returns true if success, false otherwise
bool beaconLocation(vector<KeyPoint> imgKeyPoints, beacon_loc *b_loc) 
{
    	const float BOTTOM_DIST = 20.0; //inches
    	const float TOP_DIST = 33.5; //up is negative y in solvePNP
    	const float LEFT_DIST = 30.0;
    	const float RIGHT_DIST = 30.0;

    	vector<KeyPoint> keyPoints(4);
    	keyPoints[0] = getTopKeyPoint(imgKeyPoints);
    	keyPoints[1] = getLeftKeyPoint(imgKeyPoints);
    	keyPoints[2] = getRightKeyPoint(imgKeyPoints);
    	keyPoints[3] = getBottomKeyPoint(imgKeyPoints);


	//convert keypoints to point2f points
	vector<Point2f> imgPoints;
	KeyPoint::convert(keyPoints, imgPoints);

	//fill known points with beacon dimensions
	vector<Point3f> kwnPoints = {Point3f(0, -1*TOP_DIST, 0), //Top
				     Point3f(-1*LEFT_DIST, 0, 0), //Left
				     Point3f(RIGHT_DIST, 0, 0),  //Right
				     Point3f(0, BOTTOM_DIST, 0) //Bottom
	};



	//get saved calibration matrix
	string filename = "/home/buckeye/catkin_ws/src/CapstoneROS/src/vision/out_camera_data.xml";
	FileStorage fs(filename, FileStorage::READ);

	if(!fs.isOpened())
	{
		cout<<"Calibration file could not be opened"<<endl;
		return false;
	}

	Mat cameraMatrix, distCoeffs;
	fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distCoeffs;

	if(cameraMatrix.empty() || distCoeffs.empty())
	{
		cout << "Calibration file not formatted correctly" << endl;
		return false;
	}
	else
	{
		cout << "Camera Matrix" << endl;
		cout << cameraMatrix << endl;
		cout << "Distortion Coefficients" << endl;
		cout << distCoeffs << endl;
	}

	cout << "Known Points" << endl;
	cout << kwnPoints << endl;
	cout << "Image Points" << endl;
	cout << imgPoints << endl;

	Mat rvec, tvec, R;
    bool guess;
    if(!b_loc->beacon_not_found) {
        guess = true;
    	rvec=(Mat_<double>(3,1)<<   0,
                                    atan(b_loc->x / b_loc->y)- M_PI*b_loc->angle_from_robot/180.0,
                                    0);

    	tvec=(Mat_<double>(3,1)<<   -1*b_loc->x,
                                    6, 
                                    -1*b_loc->y);

	cout << "Guessed Rotation vector" << endl;
        cout << rvec << endl;
        cout << "Guessed Translation vector" << endl;
        cout << tvec << endl;

        Rodrigues(rvec, R);
        tvec = -R.t() * tvec;
        Rodrigues(R.t(), rvec);

    } else {
        guess = false;
    }

	//Mat rvec, tvec;
	bool succ = solvePnP(Mat(kwnPoints), Mat(imgPoints), cameraMatrix, distCoeffs, rvec, tvec, guess);

    if(!succ) {
        cout<<"Could not calculate position of beacon"<<endl;
        b_loc->beacon_not_found;
        return false;
    }

	Mat camera_rvec, camera_tvec;

	Rodrigues(rvec, R);
	Rodrigues(R.t(), camera_rvec);

	camera_tvec = -R.t() * tvec;

	//print out stuff for sanity check
        cout << "Rotation vector" << endl;
        cout << rvec << endl;
        cout << "Translation vector" << endl;
        cout << tvec << endl;
	cout << "Camera Rotation vector" << endl;
	cout << camera_rvec << endl;
	cout << "Camera Translation vector" << endl;
	cout << camera_tvec << endl;
	//fill beacon struct with appropriate values
    float distance = sqrt(camera_tvec.at<double>(0) * camera_tvec.at<double>(0) + camera_tvec.at<double>(2) * camera_tvec.at<double>(2));
    float bangle = 180.0 * atan(camera_tvec.at<double>(0) / camera_tvec.at<double>(2)) / M_PI;
    float rangle = bangle - 180.0 * camera_rvec.at<double>(1) / M_PI;

    cout << endl << endl;
    cout << "Distance is " << distance << " inches." << endl;
    cout << "Angle from beacon is " << bangle << " degrees." << endl;
    cout << "Angle from robot is " << rangle << " degrees." << endl;
    cout << "Beacon x is " << -1*camera_tvec.at<double>(0) << " inches." << endl;
    cout << "Beacon y is " << -1*camera_tvec.at<double>(2) << " inches." << endl;

    b_loc->x = -1*camera_tvec.at<double>(0);
    b_loc->y = -1*camera_tvec.at<double>(2);
    b_loc->angle_from_robot = rangle;


	return true;

}
