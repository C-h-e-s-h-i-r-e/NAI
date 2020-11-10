#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <opencv2/imgproc.hpp>

// g++ `pkg-config --cflags opencv4` cv.cpp `pkg-config --libs opencv4`

using namespace std;
using namespace cv;

double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

class MyObject
{
public:
	int maxC;
	vector<Point> pos;
	Point getP()
	{
		Point sump = std::accumulate(pos.begin(), pos.end(), Point(0, 0));
		sump.x /= pos.size();
		sump.y /= pos.size();
		return sump;
	}
	void addP(Point p)
	{
		pos.push_back(p);
		if (pos.size() > maxC)
		{
			pos = vector<Point>(pos.begin() + 1, pos.end());
		}
	}
	void addEmpty()
	{
		if (pos.size() > 0)
		{
			pos = vector<Point>(pos.begin() + 1, pos.end());
		}
	}
};

int main()
{
    //
    int loRange[3] = {80, 54, 42};
	int hiRange[3] = {112, 255, 161};

	namedWindow("jakostam", cv::WINDOW_AUTOSIZE);
	createTrackbar("loRange0", "jakostam", &(loRange[0]), 255);
	createTrackbar("loRange1", "jakostam", &(loRange[1]), 255);
	createTrackbar("loRange2", "jakostam", &(loRange[2]), 255);
	createTrackbar("hiRange0", "jakostam", &(hiRange[0]), 255);
	createTrackbar("hiRange1", "jakostam", &(hiRange[1]), 255);
	createTrackbar("hiRange2", "jakostam", &(hiRange[2]), 255);
	VideoCapture camera(0);
	//VideoCapture backgroundvid("Multiwave.wmv");
	//Mat background = imread("plaza.jpg", cv::IMREAD_COLOR);

	MyObject myobj;
	myobj.maxC = 10;

    int canny_a = 30, canny_b = 250;

	namedWindow("prostokaty", cv::WINDOW_AUTOSIZE);
	createTrackbar("A", "prostokaty", &canny_a, 255);
	createTrackbar("B", "prostokaty", &canny_b, 255);
	//VideoCapture camera(0); // 0 - domyslna, 1 - podczerwien,

	while (waitKey(1) != 27)
	{
		int dilation_size = 5;
		auto structElem = getStructuringElement(MORPH_ELLIPSE,
												Size(2 * dilation_size + 1, 2 * dilation_size + 1),
												Point(dilation_size, dilation_size));
		Mat frame2;
		Mat backgroundScaled;
		Mat frameMask, frameNegMask;
		Mat frameWithMask, backgroundScaledWithMask;
		Mat meinniceplace;

		camera >> frame2;
		flip(frame2, frame2, 1);
		//resize(background, backgroundScaled, {frame.cols, frame.rows});

		

		

		
		Mat frame, frame0, frameBw, frameCanny;
		camera >> frame;
		flip(frame, frame, 1);
		frame0 = frame.clone();
		imshow("frame_from_cam", frame);
		cvtColor(frame, frameBw, COLOR_BGR2GRAY);
		//imshow("bw", frameBw);
		equalizeHist(frameBw, frameBw);
		//imshow("bw_hist", frameBw);

		Canny(frameBw, frameCanny, canny_a, canny_b, 3);
		//imshow("bw_Canny", frameCanny);
		static auto ellipse = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
		morphologyEx(frameBw, frameBw, MORPH_CLOSE, ellipse);
		morphologyEx(frameBw, frameBw, MORPH_OPEN, ellipse);

		Canny(frameBw, frameCanny, canny_a, canny_b, 3);
		//imshow("bw_Canny_2", frameCanny);
		static auto ellipse_33 = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		morphologyEx(frameCanny, frameCanny, MORPH_DILATE, ellipse_33);

		//imshow("bw_Canny_2_dil", frameCanny);
		vector<vector<Point>> contours;
		vector<vector<Point>> contours_4;
		findContours(frameCanny, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
		for (int i = 0; i < contours.size(); i++)
		{
			approxPolyDP(contours[i], contours[i], 10, true);
			if (contours[i].size() == 4)
			{
				double amax = 0;
				for (int j = 0; j < 4; j++)
				{
					double a = fabs(angle(contours[i][j], contours[i][(j + 1) % 4], contours[i][(j + 1) % 4]));
					amax = max(a, amax);
				}
				if (amax < 0.4)
					contours_4.push_back(contours[i]);
			}
		}

		for (int i = 0; i < contours_4.size(); i++)
		{
			drawContours(frame, contours_4, i, Scalar(0, 0, 255));
		}
		if (contours_4.size() > 0)
		{
			sort(contours_4.begin(), contours_4.end(),
				 [](auto &a, auto &b) {
					 return contourArea(a, false) > contourArea(b, false);
				 });
			drawContours(frame, contours_4, 0, Scalar(255, 255, 255));
			Mat dstMat(Size(600, 400), CV_8UC3);
			vector<Point2f> src = {{0, 0}, {dstMat.cols, 0}, {dstMat.cols, dstMat.rows}, {0, dstMat.rows}};
			vector<Point2f> dst;
			for (auto p : contours_4[0])
				dst.push_back(Point2f(p.x, p.y));
			auto wrap_mtx = getPerspectiveTransform(dst, src);
			warpPerspective(frame0, dstMat, wrap_mtx, Size(dstMat.cols, dstMat.rows));

			imshow("RESULT", dstMat);

cvtColor(dstMat, frameMask, cv::COLOR_BGR2HSV);
		inRange(frameMask, Scalar(loRange[0], loRange[1], loRange[2]),
				Scalar(hiRange[0], hiRange[1], hiRange[2]), frameNegMask);
		morphologyEx(frameNegMask, frameNegMask, MORPH_CLOSE, structElem);
		morphologyEx(frameNegMask, frameNegMask, MORPH_OPEN, structElem);
		imshow("dilate", frameNegMask);

		vector<vector<Point>> contours2;
		findContours(frameNegMask, contours2, cv::RETR_LIST, cv::CHAIN_APPROX_TC89_KCOS);
		sort(contours2.begin(), contours2.end(),
			 [](auto &a, auto &b) {
				 return contourArea(a, false) > contourArea(b, false);
			 });

			 for (int i = 0; i < contours2.size(); i++)
		{
			approxPolyDP(contours2.at(i), contours2.at(i), 10, true);
			//			drawContours(frame, contours, i, {0, 0, 255, 255});
			//			auto txtpos = contours.at(i).at(0);
			//			putText(frame, to_string(contours.at(i).size()), txtpos, cv::FONT_HERSHEY_PLAIN, 2, {0, 0, 255, 255});
			//			txtpos.y += 30;
			//			putText(frame, to_string(contourArea(contours.at(i), false)), txtpos, cv::FONT_HERSHEY_PLAIN, 2, {0, 0, 255, 255});
			//			txtpos.y -= 60;
			//			putText(frame, to_string(i), txtpos, cv::FONT_HERSHEY_PLAIN, 2, {0, 0, 255, 255});
		}
		if (contours2.size())
		{
			Point avg;
			Rect r = boundingRect(contours2.at(0));
			avg.x = r.x + r.width / 2;
			avg.y = r.y + r.height / 2;
			myobj.addP(avg);
//			putText(frame, "0", avg, cv::FONT_HERSHEY_PLAIN, 2, {0, 255, 255, 255});
		}
		else
		{
			myobj.addEmpty();
		}
		// if (myobj.pos.size() > 1) {
		// 	putText(frame2, "X", myobj.getP(), cv::FONT_HERSHEY_PLAIN, 2, {255, 0, 255, 255});
			
		// 	vector<vector<Point>>ctrs = {myobj.pos};
		// 	drawContours(frame2, ctrs, 0, {255, 0, 255, 255});	
		// }
		//imshow("contours", frame2);
    //
Mat dstMat1;
dstMat1 = dstMat.clone();
			if (myobj.pos.size() > 1) {
			putText(dstMat1, "X", myobj.getP(), cv::FONT_HERSHEY_PLAIN, 2, {255, 0, 255, 255});
			
			vector<vector<Point>>ctrs = {myobj.pos};
			drawContours(dstMat1, ctrs, 0, {255, 0, 255, 255});	
		}
			imshow("RESULTwC", dstMat1);
 

char k = cv::waitKey(1);
    if (k == 27) break;
    if (k == 'x') {

if (myobj.pos.size() > 1) {



        int x = myobj.getP().x ;
        int y = myobj.getP().y ;
        
    Mat dst;

        if(x > 300 && y > 200){rotate(dstMat, dst, cv::ROTATE_90_CLOCKWISE);
		
		// putText(dst, "x> y>", cv::Point(100, 100), //top-left position
        //     cv::FONT_HERSHEY_DUPLEX,
        //     1.0,
        //     CV_RGB(118, 185, 0), //font color
        //     2);
		}

        if(x < 300 && y < 300){rotate(dstMat, dst, cv::ROTATE_90_COUNTERCLOCKWISE); 
		
		// putText(dst, "x< y<", cv::Point(120,120), //top-left position
        //     cv::FONT_HERSHEY_DUPLEX,
        //     1.0,
        //     CV_RGB(118, 185, 0), //font color
        //     2);
		}

        if(x < 300 && y > 200){rotate(dstMat, dst, cv::ROTATE_90_COUNTERCLOCKWISE);
		
		// putText(dst, "x< y>", cv::Point(130,130), //top-left position
        //     cv::FONT_HERSHEY_DUPLEX,
        //     1.0,
        //     CV_RGB(118, 185, 0), //font color
        //     2);
		}

 		if(x > 300 && y < 200){rotate(dstMat, dst, cv::ROTATE_90_CLOCKWISE);
		 
		//  putText(dst, "x> y<", cv::Point(140,140), //top-left position
        //     cv::FONT_HERSHEY_DUPLEX,
        //     1.0,
        //     CV_RGB(118, 185, 0), //font color
        //     2);
		}

    imwrite("rotated_result.jpg", dst);
} 
    }
            
		}
		//imshow("frame_from_cam", frame);
	}
	return 0;
}