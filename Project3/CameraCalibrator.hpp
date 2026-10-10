#ifndef CAMERA_CALIBRATOR_HPP
#define CAMERA_CALIBRATOR_HPP

#include <opencv2/opencv.hpp>
#include <string>

class DistanceCalculating;

class CameraCalibrator {
private:
    std::string inputPath_;
    friend class DistanceCalculating;//因为DistanceCalculating类里面的processDistanceCalculating测距时需要访问标定最终存在inputPath_里面的标定参数，所以只能用友元给DistanceCalculating类开个后门啦>_<
public:

    // ★(已修改) 这里原来是对 CameraImageSave 的【内联定义】（只写 inputPath_ = inputPath;）
    // 而 CameraCalibrator.cpp 第 15 行又有一份【完整定义】（含摄像头采集循环）
    // 改前: void CameraImageSave(const std::string& inputPath) {
    //           inputPath_ = inputPath;
    //       }
    // 结果: 重复定义 -> error C2084: function ... already has a body
    // 改后: 只留声明，唯一实现在 .cpp 中（那里同时负责赋值 inputPath_ 和采集图片）
    void CameraImageSave(const std::string& inputPath);
    void processCalibrating(const std::string& outputPath);

    static bool loadCalibration(const std::string& ymlPath,
        cv::Mat& cameraMatrix,
        cv::Mat& distCoeffs);

    // ★(已修改) 标定文件名的唯一来源，避免多处硬编码 "camera.yml" 造成不一致
    static const char* calibrationFileName() { return "camera.yml"; }

    // ★(已修改) 由【目录】得到 camera.yml 的【完整文件路径】
    // 改前：调用方自己写 outputPath + "/camera.yml"，容易漏拼、或把目录当文件传
    // 改后：统一走这个函数，保证"写"和"读"用的是同一个路径
    static std::string buildCalibrationPath(const std::string& outputDir) {
        return outputDir + "/" + calibrationFileName();
    }


};

#endif