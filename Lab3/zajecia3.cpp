#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>


// g++ `pkg-config --cflags opencv4` cv.cpp `pkg-config --libs opencv4`

using namespace std;
using namespace cv;

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
	int loRange[3] = {20, 92, 76};
	int hiRange[3] = {119, 255, 255};

	namedWindow("color", cv::WINDOW_AUTOSIZE);
	createTrackbar("loRange0", "color", &(loRange[0]), 255);
	createTrackbar("loRange1", "color", &(loRange[1]), 255);
	createTrackbar("loRange2", "color", &(loRange[2]), 255);
	createTrackbar("hiRange0", "color", &(hiRange[0]), 255);
	createTrackbar("hiRange1", "color", &(hiRange[1]), 255);
	createTrackbar("hiRange2", "color", &(hiRange[2]), 255);
	VideoCapture camera(0);
	

	MyObject myobj;
	myobj.maxC = 10;
	while (waitKey(1) != 27)
	{
		int dilation_size = 5;
		auto structElem = getStructuringElement(MORPH_ELLIPSE,
												Size(2 * dilation_size + 1, 2 * dilation_size + 1),
												Point(dilation_size, dilation_size));
		Mat frame;
		Mat frameMask, frameNegMask;
		Mat frameWithMask, backgroundScaledWithMask;

		camera >> frame;
		flip(frame, frame, 1);

		cvtColor(frame, frameMask, cv::COLOR_BGR2HSV);
		inRange(frameMask, Scalar(loRange[0], loRange[1], loRange[2]),
				Scalar(hiRange[0], hiRange[1], hiRange[2]), frameNegMask);
		morphologyEx(frameNegMask, frameNegMask, MORPH_CLOSE, structElem);
		morphologyEx(frameNegMask, frameNegMask, MORPH_OPEN, structElem);
		imshow("dilate", frameNegMask);

		vector<vector<Point>> contours;
		findContours(frameNegMask, contours, cv::RETR_LIST, cv::CHAIN_APPROX_TC89_KCOS);
		sort(contours.begin(), contours.end(),
			 [](auto &a, auto &b) {
				 return contourArea(a, false) > contourArea(b, false);
			 });


// kod 


        if (contours.size() > 1) {
            Rect table[2] = {Rect(), Rect()};

            for(auto c:contours){
                Rect r = boundingRect(c);
                int area = r.area();
                if(area > table[0].area()) {
                    table[0] = r;
                } else if (area > table[1].area()) {
                    table[1] = r;
                }
            }

        for(int i = 0;i < 2;i++) {
            rectangle(frame, table[i], {255, 0, 255}, 2);
        }

		int liczba = abs(table[0].y - table[1].y);


        if(liczba < 100) {

			  arrowedLine(frame, 
        {table[0].x + table->width / 2, table[0].y + table->height / 2},
        {table[1].x + table->width / 2, table[1].y + table->height / 2},
        {0, 255, 0}, 3);

        }

        }

		imshow("contours", frame);
	}
	return 0;
}