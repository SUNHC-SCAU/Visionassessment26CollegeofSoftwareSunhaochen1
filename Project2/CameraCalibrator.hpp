#ifndef CAMERA_CALIBRATOR_HPP
#define CAMERA_CALIBRATOR_HPP

#include <opencv2/opencv.hpp>
#include <string>

class CameraCalibrator {
public:

    static void Calibrating(const std::string& inputPath);


};

#endif