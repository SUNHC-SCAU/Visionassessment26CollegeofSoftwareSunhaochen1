#ifndef RECTANGLE_AND_CENTER_HPP
#define RECTANGLE_AND_CENTER_HPP

#include <opencv2/opencv.hpp>
#include <string>

// ============================================================
// 【新增 2026-10-09】以下 include 和声明都是今天新加的
// 【你原有代码 2026-09-27】只有 #include <opencv2/opencv.hpp> 和 <string>
//   以及 processArmorVideo() 这一个函数声明
// ============================================================
#include <vector>      // 【新增 2026-10-09】detectArmor 用到 std::vector

class RectangleAndCenter {
public:

    static void processArmorVideo();

    // ------------------------------------------------------------
    // 【新增 2026-10-09】装甲板检测接口
    // 作用: 从一张图里找出装甲板, 输出 4 个【已排好序】的角点像素坐标
    //       顺序固定为: 左上 -> 右上 -> 右下 -> 左下
    //       必须和 DistanceCalculating.cpp 里 objectPoints 的顺序一一对应,
    //       否则 solvePnP 解出的位姿是错误的。
    // 返回: true = 检测到, imagePoints 被填成 4 个点;
    //       false = 没检测到, imagePoints 被清空。
    // 【你原有代码 2026-09-27】没有这个函数, 是今天新加的
    // ------------------------------------------------------------
    static bool detectArmor(const cv::Mat& frame,
        std::vector<cv::Point2f>& imagePoints);

    // ------------------------------------------------------------
    // 【新增 2026-10-09】把 4 个角点排成 左上->右上->右下->左下
    // 单独拆出来是为了让 DistanceCalculating 那边也能复用同一个排序规则,
    // 保证两个模块对"第 1/2/3/4 个点"的理解完全一致。
    // ------------------------------------------------------------
    static void orderCorners(std::vector<cv::Point2f>& corners);



};

#endif