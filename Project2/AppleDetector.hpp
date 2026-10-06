#ifndef APPLE_DETECTOR_HPP
#define APPLE_DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <string>

class AppleDetector {
public:

    static void processAppleImage(const std::string& inputPath);
    static void showcaseAppleImage();


};

#endif