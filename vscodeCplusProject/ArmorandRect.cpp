#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>

//坐标点
struct Point {
    double x;
    double y;

    Point(double x_ = 0, double y_ = 0) : x(x_), y(y_) {}//这是用x_去初始化成员x，用y_ 去初始化成员y
};

//装甲板属性
struct Rect {
    int id;          // 数字id
    int color;       // 颜色
    Point point;     // 这里存放坐标（坐标设在左上角）
    double width;    // 宽
    double height;   // 高

    Rect(int id_ = 0, int color_ = 0, const Point& p = Point(), double w = 0, double h = 0)
        : id(id_), color(color_), point(p), width(w), height(h) {}//这也是列表的初始化
};

//装甲板类
class Armor {
private:
    Rect rect;

    //这个函数是用来处理数字的显示小问题：输出数字，整数不显示小数点，非整数正常显示
    static void printNum(double v) {
        if (std::fabs(v - std::round(v)) < 1e-9) {
            std::cout << static_cast<long long>(std::round(v));
        } else {
            std::cout << v;
        }
    }

    //这个函数会将x，y转成 (x,y) 格式输出点
    static void printPoint(const Point& p) {
        std::cout << "(";
        printNum(p.x);
        std::cout << ",";
        printNum(p.y);
        std::cout << ")";
    }

public:
    Armor(const Rect& r) : rect(r) {}

    Armor(int id, int color, const Point& p, double w, double h)
        : rect(id, color, p, w, h) {}

    //计算中心点坐标
    Point Central_Point() const {
        return Point(rect.point.x + rect.width / 2.0,
                     rect.point.y + rect.height / 2.0);
    }

    //计算对角线长度
    double Diagonal() const {
        return std::sqrt(rect.width * rect.width + rect.height * rect.height);
    }

    //输出装甲板4个点，从左上角开始顺时针输出
    void Armor_Point() const {
        Point leftTop(rect.point.x, rect.point.y);                            //左上
        Point rightTop(rect.point.x + rect.width, rect.point.y);              //右上
        Point rightBottom(rect.point.x + rect.width, rect.point.y + rect.height); //右下
        Point leftBottom(rect.point.x, rect.point.y + rect.height);           //左下

        printPoint(leftTop);
        std::cout << " ";
        printPoint(rightTop);
        std::cout << " ";
        printPoint(rightBottom);
        std::cout << " ";
        printPoint(leftBottom);
    }

    //返回颜色字符串
    std::string Armor_Color() const {
        return rect.color == 0 ? "蓝" : "红";
    }

    //获取ID
    int ID() const {
        return rect.id;
    }
};

int main() {
    int id, color;
    double x, y, w, h;

    //输入：第一行 ID 和颜色；第二行 左上角坐标、宽、高
    std::cin >> id >> color;
    std::cin >> x >> y >> w >> h;

    Armor armor(id, color, Point(x, y), w, h);

    //第一行输出ID 和颜色
    std::cout << "ID：" << armor.ID() << " 颜色：" << armor.Armor_Color() << std::endl;

    //第二行输出中心坐标和对角线长度，对角线保留两位小数
    Point center = armor.Central_Point();
    std::cout << "(" << center.x << "," << center.y << ") 长度：";
    std::cout << std::fixed << std::setprecision(2) << armor.Diagonal() << std::endl;

    //恢复默认输出格式，避免影响后面坐标输出
    std::cout.unsetf(std::ios::fixed);
    std::cout << std::setprecision(6);

    //第三行输出4个点，从左上角顺时针输出
    armor.Armor_Point();
    std::cout << std::endl;

    return 0;
}