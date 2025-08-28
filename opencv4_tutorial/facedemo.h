#ifndef FACEDEMO_H
#define FACEDEMO_H

#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include<opencv2/video.hpp>

class FaceDemo:public QWidget
{
    Q_OBJECT
public:
    FaceDemo(QWidget *parent=nullptr);
    void initUI();          //界面初始化
    void connectUI();       //连接界面的功能
    bool initialize();      //初始化人脸识别模型

    QImage detectFaces(const QImage &inputImage);   // 检测人脸
    cv::Mat detectFaces(const cv::Mat &frame);      //返回处理的图片

    cv::Mat captureAndDetect();       // 从摄像头捕获并检测
    QImage loadAndDetect(const QString &filePath);  // 加载图像文件并检测

    //绘制结果
    void drawDetectionResults(cv::Mat &frame, const std::vector<cv::Rect> &faces, const std::vector<float>& confidences);  // 绘制检测结果

    QImage cvMatToQImage(const cv::Mat &mat);   // 转换图像格式 Mat -> QImage

private slots:
    void imagebt_clicked();
    void videobt_clicked();
    void camerabt_clicked();
    void closebt_clicked();
    void process_frame();  // 统一处理帧

private:
    QRadioButton* imagebt;  //图片按钮
    QRadioButton* videobt;  //视频按钮
    QRadioButton* camerabt; //摄像头按钮
    QLabel* image_label;    //展示界面
    QLabel* status_label;    //提示状态标签
    QPushButton* closebt;   //关闭按钮

    QVBoxLayout *vlayout;   //垂直布局
    QHBoxLayout *hlayout;   //水平布局
    QHBoxLayout *hl;        //用于按钮居中

private:
    QImage currentImage;      //当前选中的图片

    cv::dnn::Net net;         // DNN网络
    cv::VideoCapture videoCapture;        //处理视频
    QTimer *frameTimer;                   //统一帧定时器

    //参数
    bool isCameraActive; //判断是否打开摄像头
    bool isVideoActive;  //判断是否打开视频
    double confidenceThreshold;  //置信度阈值
    bool isFaces;        //判断是否有人脸

    // 输入尺寸和缩放因子、去均值
    const int inputWidth = 300;
    const int inputHeight = 300;
    const double scaleFactor = 1.0;
    const cv::Scalar meanValues = cv::Scalar(104.0, 177.0, 123.0);

};

#endif // FACEDEMO_H
