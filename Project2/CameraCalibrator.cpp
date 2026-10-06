#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "CameraCalibrator.hpp"

using namespace cv;
using namespace std;

void CameraCalibrator::Calibrating(const std::string& inputPath) {
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

    // 读取 images 目录下图片
    vector<String> files;
    glob(inputPath, files, false);
    /*glob("images/*.jpg", files, false);
    vector<String> pngs;
    glob("images/*.png", pngs, false);
    files.insert(files.end(), pngs.begin(), pngs.end());*/

    if (files.empty()) {
        cerr << "images 目录下没有图片" << endl;
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

    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
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
    /*FileStorage fs("camera.yml", FileStorage::WRITE);
    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;
    fs << "rms" << rms;
    fs << "board_cols" << boardCols;
    fs << "board_rows" << boardRows;
    fs << "square_size_mm" << squareSize;
    fs.release();

    cout << "已保存 camera.yml" << endl;*/
    return ;
}