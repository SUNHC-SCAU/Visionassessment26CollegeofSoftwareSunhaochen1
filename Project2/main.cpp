#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

#include "ColorSegmentation.hpp"
#include "CameraRecorder.hpp"
#include "MouseCropper.hpp"
#include "AppleDetector.hpp"
#include "CameraCalibrator.hpp"
//#include "ColorSegmentation.cpp"
using namespace cv;
using namespace std;

void printMenu() {
    cout << "\n================ 计算机视觉项目汇总 (基础+应用) ================\n";
    cout << "0. 退出程序\n";
    cout << "======================================================\n";
    cout << "请选择要运行的功能 [0-5]: ";
}

int main() {
    int choice = -1;
    string inputPath;
    string outputPath;

    while (true) {
        printMenu();
        cin >> choice;

        // 处理输入错误
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1024, '\n');
            cout << "输入无效，请输入数字！\n";
            continue;
        }

        cout << "\n";
        destroyAllWindows(); // 每次执行前清理残留窗口

        switch (choice) {
        case 0:
            cout << "退出程序。\n";
            return 0;

            case 1: // 基础题1：色彩分割
                cout << "请输入原图路径 (例如 test.jpg): ";
                cin >> inputPath;
                cout << "请输入保存边缘图的路径 (例如 edges.jpg): ";
                cin >> outputPath;
                ImageProcessor::processImage(inputPath, outputPath);
                ImageProcessor::showcaseImage();
                break;

            case 2:
                cout << "请输入保存视频的路径 (例如 video.avi): ";
                cin >> outputPath;
                VideoProcessor::processVideo(outputPath);
                break;
            case 3:
                cout << "请输入猫猫图路径 (例如 cat.jpg): ";
                cin >> inputPath;
                cout << "请输入保存猫猫图的路径 (例如 catroi.jpg): ";
                cin >> outputPath;
                MouseCropper::processCatImage(inputPath, outputPath);
                break;

            case 4:
                cout << "请输入苹果图片路径 (例如 apple.png): ";
                cin >> inputPath;
                AppleDetector::processAppleImage(inputPath);
                AppleDetector::showcaseAppleImage();
                break;

            case 5:
                cout << "请输入个角度拍摄的棋盘图片（以供标定）路径 (例如 chessboard.png): ";
                cin >> inputPath;
                CameraCalibrator::Calibrating(inputPath);
                break;


        default:
            cout << "无效的选择，请重新输入！\n";
            break;
        }

        // 暂停，防止菜单瞬间刷新，同时让用户查看OpenCV窗口输出
        cout << "\n操作完成，按回车键返回主菜单...";
        cin.ignore();
        cin.get();
    }

    return 0;
}