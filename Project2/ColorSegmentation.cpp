#include "ColorSegmentation.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
using namespace cv;
using namespace std;

Mat input;
Mat rinput;
Mat hsv;
Mat binary1;
Mat binary2;
Mat binary;
Mat rbinary;
Mat edge;
Mat edge1;
Mat redge;

//static void processImage(const std::string& inputPath, const std::string& outputPath)
void ImageProcessor::processImage(const std::string& inputPath, const std::string& outputPath)//当你在类定义的花括号外面写函数体时，必须加，否则编译器认为你在定义一个全局函数。
{
	input = imread(inputPath, IMREAD_COLOR);
	if (input.empty())
	{
		cout << "cannot read the picture, please check if the path is true" << inputPath <<"\n";
		return ;
	}

	cvtColor(input, hsv, COLOR_BGR2HSV);
	inRange(hsv, Scalar(0, 160, 160), Scalar(10, 255, 255), binary1);
	inRange(hsv, Scalar(170, 160, 160), Scalar(180, 255, 255), binary2);
	bitwise_or(binary1, binary2, binary);//二值化图像的同时通过设置阈值过滤掉除亮红色外的所有颜色，亮红色设为白色，其他颜色一律设为黑色
	Canny(binary, edge, 50, 150);
	Canny(binary, edge, 50, 150);//通过Canny提取图像的轮廓
	Mat color_layer(input.size(), input.type(), Scalar(200, 150, 80));//创建一个color_layer图像大小类型和input图像一样，设置color_layer为全部浅蓝色
	color_layer.copyTo(edge1, edge);//color_layer图像中位于edge图像的白色区域位置的像素复制给edge1
	imwrite(outputPath, edge1);
}
//static void showcaseImage()
void ImageProcessor::showcaseImage()
{
	resize(input, rinput, cv::Size(1280, 720));
	resize(binary, rbinary, cv::Size(1280, 720));
	resize(edge1, redge, cv::Size(1280, 720));//把要显示的图像都设置为1280*720
	imshow("input", rinput);
	imshow("binary", rbinary);
	imshow("edge", redge);
	waitKey(0);
	destroyAllWindows();
}
