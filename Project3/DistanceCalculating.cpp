#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include "DistanceCalculating.hpp"

// 【新增 2026-10-09】为了调用 RectangleAndCenter::detectArmor()
// 【你原有代码 2026-09-27】原来这里没有这个 include
#include "RectangleAndCenter.hpp"


using namespace cv;
using namespace std;

void DistanceCalculating::processDistanceCalculating(const std::string& ymlPath) {

    cv::Mat cameraMatrix, distCoeffs;
    if (!CameraCalibrator::loadCalibration(ymlPath, cameraMatrix, distCoeffs)) {
        std::cerr << "读取 camera.yml 失败" << std::endl;
        return;
    }

    std::cout << "cameraMatrix:\n" << cameraMatrix << std::endl;
    std::cout << "distCoeffs:\n" << distCoeffs << std::endl;

    // 打开摄像头
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "无法打开摄像头" << std::endl;
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    // 预先计算去畸变映射表
    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(
        cameraMatrix, distCoeffs, cv::Mat(), cameraMatrix,
        frame.size(), CV_16SC2, map1, map2);//CV_16SC2的话map1存x整数, y整数，map2存x小数，y小数
                                            //CV_32FC1另外一种
    // 去畸变后的畸变系数必须全零
    cv::Mat distCoeffsZero = cv::Mat::zeros(1, 5, CV_64F);

    // 装甲板真实尺寸（单位 mm，按实际修改）
    float armorWidth = 15.0f;//135
    float armorHeight = 7.0f;//55
    std::vector<cv::Point3f> objectPoints = {
        cv::Point3f(-armorWidth / 2, -armorHeight / 2, 0),
        cv::Point3f(armorWidth / 2, -armorHeight / 2, 0),
        cv::Point3f(armorWidth / 2,  armorHeight / 2, 0),
        cv::Point3f(-armorWidth / 2,  armorHeight / 2, 0)
    };

    std::cout << "测距开始，按 q 退出" << std::endl;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // 去畸变
        cv::Mat undistorted;
        cv::remap(frame, undistorted, map1, map2, cv::INTER_LINEAR);

        // ============================================
        // 这里替换为你的装甲板识别，得到四个角点
        // 顺序必须：左上、右上、右下、左下
        // ============================================
        std::vector<cv::Point2f> imagePoints;
        bool detected = false;
        // ★【修改 2026-10-09】把注释打开，真正调用检测
        // 改前: // detected = detectArmor(undistorted, imagePoints);
        //       （detectArmor 当时还没写，所以一直注释着 → detected 恒为 false
        //         → solvePnP 从不执行 → 距离从不输出）
        // 改后: 调用 RectangleAndCenter::detectArmor()
        //       它输出 4 个【已排序】角点(左上→右上→右下→左下)，
        //       顺序和上面 objectPoints 一一对应
        detected = RectangleAndCenter::detectArmor(undistorted, imagePoints);

        if (detected && imagePoints.size() == 4) {
            cv::Mat rvec, tvec;
            bool ok = cv::solvePnP(
                objectPoints, imagePoints,
                cameraMatrix, distCoeffsZero,   // 注意：全零
                rvec, tvec);

            if (ok) {
                double distance = cv::norm(tvec);   // mm
                std::cout << "距离: " << distance << " mm" << std::endl;

                cv::putText(undistorted, cv::format("%.1f mm", distance),
                    cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX,
                    1.0, cv::Scalar(0, 255, 0), 2);
            }
        }

        cv::imshow("undistorted", undistorted);
        if (cv::waitKey(30) == 'q') break;
    }
    cv::destroyAllWindows();

}










/*

{
    // 1. 填入你标定得到的相机参数
    Mat cameraMatrix = (Mat_<double>(3, 3) <<
        2335.8692, 0.0, 719.25127,
        0.0, 2333.77331, 544.18238,
        0.0, 0.0, 1.0
        );

    Mat distCoeffs = (Mat_<double>(1, 5) <<
        -0.105848, 0.096276, -0.001701, 0.000563, 0.000000
        );

    // 2. 定义装甲板的真实物理尺寸（单位：毫米，根据实际装甲板修改！）
    // 假设装甲板宽 135mm，高 55mm
    float armorWidth = 135.0f;
    float armorHeight = 55.0f;

    // 定义装甲板四个角点在真实世界中的 3D 坐标 (Z=0)
    // 顺序：左上、右上、右下、左下 (必须与图像点顺序一致)
    vector<Point3f> objectPoints = {
        Point3f(-armorWidth / 2, -armorHeight / 2, 0),
        Point3f(armorWidth / 2, -armorHeight / 2, 0),
        Point3f(armorWidth / 2,  armorHeight / 2, 0),
        Point3f(-armorWidth / 2,  armorHeight / 2, 0)
    };

    // 3. 假设这是你从图像中识别到的四个角点（像素坐标）
    vector<Point2f> imagePoints = {
        Point2f(600, 500), // 左上
        Point2f(800, 500), // 右上
        Point2f(800, 600), // 右下
        Point2f(600, 600)  // 左下
    };

    // 4. 计算距离
    Mat rvec, tvec;
    bool success = solvePnP(
        objectPoints,
        imagePoints,
        cameraMatrix,
        distCoeffs,
        rvec,
        tvec
    );

    if (success) {
        // tvec 是平移向量，单位是毫米
        double distance = norm(tvec);
        cout << "距离: " << distance << " mm" << endl;
        cout << "距离: " << distance / 1000.0 << " m" << endl;
    }
    else {
        cout << "solvePnP 失败" << endl;
    }

    return ;
}*/