// ================================================================
// RectangleAndCenter.cpp
//
// 【新增 2026-10-09】整个文件都是今天新写的。
// 【你原有代码】本文件在此之前是 0 字节空文件（修改时间 2026-09-27 20:05），
//   但 main.cpp 里调用了 RectangleAndCenter::processArmorVideo()，
//   所以链接时报错:
//     error LNK2019: 无法解析的外部符号
//     "public: static void __cdecl RectangleAndCenter::processArmorVideo(void)"
//   今天把实现补上，顺便新增 detectArmor() 供测距模块使用。
// ================================================================

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include "RectangleAndCenter.hpp"

using namespace cv;
using namespace std;

// ================================================================
// 【新增 2026-10-09】内部小工具：按角度把 4 个角点排成环状顺序（这一步只做了将相邻点逆时针排出来（比如[左下, 右下, 右上, 左上]），具体定出左上角是哪个点还需下一个函数orderCorners）
//   做法: 先算出 4 个点的中心，再算每个点相对中心的极角，按极角升序排。
//   目的: RotatedRect::points() 返回的 4 个点起始位置和方向都不固定，
//         必须先排成固定顺序，后面才能可靠地找出"左上"。
//   注意: 用 atan2(-dy, dx) 而不是 atan2(dy, dx)。
//         图像坐标里 y 轴向下，取负号后角度就变成逆时针增长，
//         这样"左上附近"对应的角度最小，便于后续定位。
// ================================================================
static void sortByAngleAroundCenter(vector<Point2f>& pts) {
    Point2f c(0.f, 0.f);
    for (int i = 0; i < 4; ++i) c += pts[i];
    c.x /= 4.f; c.y /= 4.f;//c是中心坐标

    vector<pair<double, Point2f>> tmp;
    for (int i = 0; i < 4; ++i) {
        double ang = atan2(-(double)(pts[i].y - c.y), (double)(pts[i].x - c.x)); // (-pi, pi]
        tmp.push_back(make_pair(ang, pts[i]));
    }
    sort(tmp.begin(), tmp.end(),
        [](const pair<double, Point2f>& a, const pair<double, Point2f>& b) {
            return a.first < b.first;
        });
    for (int i = 0; i < 4; ++i) pts[i] = tmp[i].second;
}

// ================================================================
// 【新增 2026-10-09】把 4 个角点规范成固定顺序: 左上 -> 右上 -> 右下 -> 左下
//
// 为什么要这个顺序:
//   DistanceCalculating.cpp 里的 objectPoints 就是按这个顺序给的:
//     Point3f(-w/2, -h/2, 0)  左上
//     Point3f( w/2, -h/2, 0)  右上
//     Point3f( w/2,  h/2, 0)  右下
//     Point3f(-w/2,  h/2, 0)  左下
//   solvePnP 要求两组点【一一对应】，顺序错了解出的位姿就是错的（距离会离谱）。
//
// 做法（三步）:
//   ① 先按角度排成环状顺序（上面那个函数）
//   ② 在环里找 y 最小（最靠上）的点作为锚点 = 左上。
//      y 相同时取 x 更小的。上边缘两个角点里，左上的 y 通常 <= 右上的 y，
//      所以这个规则能把左上唯一确定下来。
//   ③ 以左上为起点重新排列成数组，然后按【相对左上】的角度排序:
//        角度 = atan2(-(p.y - TL.y), p.x - TL.x)
//        右上 → 约 45 度     右下 → 约 135 度     左下 → 约 -135 度
//      升序排完正好是: 右上, 右下, 左下，拼在左上后面就是目标顺序。
//
// 局限性（写在这里备注，不是 bug）:
//   装甲板接近竖直（旋转超过约 60 度）时，"最靠上的点"不再是物理上的左上角，
//   此时 solvePnP 的角点对应关系会错位。本项目装甲板大致正对相机，够用。
//   要彻底解决需要 PnP 的 IPPE 方案或按灯条方向建模，属于后续优化。
// ================================================================
void RectangleAndCenter::orderCorners(vector<Point2f>& corners) {
    if (corners.size() != 4) return;

    // ① 先排成环状顺序
    sortByAngleAroundCenter(corners);

    // ② 找最靠上的点（y 最小；y 相同时取 x 更小）作为左上锚点
    int topIdx = 0;
    for (int i = 1; i < 4; ++i) {
        if (corners[i].y < corners[topIdx].y - 1e-6f) {
            topIdx = i;
        }
        else if (fabs(corners[i].y - corners[topIdx].y) <= 1e-6f &&
            corners[i].x < corners[topIdx].x) {
            topIdx = i;
        }
    }

    // 把锚点转到数组第 0 位
    vector<Point2f> ring;
    for (int i = 0; i < 4; ++i) ring.push_back(corners[(topIdx + i) % 4]);

    // ③ 以左上为起点，按相对角度排序（跳过第 0 个，它就是起点）
    const Point2f TL = ring[0];
    vector<pair<double, Point2f>> rest;
    for (int i = 1; i < 4; ++i) {
        double ang = atan2(-(double)(ring[i].y - TL.y), (double)(ring[i].x - TL.x));
        rest.push_back(make_pair(ang, ring[i]));
    }
    sort(rest.begin(), rest.end(),
        [](const pair<double, Point2f>& a, const pair<double, Point2f>& b) {
            return a.first < b.first;
        });

    corners.clear();
    corners.push_back(TL);
    for (size_t i = 0; i < rest.size(); ++i) corners.push_back(rest[i].second);
    // 现在 corners = [左上, 右上, 右下, 左下]
}

// ================================================================
// 【新增 2026-10-09】装甲板检测
//   思路: HSV 阈值提红色/蓝色 -> 形态学 -> 找轮廓 -> minAreaRect
//         -> 用面积和长宽比筛掉不像装甲板的区域 -> 顶点排序后输出
//
//   为什么用 minAreaRect 而不是 boundingRect:
//     boundingRect 给的是【轴对齐】外接正矩形，装甲板一倾斜，
//     它的 4 个角点就落在"外接大矩形"上，不在装甲板本体上，
//     拿去 solvePnP 会算出错误距离。minAreaRect 给的是【最小面积旋转矩形】，
//     4 个顶点才是装甲板真实的 4 个角，并且能拿到 angle。
//
//   返回: 检测到就填 4 个点([左上,右上,右下,左下])并返回 true；
//         没检测到就清空 imagePoints 并返回 false。
// ================================================================
bool RectangleAndCenter::detectArmor(const Mat& frame, vector<Point2f>& imagePoints) {
    imagePoints.clear();
    if (frame.empty()) return false;

    // ---------- ① BGR 转 HSV，分别提红色和蓝色 ----------
    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    // 红色在 HSV 里跨 0 度，必须用两段区间
    Mat maskRed1, maskRed2, maskRed, maskBlue, mask;
    inRange(hsv, Scalar(0, 80, 60), Scalar(10, 255, 255), maskRed1);
    inRange(hsv, Scalar(170, 80, 60), Scalar(180, 255, 255), maskRed2);
    bitwise_or(maskRed1, maskRed2, maskRed);
    // 蓝色一段就够
    inRange(hsv, Scalar(100, 80, 60), Scalar(130, 255, 255), maskBlue);
    bitwise_or(maskRed, maskBlue, mask);

    // ---------- ② 形态学: 开运算去噪点, 闭运算把断裂灯条连起来 ----------
    morphologyEx(mask, mask, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    morphologyEx(mask, mask, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(9, 9)));

    // ---------- ③ 找轮廓 ----------
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // ---------- ④ 挑最大的那个合格轮廓 ----------
    double bestArea = 0.0;
    RotatedRect bestRR;
    bool foundOne = false;

    for (size_t i = 0; i < contours.size(); ++i) {
        double area = contourArea(contours[i]);
        if (area < 300.0) continue;            // 太小，当噪声

        RotatedRect rr = minAreaRect(contours[i]);

        // 用长宽比过滤: 装甲板是细长条形，长边/短边 一般在 1.5~6 之间
        double w = rr.size.width, h = rr.size.height;
        double longest = max(w, h), shortest = min(w, h);
        if (shortest < 1.0) continue;
        double ratio = longest / shortest;
        if (ratio < 1.5 || ratio > 6.0) continue;

        if (area > bestArea) {
            bestArea = area;
            bestRR = rr;
            foundOne = true;
        }
    }

    if (!foundOne) return false;

    // ---------- ⑤ 输出 4 个顶点，并按 左上->右上->右下->左下 排序 ----------
    Point2f pts[4];
    bestRR.points(pts);//findContours输出一堆轮廓点，minAreaRect从这些点中拟合出最小矩形，points是RotatedRect类的成员函数，用来返回矩形的 4 个顶点
    imagePoints.assign(pts, pts + 4);
    orderCorners(imagePoints);        // 复用上面那个排序函数，保证和 objectPoints 对应

    return imagePoints.size() == 4;
}

// ================================================================
// 【你原有代码 2026-09-27】processArmorVideo() 的声明是你原来写的
// 【新增 2026-10-09】下面是今天补的实现（原来这个 .cpp 是空的）
//
// 作用: 打开摄像头（打不开就用 static/shapes.png 兜底），
//       逐帧调用 detectArmor()，把装甲板外接框和中心点画在画面上，
//       并在控制台输出 4 个顶点坐标与中心坐标。按 q 或 ESC 退出。
// ================================================================
void RectangleAndCenter::processArmorVideo() {
    // ---------- ① 打开视频源: 优先摄像头, 失败则用静态图片 ----------
    VideoCapture cap(0);
    bool useCamera = false;
    if (cap.isOpened()) {
        useCamera = true;
        cap.set(CAP_PROP_FRAME_WIDTH, 1280);
        cap.set(CAP_PROP_FRAME_HEIGHT, 720);
        cout << "[RectangleAndCenter] 已打开摄像头，按 q 或 ESC 退出" << endl;
    }
    else {
        // 没有摄像头的环境用静态图片兜底，方便调试
        if (imread("static/shapes.png").empty()) {
            cerr << "[RectangleAndCenter] 摄像头打不开，也读不到 static/shapes.png，跳过该步骤" << endl;
            return;
        }
        cout << "[RectangleAndCenter] 摄像头不可用，改用静态图片 static/shapes.png" << endl;
        cout << "[RectangleAndCenter] 按任意键退出" << endl;
    }

    Mat frame;
    int frameIdx = 0;

    while (true) {
        // ---------- ② 取一帧 ----------
        if (useCamera) cap >> frame;
        else           frame = imread("static/shapes.png");

        if (frame.empty()) {
            cerr << "[RectangleAndCenter] 无法读取画面" << endl;
            break;
        }
        frameIdx++;

        // ---------- ③ 检测装甲板 ----------
        vector<Point2f> corners;
        bool detected = detectArmor(frame, corners);

        Mat display = frame.clone();

        if (detected) {
            // 画旋转矩形（4 条边连起来）
            for (int i = 0; i < 4; ++i) {
                line(display, corners[i], corners[(i + 1) % 4], Scalar(0, 255, 0), 2);
            }

            // 算中心（4 个顶点的平均值）并标注
            Point2f center(0.f, 0.f);
            for (int i = 0; i < 4; ++i) center += corners[i];
            center.x /= 4.f; center.y /= 4.f;

            circle(display, center, 5, Scalar(0, 0, 255), -1);
            putText(display, format("C(%.0f,%.0f)", center.x, center.y),
                Point((int)center.x + 8, (int)center.y - 8),
                FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 255), 2);

            // 把 4 个顶点按顺序标上编号，方便核对顺序对不对
            const char* tag[4] = { "1 TL", "2 TR", "3 BR", "4 BL" };
            for (int i = 0; i < 4; ++i) {
                circle(display, corners[i], 4, Scalar(255, 0, 255), -1);
                putText(display, tag[i], Point((int)corners[i].x + 6, (int)corners[i].y + 16),
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 255), 1);
            }

            // 每 30 帧打印一次，避免刷屏
            if (frameIdx % 30 == 1) {
                cout << "----- frame " << frameIdx << " 检测到装甲板 -----" << endl;
                for (int i = 0; i < 4; ++i) {
                    cout << "  [" << i + 1 << "] " << tag[i] << " = ("
                        << corners[i].x << ", " << corners[i].y << ")" << endl;
                }
                cout << "  center = (" << center.x << ", " << center.y << ")" << endl;
            }
        }
        else if (frameIdx % 30 == 1) {
            cout << "----- frame " << frameIdx << " 未检测到装甲板 -----" << endl;
        }

        // ---------- ④ 画面左上角显示状态 ----------
        putText(display,
            format("frame %d | %s | %s", frameIdx,
                useCamera ? "camera" : "image",
                detected ? "DETECTED" : "none"),
            Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7,
            detected ? Scalar(0, 255, 0) : Scalar(0, 0, 255), 2);

        imshow("RectangleAndCenter - processArmorVideo", display);

        int key = waitKey(useCamera ? 30 : 0);   // 静态图片时等按键
        if (key == 'q' || key == 27) break;      // q 或 ESC 退出
        if (!useCamera) break;                  // 静态图片只显示一帧
    }

    cap.release();
    destroyAllWindows();
    return;
}