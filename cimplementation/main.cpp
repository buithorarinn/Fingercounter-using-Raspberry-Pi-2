#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include "oled_display.h"

using namespace cv;
using namespace std;

int main()
{
    if (!oled_init())
    {
        cerr << "OLED initialization failed!" << endl;
        return -1;
    }

    VideoCapture cap(0);
    if (!cap.isOpened())
    {
        cerr << "Camera not detected!" << endl;
        return -1;
    }

    while (true)
    {
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;

        flip(frame, frame, 1);
        Rect roi_rect(100, 100, 300, 300);
        rectangle(frame, roi_rect, Scalar(0, 255, 0), 2);
        Mat roi = frame(roi_rect);

        Mat hsv;
        cvtColor(roi, hsv, COLOR_BGR2HSV);

        Scalar lower_skin(0, 20, 70);
        Scalar upper_skin(20, 255, 255);
        Mat mask;
        inRange(hsv, lower_skin, upper_skin, mask);

        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
        dilate(mask, mask, kernel, Point(-1, -1), 4);
        GaussianBlur(mask, mask, Size(5, 5), 100);

        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

        int fingers = 0;

        if (!contours.empty())
        {
            size_t max_idx = 0;
            double max_area = 0;

            for (size_t i = 0; i < contours.size(); ++i)
            {
                double area = contourArea(contours[i]);
                if (area > max_area)
                {
                    max_area = area;
                    max_idx = i;
                }
            }

            vector<int> hull;
            vector<Vec4i> defects;

            convexHull(contours[max_idx], hull, false, false);
            if (hull.size() > 3)
            {
                convexityDefects(contours[max_idx], hull, defects);
                int count_defects = 0;

                for (size_t i = 0; i < defects.size(); ++i)
                {
                    Point s = contours[max_idx][defects[i][0]];
                    Point e = contours[max_idx][defects[i][1]];
                    Point f = contours[max_idx][defects[i][2]];

                    double a = norm(s - e);
                    double b = norm(s - f);
                    double c = norm(e - f);
                    double angle = acos((b * b + c * c - a * a) / (2 * b * c)) * 57;

                    if (angle <= 90)
                    {
                        count_defects++;
                        circle(roi, f, 5, Scalar(0, 0, 255), -1);
                    }
                }
                fingers = count_defects + 1;
            }
            else
            {
                fingers = 1;
            }
        }

        string text = "Fingers: " + to_string(fingers);
        putText(frame, text, Point(10, 40), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(255, 0, 0), 2);
        oled_show_message(text);

        imshow("Finger Counter", frame);
        if (waitKey(1) == 'q')
            break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
