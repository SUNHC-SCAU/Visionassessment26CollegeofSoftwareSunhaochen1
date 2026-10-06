#ifndef COLOR_SEGMENTATION_HPP
#define COLOR_SEGMENTATION_HPP

#include <opencv2/opencv.hpp>
#include <string>

class ImageProcessor {
public:
    /**
     * @brief 处理图像的主函数
     * @param inputPath 原图路径
     * @param outputPath 边缘图保存路径
     */
    static void processImage(const std::string& inputPath, const std::string& outputPath);
    static void showcaseImage();
    

};

#endif // COLOR_SEGMENTATION_HPP