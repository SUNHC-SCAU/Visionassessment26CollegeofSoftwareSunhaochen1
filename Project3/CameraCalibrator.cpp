#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "CameraCalibrator.hpp"


using namespace cv;
using namespace std;





void CameraCalibrator::CameraImageSave(const std::string& inputPath) {
    // ★(已修改) 补上对 inputPath_ 的赋值
    // 改前: （无此行，赋值原本写在 .hpp 的内联定义里）
    // 后果: 删掉 .hpp 内联定义后 inputPath_ 永远为空，
    //       processCalibrating 里的 inputPath_ + "/*.jpg" 会 glob 不到任何图片
    // 改后: 在这里赋值，保证 processCalibrating 能找到标定图片目录
    inputPath_ = inputPath;
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "无法打开摄像头" << endl;
        return ;
    }

    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    Mat frame;
    int idx = 0;
    cout << "按 s 保存图片，按 ESC 退出" << endl;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        imshow("camera", frame);
        int key = waitKey(30);

        if (key == 's') {
            string name = inputPath + "/img_" + to_string(idx++) + ".jpg";//图片存到inputPath
            imwrite(name, frame);
            cout << "saved: " << name << endl;
        }
        else if (key == 27) {
            break;
        }
    }
    return ;
}




void CameraCalibrator::processCalibrating(const std::string& outputPath) {
    // 重要：这里填“内角点”数量
    // 如果 PDF 的 8x11 是方格数，内角点为 7x10
    // 如果 8x11 是内角点数，则改为 8, 11
    const int boardCols = 7;   // 内角点列数
    const int boardRows = 10;  // 内角点行数
    const float squareSize = 15.0f; // 方格实际边长，单位 mm

    Size patternSize(boardCols, boardRows);

    // 构造棋盘格三维坐标，Z=0
    vector<Point3f> obj;
    for (int r = 0; r < boardRows; ++r) {
        for (int c = 0; c < boardCols; ++c) {
            obj.push_back(Point3f(c * squareSize, r * squareSize, 0));
        }
    }

    // 读取 inputPath 目录下图片
    vector<String> files;
    std::string inputPath1 = inputPath_ + "/*.jpg";
    glob(inputPath1, files, false);
    vector<String> pngs;
    std::string inputPath2 = inputPath_ + "/*.png";
    glob(inputPath2, pngs, false);
    files.insert(files.end(), pngs.begin(), pngs.end());

    if (files.empty()) {
        cerr << "你定的目录下没有图片" << endl;
        return ;
    }

    vector<vector<Point3f>> objectPoints;
    vector<vector<Point2f>> imagePoints;

    Size imageSize;
    Mat gray;
    int okCount = 0;

    for (const auto& f : files) {
        Mat img = imread(f);
        if (img.empty()) continue;

        cvtColor(img, gray, COLOR_BGR2GRAY);

        if (imageSize.width == 0) {
            imageSize = gray.size();
        }

        if (gray.size() != imageSize) {
            cerr << "跳过尺寸不一致图片: " << f << endl;
            continue;
        }

        vector<Point2f> corners;
        bool found = findChessboardCorners(
            gray,
            patternSize,
            corners,
            CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE
        );

        if (found) {
            // 亚像素优化
            cornerSubPix(
                gray,
                corners,
                Size(11, 11),
                Size(-1, -1),
                TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 30, 0.001)
            );

            imagePoints.push_back(corners);
            objectPoints.push_back(obj);
            okCount++;

            drawChessboardCorners(img, patternSize, corners, found);
            imwrite("corner_" + to_string(okCount) + ".jpg", img);

            cout << "OK: " << f << endl;
        }
        else {
            cout << "FAIL: " << f << endl;
        }
    }

    if (okCount < 10) {
        cerr << "有效标定图太少: " << okCount << "，建议至少 15 张" << endl;
        return ;
    }

    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);//先创建 3×3 单位矩阵，准备接收相机内参
    Mat distCoeffs = Mat::zeros(5, 1, CV_64F);
    vector<Mat> rvecs, tvecs;

    double rms = calibrateCamera(
        objectPoints,
        imagePoints,
        imageSize,
        cameraMatrix,
        distCoeffs,
        rvecs,
        tvecs
    );

    cout << "RMS 重投影误差: " << rms << " pixels" << endl;
    cout << "cameraMatrix:\n" << cameraMatrix << endl;
    cout << "distCoeffs:\n" << distCoeffs << endl;

    // 保存标定参数
    //FileStorage fs("camera.yml", FileStorage::WRITE);//现在改用变量存路径
    // ★(已修改) 用统一函数生成路径，不再手写 "/camera.yml"
    // 改前: std::string cameraymloutputPath = outputPath + "/camera.yml";
    // 改后: 由 buildCalibrationPath() 统一拼接，和调用方读取时用的是同一个路径
    std::string cameraymloutputPath = buildCalibrationPath(outputPath);
    FileStorage fs(cameraymloutputPath, FileStorage::WRITE);
    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;
    fs << "rms" << rms;
    fs << "board_cols" << boardCols;
    fs << "board_rows" << boardRows;
    fs << "square_size_mm" << squareSize;
    fs.release();

    cout << "已保存 camera.yml" << endl;
    return ;
}



bool CameraCalibrator::loadCalibration(const std::string& ymlPath,
    cv::Mat& cameraMatrix,
    cv::Mat& distCoeffs) {
    cv::FileStorage fs(ymlPath, cv::FileStorage::READ);
    if (!fs.isOpened()) return false;

    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    fs.release();

    if (cameraMatrix.empty() || distCoeffs.empty()) return false;

    // 确保类型正确
    cameraMatrix.convertTo(cameraMatrix, CV_64F);
    distCoeffs.convertTo(distCoeffs, CV_64F);
    return true;
}