#include "functions.h"
#include "blob.h"

sample_loc blob_main(float min, float max)
{
    sample_loc s_loc;
    int obj=2;
    hsvParams hsvWhite = {0,0,200,180,90,255};
    hsvParams hsvPurple = {45,0,20,150,90,255};
    hsvParams hsv = obj==1? hsvWhite:hsvPurple;

    //Set up blob detection parameters
    SimpleBlobDetector::Params params = setupObjectBlobParams();

    vector<KeyPoint> keypoints;

    const string filename("/home/buckeye/catkin_ws/src/CapstoneROS/src/vision/medPurple.jpg");
    //Initialize camera
/*    VideoCapture cap(1);
    if ( !cap.isOpened() ){
        cout << "Cannot open the web cam" << endl;
        return -1;
    }
*/
    while(true){
        Mat img, imgHSV, imgTHRESH, out;
img = imread(filename, CV_LOAD_IMAGE_COLOR);
       // cap>>img;

        if(img.empty()){
            cout << "can not open image" << endl;
            return s_loc;
        }

        //convert color to HSV, threshold and remove noise
        cvtColor(img, imgHSV, COLOR_BGR2HSV);
        findGrass(img,imgHSV);
        cvtColor(img, imgHSV, COLOR_BGR2HSV);

        inRange(imgHSV, Scalar(hsv.hL, hsv.sL, hsv.vL), Scalar(hsv.hH, hsv.sH, hsv.vH), imgTHRESH);
        removenoise(imgTHRESH);

        namedWindow("Input", WINDOW_AUTOSIZE);
        namedWindow("Detection", WINDOW_AUTOSIZE);

        Ptr<SimpleBlobDetector> blobDetect = SimpleBlobDetector::create(params);
        blobDetect->detect( imgTHRESH, keypoints );

        drawKeypoints(imgTHRESH, keypoints, out, CV_RGB(0,0,255), DrawMatchesFlags::DEFAULT);
        //Circle blobs
        for(int i = 0; i < keypoints.size(); i++)
            circle(out, keypoints[i].pt, 1.5*keypoints[i].size, CV_RGB(0,255,0), 20, 8);

        if(keypoints.size() == 1){
            cout<<endl<<endl<<"Object Found"<<endl;
            tilt_turn_degrees(imgTHRESH, keypoints[0].pt.y, keypoints[0].pt.x, &s_loc);
	    robot_angle(&s_loc, imgTHRESH, keypoints[0].pt.x);
        }
        else{
            cout<<"No Object Found"<<endl;
        }

        imshow("Input", img);
        imshow("Detection", out);
        waitKey(-1);

    }
  return s_loc;
  }


