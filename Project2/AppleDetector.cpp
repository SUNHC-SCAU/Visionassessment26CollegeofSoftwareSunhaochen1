#include "AppleDetector.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
using namespace cv;
using namespace std;

Mat Ainput;

Mat Ahsv;
Mat Abinary1;
Mat Abinary2;
Mat Abinary;

Mat Aedge;

Mat Rect_apple;

//static void processImage(const std::string& inputPath, const std::string& outputPath)
void AppleDetector::processAppleImage(const std::string& inputPath)//当你在类定义的花括号外面写函数体时，必须加，否则编译器认为你在定义一个全局函数。
{
	Ainput = imread(inputPath, IMREAD_COLOR);
	if (Ainput.empty())
	{
		cout << "cannot read the picture, please check if the path is true" << inputPath << "\n";
		return;
	}
	Rect_apple = Ainput.clone();//cvtColor/inRange/resize/Canny/bitwise_or 这些函数的 dst 参数是输出参数，它们内部会调用dst.create(src.size(), src.type());← 自动分配（如果尺寸/类型不符就重新分配），但 rectangle/circle/putText/drawContours 不一样——它们是绘制函数，Mat Rect_apple; 只创建了这个描述符，一块像素内存都没分配。没内存rectangle就画不了

	cvtColor(Ainput, Ahsv, COLOR_BGR2HSV);
	inRange(Ahsv, Scalar(0, 50, 160), Scalar(50, 255, 255), Abinary1);
	inRange(Ahsv, Scalar(170, 50, 160), Scalar(180, 255, 255), Abinary2);
	bitwise_or(Abinary1, Abinary2, Abinary);//二值化图像的同时通过设置阈值过滤掉除亮红色外的所有颜色，亮红色设为白色，其他颜色一律设为黑色
	// 4. 形态学操作（处理树叶遮挡和噪点）
		// 使用较大的椭圆核进行闭运算，可以将被树叶遮挡切割开的苹果区域连接起来
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(25, 25));

	// 开运算：去除背景中散落的小红点（噪点）
	morphologyEx(Abinary, Abinary, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	// 闭运算：填补苹果内部的空洞，连接被树叶遮挡的区域
	morphologyEx(Abinary, Abinary, MORPH_CLOSE, kernel);
	Canny(Abinary, Aedge, 50, 150);
	Canny(Abinary, Aedge, 50, 150);//通过Canny提取图像的轮廓

	// 第1步：找轮廓（轮廓 = 一堆点）
	vector<vector<Point>> contours;
	findContours(Abinary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	// 第2步：选面积最大的轮廓（即苹果主体）
	double maxArea = 0;
	int maxIdx = -1;
	for (size_t i = 0; i < contours.size(); i++) {
		double area = contourArea(contours[i]);
		if (area > maxArea) { maxArea = area; maxIdx = (int)i; }
	}

	// 第3步：对该轮廓求外接矩形
	if (maxIdx >= 0) {
		Rect bounding_rect = boundingRect(contours[maxIdx]);   // ← 传【轮廓】才对
		rectangle(Rect_apple, bounding_rect, Scalar(255, 0, 0), 3);
	}

	//Rect bounding_rect = boundingRect(Aedge);
	 // 蓝色矩形框，线宽3
	//imwrite(outputPath, edge);
}
//static void showcaseImage()
void AppleDetector::showcaseAppleImage()
{
	//imshow("input", rinput);
	imshow("Rect_apple", Rect_apple);
	imshow("edge", Aedge);
	waitKey(0);
	destroyAllWindows();
}
