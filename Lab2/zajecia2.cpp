#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
int main( int argc, char** argv ) {
    
    bool capturing = true;
	cv::VideoCapture cap(0); 
    // capturing from video files frame seq or camera

if (!cap.isOpened() ) {
    // check if capturing is inicialized 
    std::cerr << "error opening frames source" << std::endl;
    return -1;
}

 int frame_height, frame_width;
    if (argc == 3)
    {
        frame_width = atoi(argv[1]);
        frame_height = atoi(argv[2]);

    }else
    {
        frame_height = 200, frame_width = 320;
    } 

cv::namedWindow("hvs", cv::WINDOW_AUTOSIZE);
// create a window can be referrd by name
int h = 0, s = 0, v = 0, hh = 255, ss = 255, vv = 255;
cv::createTrackbar("h-lower", "hvs", &h, 255);
// creates trackbar and attaches it to window
cv::createTrackbar("s-lower", "hvs", &s, 255);
cv::createTrackbar("v-lower", "hvs", &v, 255);
cv::createTrackbar("h-upper", "hvs", &hh, 255);
cv::createTrackbar("s-upper", "hvs", &ss, 255);
cv::createTrackbar("v-upper", "hvs", &vv, 255);

cv::Mat frame, screen_frame;
// n-dimentional dense array class (macierz)

//do {
// do video


cap.read(frame);
// true/false if frame is coreectrly read true 
cv::GaussianBlur(frame, frame, cv::Size(45 , 45), 0);
// adds gauss to the frame

	

cv::resize(frame, frame, {frame_width, frame_height});
// change the size of window



while(true) {
    cv::imshow("IMG", frame);
    // display an frame in opencv window
    cv::cvtColor(frame, screen_frame, cv::COLOR_BGR2HSV);
    // conver the frame color
    cv::inRange(screen_frame, cv::Scalar(h, s, v), cv::Scalar(hh, ss, vv), screen_frame);
    // check if array elements lie between the elements of 2 other arrays
    int pos = 0;
    cv::putText(screen_frame, "H-low"+ std::to_string(h), {10, pos+=30}, cv::FONT_HERSHEY_PLAIN, 1.0, {0, 255, 0, 255});
    // drows text string on any frame
    cv::putText(screen_frame, "S-low"+ std::to_string(s), {10, pos+=30}, cv::FONT_HERSHEY_PLAIN, 1.0, {0, 255, 0, 255});
    cv::putText(screen_frame, "V-low"+ std::to_string(v), {10, pos+=30}, cv::FONT_HERSHEY_PLAIN, 1.0, {0, 255, 0, 255});
    cv::putText(screen_frame, "H-high"+ std::to_string(hh), {10, pos+=30}, cv::FONT_HERSHEY_PLAIN, 1.0, {0, 255, 0, 255});
    cv::putText(screen_frame, "S-high"+ std::to_string(ss), {10, pos+=30}, cv::FONT_HERSHEY_PLAIN, 1.0, {0, 255, 0, 255});
    cv::putText(screen_frame, "V-high"+ std::to_string(vv), {10, pos+=30}, cv::FONT_HERSHEY_PLAIN, 1.0, {0, 255, 0, 255});
    cv::imshow("screen_frame", screen_frame);
    char k = cv::waitKey(1);
    if (k == 27) break;
    if (k == 'x') {
        cv::Rect2d r = cv::selectROI("screen_frame", screen_frame);
        // class for 2d rectangles  ROI (region of interest)
        cv::imwrite("plik.jpg", cv::Mat(screen_frame, r));
        // saves an frame to specified file
    }
}

}//while(capturing);
// do video


return 0;
}