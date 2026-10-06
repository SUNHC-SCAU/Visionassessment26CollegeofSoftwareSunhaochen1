#include "MouseCropper.hpp"
#include <iostream>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>
using namespace cv;
using namespace std;

// ---- 全局状态 ----
Mat   img;                 // 原图，只读
Mat   imgCopy;             // 显示副本（所有绘制画在它上面）
Point startPoint(-1, -1);
Point endPoint(-1, -1);
bool  isDrawing = false;
bool  hasSelection = false;
Rect  roi(0, 0, 0, 0);     // ★ 改点3：空矩形，在回调里更新

// ---------------------------------------------------------------------------
// 重画：从干净的原图开始，画框线 + 鼠标信息（★ 改点6）
// ---------------------------------------------------------------------------
static void redraw(int mouseX = -1, int mouseY = -1) {
    imgCopy = img.clone();

    // 框线
    if (startPoint.x >= 0 && endPoint.x >= 0) {
        rectangle(imgCopy, startPoint, endPoint, Scalar(0, 255, 0), 2);
    }

    // 鼠标像素信息（★ 改点2：先做边界检查）
    if (mouseX >= 0 && mouseY >= 0 && mouseX < img.cols && mouseY < img.rows) {
        const Vec3b pixel = img.at<Vec3b>(mouseY, mouseX);
        char info[128];
        snprintf(info, sizeof(info), "Pos: (%d, %d) RGB: (%d, %d, %d)",
            mouseX, mouseY, pixel[2], pixel[1], pixel[0]);

        const int tx = std::min(mouseX + 10, std::max(0, img.cols - 250));
        const int ty = std::max(mouseY - 10, 30);
        putText(imgCopy, info, Point(tx, ty),
            FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 255), 2);
    }

    imshow("Image", imgCopy);
}

// ---------------------------------------------------------------------------
// 鼠标回调
// ---------------------------------------------------------------------------
void onMouse(int event, int x, int y, int flags, void* userdata) {
    // ★ 改点2：统一边界检查，越界直接忽略
    if (img.empty()) return;
    if (x < 0 || y < 0 || x >= img.cols || y >= img.rows) return;

    switch (event) {
    case EVENT_LBUTTONDOWN:
        isDrawing = true;
        hasSelection = false;
        startPoint = Point(x, y);
        endPoint = Point(x, y);
        redraw(x, y);
        break;

    case EVENT_MOUSEMOVE:
        if (isDrawing) endPoint = Point(x, y);
        redraw(x, y);
        break;

    case EVENT_LBUTTONUP:
        if (isDrawing) {
            isDrawing = false;
            endPoint = Point(x, y);

            // ★ 改点3：在这里计算 roi
            roi = Rect(std::min(startPoint.x, endPoint.x),
                std::min(startPoint.y, endPoint.y),
                std::abs(endPoint.x - startPoint.x),
                std::abs(endPoint.y - startPoint.y));
            hasSelection = (roi.width > 0 && roi.height > 0);

            if (hasSelection) {
                // ★ 改点4：正确的中心坐标
                const int center_x = roi.x + roi.width / 2;
                const int center_y = roi.y + roi.height / 2;
                cout << "[INFO] 框选区域: x=" << roi.x << " y=" << roi.y
                    << " w=" << roi.width << " h=" << roi.height << endl;
                cout << "[INFO] 框中心像素坐标: (" << center_x << ", "
                    << center_y << ")" << endl;

                // ★ 改点5：拖动完成后【单独显示】框取的图像
                const Mat cropped = img(roi).clone();
                imshow("CroppedImage", cropped);
                cout << "[INFO] 已在新窗口显示裁剪结果，按 's' 保存" << endl;
            }
            else {
                cout << "[INFO] 选区太小（只是单击了一下？），请重新框选" << endl;
            }
        }
        redraw(x, y);
        break;

    default:
        redraw(x, y);
        break;
    }
}

// ---------------------------------------------------------------------------
void MouseCropper::processCatImage(const std::string& inputPath,
    const std::string& outputPath)
{
    img = imread(inputPath);
    if (img.empty()) {
        cout << "图片读取失败: " << inputPath << endl;
        return;
    }
    imgCopy = img.clone();          // ★ 改点1：关键！否则 putText 对空 Mat 操作会崩

    namedWindow("Image", WINDOW_NORMAL);
    imshow("Image", imgCopy);
    setMouseCallback("Image", onMouse);

    cout << "拖拽鼠标框选猫猫，按 's' 保存，按 'q' 退出" << endl;

    while (true) {
        const char key = static_cast<char>(waitKey(10));
        if (key == 's' || key == 'S') {
            if (!hasSelection || roi.width <= 0 || roi.height <= 0) {
                cout << "选区无效，请先框选" << endl;
                continue;
            }
            const Rect safe = roi & Rect(0, 0, img.cols, img.rows);
            if (safe.width <= 0 || safe.height <= 0) {
                cout << "选区超出图像范围" << endl;
                continue;
            }
            const Mat cropped = img(safe).clone();
            if (imwrite(outputPath, cropped)) {
                cout << "保存成功: " << outputPath
                    << "  (" << cropped.cols << "x" << cropped.rows << ")" << endl;
            }
            else {
                cout << "保存失败，检查输出路径: " << outputPath << endl;
            }
        }
        else if (key == 'q' || key == 'Q' || key == 27) {
            break;
        }
    }

    destroyAllWindows();
}