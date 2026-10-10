#ifndef DISTANCE_CALCULATING_HPP
#define DISTANCE_CALCULATING_HPP

#include <opencv2/opencv.hpp>
#include <string>
#include "CameraCalibrator.hpp"

class DistanceCalculating {
public:

    static void processDistanceCalculating(const std::string& ymlPath);



};

#endif