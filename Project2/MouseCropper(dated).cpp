//#include "MouseCropper.hpp"
//#include <iostream>
//#include <opencv2/opencv.hpp>
//#include <string>
//using namespace cv;
//using namespace std;
//
//// 全局变量：记录鼠标状态和选区
//Mat img, imgCopy;
//Point startPoint, endPoint;
//bool isDrawing = false;
//
//
//// 保存框选区域
//Rect roi(0, 0,
//    0, 0);
//
//
//// 鼠标回调函数
//void onMouse(int event, int x, int y, int flags, void* userdata) {
//    if (event == EVENT_LBUTTONDOWN) {
//        // 左键按下，记录起点
//        isDrawing = true;
//        startPoint = Point(x, y);
//        endPoint = startPoint;
//    }
//    else if (event == EVENT_MOUSEMOVE && isDrawing) {
//        // 拖动时实时显示矩形
//        endPoint = Point(x, y);
//        imgCopy = img.clone();
//        rectangle(imgCopy, startPoint, endPoint, Scalar(0, 0, 255), 2);
//        imshow("Image", imgCopy);
//    }
//    else if (event == EVENT_LBUTTONUP) {
//        // 左键抬起，结束框选
//        isDrawing = false;
//        endPoint = Point(x, y);
//        imgCopy = img.clone();
//        rectangle(imgCopy, startPoint, endPoint, Scalar(0, 255, 0), 2);
//        imshow("Image", imgCopy);
//        //imshow("CroppedImage", imgCopy);
//        //destroyWindow("Image");
//
//        // 计算并输出框中心像素点坐标
//        int center_x = startPoint.x + roi.width / 2;
//        int center_y = startPoint.y + roi.height / 2;
//        cout << "[INFO] 框中心像素点坐标: (" << center_x << ", " << center_y << ")" << endl;
//
//
//    }
//    imgCopy = img.clone();
//    // 获取当前鼠标位置的像素值（注意 OpenCV 是 BGR 顺序）
//    Vec3b pixel = img.at<Vec3b>(y, x);
//    int b = pixel[0];
//    int g = pixel[1];
//    int r = pixel[2];
//
//    // 格式化信息字符串
//    char info[128];
//    snprintf(info, sizeof(info), "Pos: (%d, %d) RGB: (%d, %d, %d)", x, y, r, g, b);
//
//    // 在图像上显示信息（为了不超出边界，x 和 y 需要做偏移处理）
//    int text_x = std::min(x + 10, img.cols - 250);
//    int text_y = std::max(y - 10, 30);
//    putText(imgCopy, info, Point(text_x, text_y), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 255), 2);
//
//}
//
//void MouseCropper::processCatImage(const std::string& inputPath, const std::string& outputPath)
//{
//    // 读取图片，路径改成你自己的
//    img = imread(inputPath);
//    if (img.empty()) {
//        cout << "图片读取失败" << endl;
//        return ;
//    }
//
//    namedWindow("Image", WINDOW_NORMAL);
//
//
//
//    imshow("Image", img);
//
//    // 注册鼠标回调
//    setMouseCallback("Image", onMouse);
//
//    cout << "用鼠标左键拖拽框选猫咪，按 's' 保存，按 'q' 退出" << endl;
//
//
//
//
//
//    while (true) {
//        char key = waitKey(10);
//        if (key == 's') {
//
//            if (roi.width > 0 && roi.height > 0) {
//                // 保存框选区域
//                Rect roi(min(startPoint.x, endPoint.x), min(startPoint.y, endPoint.y),
//                    abs(endPoint.x - startPoint.x), abs(endPoint.y - startPoint.y));
//                Mat cropped = img(roi);
//                imwrite(outputPath, cropped);
//                cout << "保存成功" << endl;
//
//            }
//            else {
//                cout << "选区无效，请重新框选" << endl;
//            }
//        }
//        else if (key == 'q') {
//            break;
//        }
//    }
//
//    destroyAllWindows();
//    return ;
//}
