//功能： 透视变换完成图像畸变矫正
//缺点： 由于答题卡形状问题，无法完成角点自动选取，故采用鼠标人工选择
//作者： liangzelang（liangzelang@gmail.com）
//时间： 2017/05/18
//版本： 0.01

#include <opencv2\opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

vector<Point2f> srcpt(4);
vector<Point2f> dstpt(4);
int ptflag=0;
Mat pic;
Mat perImage;
Mat pdstImage;
Mat pmidImage;
Mat psrcImage;
int threshold_value = 100;
static void on_Mouse(int event, int x, int y, int flags, void *);
void on_Change(int, void*);

int main()
{
	Mat dstImage;
	Mat midImage;

	pic = imread("D:\\C++程序联系文件夹（可选择性删除）\\Answer-Card-Recognition\\pic\\1.jpg",1);
	resize(pic, pic, cv::Size(480, 640));	
	imshow("source", pic);
	namedWindow("fuck",1);
	setMouseCallback("source", on_Mouse, 0);
	waitKey(0);
	return 0;

}

void on_Change(int,void *)
{	
	threshold(pmidImage, pdstImage, threshold_value, 255, 0);
	imshow("process", pdstImage);
}

//-----------------------------------【onMouse( )函数】---------------------------------------
//		描述：鼠标消息回调函数
//-----------------------------------------------------------------------------------------------
static void on_Mouse( int event, int x, int y, int flags, void* )
{	
	if( x < 0 || x >= pic.cols || y < 0 || y >= pic.rows )
	{
		cout << "Mouse is out \n" << endl;
		return;
	}		
	switch(event)
	{
		case CV_EVENT_LBUTTONUP :
			srcpt[ptflag] = Point(x,y);
			//srcpt.push_back(Point(x,y));  //保存选取的点
			cout << "The chosen point is : " << srcpt[ptflag].x << " , " << srcpt[ptflag].y << endl;
			ptflag++;
			if(ptflag == 4)
			{
				ptflag = 0;
				cout << "Work has done\n" << endl;
				dstpt[0] = Point2f(0,0);
				dstpt[3] = Point2f(0,640);
				dstpt[2] = Point2f(480,640);
				dstpt[1] = Point2f(480,0);
			
				//求取映射矩阵
				perImage = Mat::zeros(640,480, CV_8UC3);   //这里的zeros 第一个参数是rows 行  第二是cols 列  和width（）函数定义正好相反
				Mat transMat = getPerspectiveTransform(srcpt, dstpt);
				warpPerspective(pic, perImage, transMat, perImage.size());	
				imshow("fuck", perImage);

				imwrite("D:\\C++程序联系文件夹（可选择性删除）\\Answer-Card-Recognition\\pic\\result.jpg",perImage);	

				if(ptflag == 0)
				{
					psrcImage = imread("D:\\C++程序联系文件夹（可选择性删除）\\Answer-Card-Recognition\\pic\\result.jpg",1);
					cvtColor(psrcImage, pmidImage, CV_RGB2GRAY);
					namedWindow("process", 1);
					createTrackbar("value", "process", &threshold_value, 255, on_Change);
				}
			}
				
			break;
		default:
			break;
	}
}

