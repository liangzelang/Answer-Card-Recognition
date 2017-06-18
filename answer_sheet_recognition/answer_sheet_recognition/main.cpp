//功能： 透视变换完成图像畸变矫正
//缺点： 由于答题卡形状问题，无法完成角点自动选取，故采用鼠标人工选择
//作者： liangzelang（liangzelang@gmail.com）
//时间： 2017/05/18
//版本： 0.01

#include <opencv2\opencv.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;
using namespace cv;
const int NUMS = 20;
vector<Point2f> srcpt(4);
vector<Point2f> dstpt(4);
vector<string> Answer(NUMS);
int ptflag=0;
Mat pic;
Mat perImage;
Mat pdstImage;
Mat pmidImage;
Mat psrcImage;

int threshold_value = 100;
static void on_Mouse(int event, int x, int y, int flags, void *);
void on_Change(int, void*);
void process_Pic1();
void project_Pic(const Mat & src, vector<int> & horizon_out, vector<int> & vertical_out);
void findMax(const vector<int> & inputArray, vector<int> & maxIndex);
int findVerMax(const vector<int> & inputArray, vector<int> & upIndex, vector<int> & downIndex);
void findVerticalMax(const vector<int> & inputArray, vector<int> & maxIndex);
void findLocHorizon(const vector<int> & inputArray, vector<int> & locUp, vector<int> & locDown);
void picCut(const Mat & inputArray, Mat & outputArray, const Rect & rectRoi);
void findAnswer(const vector<int> & input, const vector<int> locUp, const vector<int> locDown, string & ans);
int main()
{
	Mat dstImage;
	Mat midImage;
	process_Pic1();
	waitKey(0);
	return 0;

}

int findVerMax(const vector<int> & inputArray, vector<int> & upIndex, vector<int> & downIndex)
{
	int length = inputArray.size();  //0 黑色  255白色
	int maxValue = inputArray[0];
	int m = 0,n= 0;
	int maxIndex = 0;
	for(int i = 1; i < length-1; i++)
	{
		if(inputArray[i-1] ==0 && inputArray[i] > 0)
		{
			upIndex[m] = i; 
			m++;
		}
		else if(inputArray[i-1] > 0 && inputArray[i] == 0)
		{
			downIndex[n] = i;
			n++;
		}
		if(maxValue < inputArray[i])
		{
			maxValue = inputArray[i];
			maxIndex = i;
		}
	}
	return maxIndex;
}

void findVerticalMax(const vector<int> & inputArray, vector<int> & maxIndex)
{
	int length = inputArray.size();
	int maxVal = 0;
	int maxInd = 0;
	//找到最大的index
	for(int i = 0; i < length; i++)
	{
		if(maxVal < inputArray[i])
		{
			maxVal = inputArray[i];
			maxInd = i;
		}
	}
	//寻找定位标左边界
	for(int i = maxInd; i > 0; i--)
	{
		if(inputArray[i] == 0 && inputArray[i+1] > 0)
		{
			maxIndex.push_back(i);
			break;
		}
			
	}
	//寻找定位标右边界
	for(int i = maxInd; i < length; i++)
	{
		if(inputArray[i] == 0 && inputArray[i-1] > 0)
		{
			maxIndex.push_back(i);
			break;
		}
	}
}

void findMax(const vector<int> & inputArray, vector<int> & maxIndex)
{
	//vector<int> maxIndex(2);
	int length = inputArray.size();
	int maxValue1 = inputArray[0];
	maxIndex[0] = 0;
	for(int i = 1; i < length/2; i++)
	{
		if(maxValue1 < inputArray[i])
		{
			maxValue1 = inputArray[i];
			maxIndex[0] = i;
		}
	}
	int m = maxIndex[0];
	while(inputArray[m]>3)
		m++;
	maxIndex[2] = m;
	int maxValue2 = inputArray[length/2];
	maxIndex[1] = length/2;
	for(int i = length/2; i < length; i++)
	{
		if(maxValue2 < inputArray[i])
		{
			maxValue2 = inputArray[i];
			maxIndex[1] = i;
		}
	}
	int n = maxIndex[1];
	while(inputArray[n]>3)
		n--;
	maxIndex[3] = n;
}

//第一种方法，使用形态学滤波的方式对图像预处理
void process_Pic1()
{

// 图像预处理
// 得到二值化图像，基本能够分离出涂卡位置，为后面处理提供源
// 这个阶段重难点： 1、透视变换的准确性   2、二值化阈值的选取合理性 3、形态学滤波
	Mat rsrcImage = imread("D:\\C++程序联系文件夹（可选择性删除）\\Answer-Card-Recognition\\pic\\result1.jpg",1);
	Mat rGrayImage;
	Mat rBinImage;
	cvtColor(rsrcImage,rGrayImage, CV_BGR2GRAY);  //灰度化
	//CV_THRESH_OTSU参数自动生成阈值，跟第三个参数也就没有关系了。 
    threshold(rGrayImage, rBinImage, 0, 255,  CV_THRESH_BINARY | CV_THRESH_OTSU); //二值化
	///imshow("binary image", rBinImage);
	Mat erodeImage, dilateImage, edImage;
	//定义核  
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));   
	//进行形态学操作 	
	morphologyEx(rBinImage, edImage, MORPH_CLOSE, element,Point(-1,-1),1);
	imshow("先膨胀后腐蚀--闭运算", edImage);

	
// 图像垂直投影 得到定位标位置，并切割
// 图像水平投影  得到答题卡区域位置，并切割
// 此段处理的图像源： edImage
	Mat psrcImage(edImage);  //处理源图像
	vector<int> horizon(psrcImage.rows);
	vector<int> vertical(psrcImage.cols);
	project_Pic(psrcImage,horizon, vertical);
	
	//查找定位标
	vector<int> locIndex;
	findVerticalMax(vertical, locIndex);
	Mat locImage;
	psrcImage(Rect(locIndex[0],0, locIndex[1] - locIndex[0] + 1, psrcImage.rows-1) ).copyTo(locImage);
	imshow("LocateImage", locImage);
	//如果不要求鲁棒性，就直接水平投影定位标就OK
	vector<int> locHorizon(locImage.rows);
	vector<int> locVertical(locImage.cols);
	project_Pic(locImage, locHorizon, locVertical);

	vector<int> locUpIndex, locDownIndex;
	findLocHorizon(locHorizon, locUpIndex, locDownIndex);
// #ifdef 0
	if(locUpIndex.size() == locDownIndex.size())
	{
		Mat tempq;
		psrcImage.copyTo(tempq);
		int lengthq = locUpIndex.size();
		for(int i = 0; i < lengthq ; i++)
		{
			Point ppt1 = Point(0,locUpIndex[i]);
			Point ppt2 = Point(tempq.cols-1, locUpIndex[i]);
			Point ppt3 = Point(0,locDownIndex[i]);
			Point ppt4 = Point(tempq.cols-1, locDownIndex[i]);
			line(tempq, ppt1, ppt2, Scalar(0, 0, 100));
			line(tempq, ppt3, ppt4, Scalar(0, 0, 100));
		}
		imshow("srljsf", tempq);
	}
// #endif
	
	Mat answer;
	picCut(psrcImage,answer, Rect(0,locDownIndex[11], psrcImage.cols-1, locUpIndex[17]-locDownIndex[11]));

	vector<int> ansHorizon(answer.rows);
	vector<int> ansVertical(answer.cols);
	project_Pic(answer, ansHorizon, ansVertical);
	vector<int> ansUpIndex, ansDownIndex;
	findLocHorizon(ansVertical, ansUpIndex, ansDownIndex);
	//imshow("answer", answer);


	for(int i = 0; i < NUMS; i ++)
	{
		Mat tempAns = answer(Rect(ansUpIndex[i], 0, ansDownIndex[i] - ansUpIndex[i] , answer.rows));
		vector<int> tmpHorizon(tempAns.rows);
		vector<int> tmpVertical(tempAns.cols);
		project_Pic(tempAns,tmpHorizon, tmpVertical);
		findAnswer(tmpHorizon, locUpIndex, locDownIndex, Answer[i]);
		cout << i+1 << " : "<< Answer[i] << endl;
	}

}

void findAnswer(const vector<int> & input, const vector<int> locUp, const vector<int> locDown, string & ans)
{
	int length = input.size();
	int m, n;
	for(int i = 1; i < length; i++)
	{
		if(input[i-1] == 0 && input[i] >0)
		{
			m = i;	
		}
		else if(input[i-1] > 0 && input[i] == 0 )
		{
			n = i;
		}
	}
	int mid = (int)(m+n)/2 + locDown[11];
	if(mid >= locUp[13] && mid <= locDown[13])  //A
	{
		ans = 'A';
	}
	else if(mid >= locUp[14] && mid <= locDown[14])
	{
		ans = 'B';
	}
	else if(mid >= locUp[15] && mid <= locDown[15])
	{
		ans = 'C';
	}
	else if(mid >= locUp[16] && mid <= locDown[16]) //D
	{
		ans = 'D';
	}
	else
	{
		ans = "None";
	}
}

void picCut(const Mat & inputArray, Mat & outputArray, const Rect & rectRoi)
{
	outputArray = inputArray(rectRoi);
}

void findLocHorizon(const vector<int> & inputArray, vector<int> & locUp, vector<int> & locDown)
{
	int length = inputArray.size();  //
	int upNums = 0, downNums = 0;
	for(int i = 1; i < length; i++)
	{
		if(inputArray[i-1] == 0 && inputArray[i] > 0)
		{
			locUp.push_back(i);
			upNums ++;
		}
		else if(inputArray[i-1] > 0 && inputArray[i] == 0)
		{
			locDown.push_back(i);
			downNums ++;
		}
	}
}

void on_Change(int,void *)
{	
	threshold(pmidImage, pdstImage, threshold_value, 255, 0);
	imshow("process", pdstImage);
}


//函数作用： 将图像水平和垂直投影，得到 水平和垂直投影的图
//返回： horizon_out 水平投影的图
//		 vertical_out 垂直投影的图
void project_Pic(const Mat & src, vector<int> & horizon_out, vector<int> & vertical_out)
{
	// 对src进行二值化值统计
	//horizontal 水平
	
	int pixelValue = 0;
	for(int i = 0; i < src.rows; i++)   //有多少行
	{
		for(int j = 0; j < src.cols; j++)
		{
			if(src.at<uchar>(i,j) == 0)
				pixelValue++;
		}
		horizon_out[i] = pixelValue;
		pixelValue = 0;
	}

	//vertical 垂直
	//vector<int> vertical_out;
	for(int i = 0; i < src.cols; i++)
	{
		for(int j = 0; j < src.rows; j ++)
		{
			if(src.at<uchar>(j,i) == 0)
				pixelValue++;
		}
		vertical_out[i] = pixelValue;
		pixelValue = 0;
	}
	// show it
	Mat horImage(src.rows, src.cols, CV_8UC1);
	Mat verImage(src.rows, src.cols, CV_8UC1);
	// horizon
	for(int i = 0; i < horImage.rows; i++)
	{
		for(int j = 0; j < horizon_out[i]; j++)
			horImage.at<uchar>(i,j) = 0;  //假设初始化为0；
	}
	for(int i = 0; i < verImage.cols; i ++)
	{
		for(int j = 0; j < vertical_out[i]; j++)
			verImage.at<uchar>(j,i) = 0;
	}
	imshow("hor",horImage);
	imshow("ver",verImage);
	imwrite("3.jpg",horImage);
	imwrite("4.jpg",verImage);
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
					psrcImage = imread("D:\\C++程序联系文件夹（可选择性删除）\\Answer-Card-Recognition\\pic\\result1.jpg",1);
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

