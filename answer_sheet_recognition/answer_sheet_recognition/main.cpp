#include <opencv2\opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

//Mat source_pic;
void rotateImage(Mat & srcImage, Mat & dstImage, float degree)
{
	Point2f center(srcImage.cols/2, srcImage.rows/2);
	Mat rotateMat;
	rotateMat = getRotationMatrix2D(center, degree, 1 );
	warpAffine(srcImage, dstImage, rotateMat, srcImage.size());
}

int main()
{
	Mat pic = imread("D:\\C++程序联系文件夹（可选择性删除）\\Answer-Card-Recognition\\pic\\2.jpg",1);
	resize(pic, pic, cv::Size(480, 640));
	Mat dstImage;
    Canny(pic, dstImage, 50, 150, 3);
	imshow("canny", dstImage);
	//cvtColor(dstImage, dstImage, CV_GRAY2BGR);
	//

	vector<Vec2f> lines;
	HoughLines(dstImage, lines, 1, CV_PI/180, 255, 0, 0);

	for(size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0] , theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);  
        double x0 = a*rho, y0 = b*rho;  
        pt1.x = cvRound(x0 + 1000*(-b));  
        pt1.y = cvRound(y0 + 1000*(a));  
        pt2.x = cvRound(x0 - 1000*(-b));  
        pt2.y = cvRound(y0 - 1000*(a));  
        line( pic, pt1, pt2, Scalar(55,100,195), 1, CV_AA); 
	}
	Mat pic_rot;
	cout << "倾斜角度： " << lines[0][1] << endl; 
	rotateImage(pic, pic_rot, -lines[0][1]);
	imshow("source", pic);
	imshow("rot", pic_rot);
	//imshow(pic)
	waitKey(0);
	return 0;

}