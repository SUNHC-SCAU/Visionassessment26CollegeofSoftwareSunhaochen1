#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "CameraCalibrator.hpp"
#include "RectangleAndCenter.hpp"
#include "DistanceCalculating.hpp"



using namespace cv;
using namespace std;



// ============================================================
// ★(已修改) 本文件路径逻辑统一说明：
//   ymlPath : camera.yml 的【完整文件路径】（loadCalibration / 测距需要）
//   outDir  : 存放 camera.yml 的【目录】（processCalibrating 需要）
//   改前：两者混用一个 outputPath，导致第54行把目录当文件读、第68行传空串
//   改后：变量名区分语义，路径拼接统一走 buildCalibrationPath()
// ============================================================
int main() {
    std::string inputPath;
    std::string ymlPath;      // ★(已修改) camera.yml 的完整文件路径

    // 改前: std::string outputPath;
    // 改后: 改名并明确语义 —— 这是"目录"，不是"文件路径"
    std::string outDir;       // ★(已修改) 存放 camera.yml 的目录

    // ============================================
    // 第一步：先问 camera.yml 路径
    // ============================================
    std::cout << "请输入 camera.yml 的路径 (例如 camera.yml): ";
    std::cin >> ymlPath;

    cv::Mat cameraMatrix, distCoeffs;

    // ============================================
    // 第二步：检查这个路径下有没有可用的 camera.yml
    // ============================================
    if (!CameraCalibrator::loadCalibration(ymlPath, cameraMatrix, distCoeffs)) {
        std::cout << "未找到有效的 " << ymlPath << "，需要先进行标定。" << std::endl;

        // 问标定图片存哪里
        std::cout << "请输入你想存入的标定图片目录 (例如 images): ";
        std::cin >> inputPath;

        CameraCalibrator A;

        // 采集图片
        std::cout << "即将打开摄像头，按 s 保存图片，按 ESC 结束采集。" << std::endl;
        A.CameraImageSave(inputPath);

        // ★(已修改) 这里让用户输入的应该是【目录】，不是文件路径
        // 改前: cout << "请输入你想存入的camera.yml路径 (例如 camera.yml): ";
        //       cin >> outputPath;
        //       A.processCalibrating(outputPath);
        // 改后: 提示语说明是"目录"，变量用 outDir，避免和 ymlPath 混淆
        //       （注意：变量名不要加引号，加了就变成字符串字面量了）
        cout << "请输入你想存入 camera.yml 的目录 (例如 . ): ";
        cin >> outDir;
        A.processCalibrating(outDir);
        // 标定，生成 camera.yml 到 outDir 目录下

        // ★(已修改) Bug1修复：必须拼上文件名再读，不能把【目录】当【文件】传
        // 改前: if (!CameraCalibrator::loadCalibration(outputPath, cameraMatrix, distCoeffs)) {
        //           std::cerr << "标定后仍无法读取 " << outputPath << "，程序退出。" << std::endl;
        // 改后: 用 buildCalibrationPath(outDir) 得到和上面"写入"时完全相同的路径
        ymlPath = CameraCalibrator::buildCalibrationPath(outDir);
        if (!CameraCalibrator::loadCalibration(ymlPath, cameraMatrix, distCoeffs)) {
            std::cerr << "标定后仍无法读取 " << ymlPath << "，程序退出。" << std::endl;
            return -1;
        }

        std::cout << "标定完成，继续测距。" << std::endl;
    }
    else {
        // ★(已修改) Bug2修复：这个分支里 ymlPath 已经是有效的完整路径，
        // 不需要再给任何变量赋值（改前 outputPath 在这里始终是空串 ""）
        std::cout << "已检测到有效的 " << ymlPath << "，直接使用标定参数。" << std::endl;
    }

    // ============================================
    // 第三步：测距
    // ============================================
    // ★(已修改) Bug2修复：传 ymlPath（完整文件路径），不再传 outputPath（可能是空串）
    // 改前: DistanceCalculating::processDistanceCalculating(outputPath);
    // 改后: 它的形参名本来就是 ymlPath，说明要的就是文件路径
    DistanceCalculating::processDistanceCalculating(ymlPath);

    // ============================================
    // 第四步：装甲板视频处理
    // ============================================





   // RectangleAndCenter::processArmorVideo();

    return 0;
}