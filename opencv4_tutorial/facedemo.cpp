#include "facedemo.h"
#include "qapplication.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

FaceDemo::FaceDemo(QWidget* parent):QWidget(parent),
    isCameraActive(false), isVideoActive(false),confidenceThreshold(0.7), isFaces(true)
{
    qDebug() << "FaceDemo构造函数开始";

    //全局设置样式
    this->setStyleSheet("QLabel { border: 2px solid gray; background-color: #f0f0f0; border-radius: 10px; }");

    initUI();      //初始化界面

    connectUI();   //处理组件逻辑

    initialize();  //加载模型
}

void FaceDemo::initUI()
{
    hlayout = new QHBoxLayout();
    imagebt = new QRadioButton("图片");
    videobt = new QRadioButton("视频");
    camerabt = new QRadioButton("摄像头");
    hlayout->addWidget(imagebt);
    hlayout->addWidget(videobt);
    hlayout->addWidget(camerabt);

    image_label = new QLabel();
    image_label->setAlignment(Qt::AlignCenter);
    image_label->setMinimumSize(640, 480);
    image_label->setText("展示界面");
    status_label = new QLabel();
    status_label->setMaximumHeight(30);
    status_label->setAlignment(Qt::AlignCenter);

    hl = new QHBoxLayout();
    closebt = new QPushButton("关闭");
    closebt->setFixedSize(150, 50);
    hl->addWidget(closebt);

    vlayout = new QVBoxLayout(this);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(image_label);
    vlayout->addWidget(status_label);
    vlayout->addLayout(hl);

    frameTimer = new QTimer(this);  // 统一帧定时器
}

void FaceDemo::connectUI()
{
    connect(imagebt, &QRadioButton::clicked, this, &FaceDemo::imagebt_clicked);
    connect(videobt, &QRadioButton::clicked, this, &FaceDemo::videobt_clicked);
    connect(camerabt, &QRadioButton::clicked, this, &FaceDemo::camerabt_clicked);
    connect(closebt, &QPushButton::clicked, this, &FaceDemo::closebt_clicked);

    connect(frameTimer, &QTimer::timeout, this, &FaceDemo::process_frame);  // 连接统一处理函数
}

bool FaceDemo::initialize()
{
    try {
        // 加载DNN模型
        QString modelConfig = "E:/QT/QtOpencv/opencv4_tutorial/opencv_face_detector.pbtxt";
        QString modelWeights = "E:/QT/QtOpencv/opencv4_tutorial/opencv_face_detector_uint8.pb";

        net = cv::dnn::readNetFromTensorflow(modelWeights.toStdString(), modelConfig.toStdString());

        if (net.empty()) {
            status_label->setText("错误: 加载DNN模型失败");
            return false;
        }

        status_label->setText("就绪: DNN模型加载成功");
        return true;

    } catch (const cv::Exception &e) {
        status_label->setText(QString("错误: OpenCV异常 - %1").arg(e.what()));
        return false;
    }
}


cv::Mat FaceDemo::detectFaces(const cv::Mat &frame)
{
    cv::Mat result = frame.clone();
    cv::Mat blob;

    // 创建blob从输入图像
    cv::dnn::blobFromImage(frame, blob, scaleFactor,
                           cv::Size(inputWidth, inputHeight),
                           meanValues, false, false);

    // 设置网络输入
    net.setInput(blob);

    // 前向传播获取检测结果
    cv::Mat detection = net.forward();

    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    std::vector<cv::Rect> faces;
    std::vector<float> confidences;

    for (int i = 0; i < detectionMat.rows; i++) {
        float confidence = detectionMat.at<float>(i, 2);

        if (confidence > confidenceThreshold) {
            int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
            int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
            int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
            int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

            cv::Rect face(x1, y1, x2 - x1, y2 - y1);
            faces.push_back(face);
            confidences.push_back(confidence);
        }
    }

    // 绘制检测结果
    if (isFaces && !faces.empty()) {
        drawDetectionResults(result, faces, confidences);
    }

    return result;
}

QImage FaceDemo::loadAndDetect(const QString &filePath)
{
    try {
        cv::Mat image = cv::imread(filePath.toStdString());
        if (image.empty()) {
            status_label->setText("错误: 无法加载图像文件");
            return QImage();
        }

        cv::Mat result = detectFaces(image);
        return cvMatToQImage(result);

    } catch (const cv::Exception &e) {
        status_label->setText(QString("错误: 图像处理失败 - %1").arg(e.what()));
        return QImage();
    }
}

void FaceDemo::drawDetectionResults(cv::Mat &frame, const std::vector<cv::Rect> &faces, const std::vector<float>& confidences)
{
    for (size_t i = 0; i < faces.size(); ++i) {
        const cv::Rect &face = faces[i];
        float confidence = confidences[i];

        // 绘制人脸矩形框
        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

        // 显示置信度
        std::string label = cv::format("Face %d: %.2f", i + 1, confidence);
        cv::putText(frame, label, cv::Point(face.x, face.y - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 0), 3);
    }

    // 在左上角显示总人数
    std::string countText = "Faces: " + std::to_string(faces.size());
    cv::putText(frame, countText, cv::Point(40, 40),
                cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 255, 0), 3);
}

QImage FaceDemo::cvMatToQImage(const cv::Mat &mat)
{
    switch (mat.type()) {
    case CV_8UC4: {
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_ARGB32);
        return image.copy();
    }
    case CV_8UC3: {
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    case CV_8UC1: {
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image.copy();
    }
    default:
        return QImage();
    }
}

void FaceDemo::imagebt_clicked()
{
    qDebug() << "图片按钮点击";
    QString filePath = QFileDialog::getOpenFileName(this, "选择图像",
                                                    "",
                                                    "图像文件 (*.jpg *.jpeg *.png *.bmp *.tiff)");
    if (filePath.isEmpty()) {
        return;
    }

    status_label->setText("正在处理图像...");
    QApplication::processEvents();

    QImage result = loadAndDetect(filePath);
    if (!result.isNull()) {
        currentImage = result;
        image_label->setPixmap(QPixmap::fromImage(result).scaled(
            image_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        status_label->setText("图像处理完成");
    }
}

void FaceDemo::videobt_clicked()
{
    qDebug() << "视频按钮点击";

    // 先停止其他媒体
    closebt_clicked();

    QString filePath = QFileDialog::getOpenFileName(this, "选择视频文件",
                                                    "",
                                                    "视频文件 (*.mp4 *.avi *.mov *.wmv *.mkv)");
    if (filePath.isEmpty()) {
        return;
    }

    // 释放之前的视频资源
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }

    // 打开视频文件
    videoCapture.open(filePath.toStdString());
    if (!videoCapture.isOpened()) {
        status_label->setText("错误: 无法打开视频文件");
        return;
    }

    status_label->setText("正在播放视频...");
    isVideoActive = true;

    // 启动统一帧定时器
    frameTimer->start(33); // 33ms ≈ 30fps
}

void FaceDemo::camerabt_clicked()
{
    qDebug() << "摄像头按钮点击";

    // 先停止其他媒体
    closebt_clicked();

    // 释放之前的摄像头资源
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }

    // 尝试打开摄像头
    videoCapture.open(0);
    if (!videoCapture.isOpened()) {
        status_label->setText("错误: 无法打开摄像头");
        return;
    }

    status_label->setText("摄像头已启动");
    isCameraActive = true;

    // 启动统一帧定时器
    frameTimer->start(33); // 33ms ≈ 30fps
}

void FaceDemo::closebt_clicked()
{
    qDebug() << "关闭按钮点击";

    // 停止定时器
    if (frameTimer->isActive()) {
        frameTimer->stop();
    }

    // 释放摄像头/视频资源
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }

    // 重置状态
    isCameraActive = false;
    isVideoActive = false;

    // 清空显示
    image_label->clear();
    image_label->setText("展示界面");
    status_label->setText("已停止媒体播放");

    // 如果有缓存的图片，也清空
    currentImage = QImage();
}

void FaceDemo::process_frame()
{
    cv::Mat frame;

    if (isCameraActive || isVideoActive) {
        videoCapture >> frame;

        if (frame.empty()) {
            if (isCameraActive) {
                status_label->setText("错误: 无法从摄像头获取帧");
                isCameraActive = false;
            } else if (isVideoActive) {
                status_label->setText("视频播放完成");
                isVideoActive = false;
            }
            frameTimer->stop();
            return;
        }

        // 如果是摄像头，将画面左右翻转
        if (isCameraActive) {
            cv::flip(frame, frame, 1);  // 1 表示水平翻转（左右镜像）
        }

        cv::Mat result = detectFaces(frame);
        QImage qimage = cvMatToQImage(result);

        image_label->setPixmap(QPixmap::fromImage(qimage).scaled(
            image_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
