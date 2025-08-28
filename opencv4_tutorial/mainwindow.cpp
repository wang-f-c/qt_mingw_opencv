// MainWindow.cpp
#include "MainWindow.h"
#include "facedemo.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    FaceDemo *demo=new FaceDemo(this);
    setCentralWidget(demo);
    setWindowTitle("人脸检测程序");
    resize(800, 700);
}

MainWindow::~MainWindow()
{

}
