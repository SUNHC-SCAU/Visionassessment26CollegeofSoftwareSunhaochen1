#include "CameraRecorder.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
using namespace cv;
using namespace std;


   

    void VideoProcessor::processVideo(const std::string & outputPath)
    { 
    VideoCapture cap(0);
    if (!cap.isOpened()) {//一个检查
        cout << "无法打开摄像头" << std::endl;
        return ;
    }

    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);  // 0.25 通常是手动模式
    cap.set(cv::CAP_PROP_EXPOSURE, -5);      // -5 对应约 40ms 快门

    // 回读确认是否生效，set不一定成功，还要get看看当前曝光值真实是多少
    std::cout << "当前曝光值: " << cap.get(cv::CAP_PROP_EXPOSURE) << std::endl;


    // 获取摄像头实际分辨率，这个后面会打印一下并且被用到存视频的构造函数VideoWriter中（对象writer）（注意：有些摄像头默认可能只有 640x480）
    int frame_width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));//static_cast<int>强制将double类型转换成int类型，static_cast 是 C++ 里的类型转换运算符，用来把一种类型显式地转换成另一种类型。它是四种标准转换（static_cast、dynamic_cast、const_cast、reinterpret_cast）中最常用的一个。
    int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(CAP_PROP_FPS);
    if (fps < 1.0 || fps > 1000.0) fps = 25.0;  // 摄像头 FPS 不可靠时兜底

    cout <<"摄像头分辨率: " << frame_width << " x " << frame_height << " @ " << fps << " fps"<< endl;

    // 创建 VideoWriter，指定编码格式、帧率、尺寸
    //    'mp4v' 写 MP4，'MJPG' 写 AVI，具体看系统安装的编码器
    VideoWriter writer(outputPath,VideoWriter::fourcc( 'M', 'J', 'P', 'G'),fps,Size(frame_width, frame_height),true);//变量名outputPath不能"outputPath"
    /*注：上面那一行代码的逻辑
class VideoWriter {
public:
    VideoWriter(const String& filename, int fourcc, double fps, Size frameSize, bool isColor);
};

// 你使用它：定义对象，编译器自动调用构造函数
VideoWriter writer("out.avi", fourcc, fps, Size(w,h), true);
    */
    
    if (!writer.isOpened()) {//一个检查
        cout <<"无法创建输出视频文件，检查编码器是否可用"<< endl;
        return ;
    }
    // +滑动条，用来调节亮度与曝光时间，范围 0~100
    int brightness = 50;
    int exposure = 50;
    namedWindow("摄像头画面", WINDOW_AUTOSIZE);//只要后面的imshow 用的名字和 namedWindow 完全一样，imshow就会用那个已经建好的窗口，属性也保持 namedWindow 设定的
    createTrackbar("Brightness", "摄像头画面", &brightness, 100);//createTrackbar是用来创建滑动条的
    createTrackbar("Exposure", "摄像头画面", &exposure, 100);

    Mat frame;
    double prevTick = (double)getTickCount();//getTickCount();是用来计算tick数的
    double realFps = 0.0;
    while (true) {
        
        cap >> frame;//把cap的每一帧传给frame
        if (frame.empty()) {
            cout << "无法读取画面" << std::endl;
            break;
        }
        //把滑动条的值应用到摄像头
        cap.set(CAP_PROP_BRIGHTNESS, brightness);
        cap.set(CAP_PROP_EXPOSURE, exposure);

        //算实时 FPS 并画到画面上
        double now = (double)getTickCount();
        double dt = (now - prevTick) / getTickFrequency();//now-prevTick是间隔tick数，getTickFrequency();计算每秒有多少个 tick，所以这个除法能得到得到秒数
        prevTick = now;
        if (dt > 0) {
            double inst = 1.0 / dt;//dt 是“一帧用了多少秒”，那么“一秒能出多少帧”就是它的倒数，inst是实时帧率
            realFps = (realFps <= 0) ? inst : (0.9 * realFps + 0.1 * inst);//结果就是新值只轻微影响结果，FPS 显示平滑，不会因为某帧抖动而剧烈跳变
        }

        char info[128];//字符串缓冲区，配合 snprintf 存格式化后的文字
        snprintf(info, sizeof(info), "Size: %dx%d | FPS: %.1f | Rec: ON",frame.cols, frame.rows, realFps);//
        putText(frame, info, Point(10, 30), FONT_HERSHEY_SIMPLEX,0.7, Scalar(0, 255, 0), 2);
        //一个负责把数字格式化成字符串，一个负责把字符串画到图像上。配合使用就能在画面上显示 FPS、分辨率等信息

        // 在窗口中显示画面
        imshow("摄像头画面", frame);

        writer.write(frame);
        // 等待按键事件，按下 'q' 键退出循环
        if (waitKey(30) == 'q') {
            break;
        }
    }

    // 释放摄像头资源
    cap.release();
    writer.release();
    // 关闭所有 OpenCV 窗口
   destroyAllWindows();

;
}