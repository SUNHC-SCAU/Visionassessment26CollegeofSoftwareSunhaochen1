#ifndef MOUSE_CROPPER_HPP
#define MOUSE_CROPPER_HPP

#include <opencv2/opencv.hpp>
#include <string>

class MouseCropper {
public:

    static void processCatImage(const std::string& inputPath, const std::string& outputPath);


};

#endif 