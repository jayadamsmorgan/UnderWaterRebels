#include "thirdpilot.h"
#include "cameraview.h"

#include <QCameraViewfinder>
#include <QCamera>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainter>
#include <QCamera>
#include <QCameraImageCapture>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>
#include <cmath>
#include <QMainWindow>
#include <QTimer>



thirdpilot::thirdpilot(QWidget *parent) :
    QMainWindow(parent)
{
    resize(1925,1025);



    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timeout()));
    timer -> start(10);




    mCamera = new QCamera(this);
    mCameraV = new CameraView(this);
    image = new QLabel(this);
    image -> setAlignment(Qt::AlignHCenter);
    mCamera -> setViewfinder(mCameraV);
    setCentralWidget(mCameraV);

    StartB = new QPushButton ("Старт", this);
    StartB -> move(1760,20);
    StartB -> resize(80,20);
    connect(StartB, &QPushButton::clicked, [&](){
        mCamera->start();
    });

    StopB = new QPushButton ("Стоп", this);
    StopB ->move(1760,40);
    StopB ->resize(80,20);
    connect(StopB, &QPushButton::clicked, [this]() {
        mCamera->stop();
    });
    updL = new QLabel(this);
    updL->resize(1000,800);updL->move(300,0);
    updB = new QPushButton (this);
    updB -> move (1840,20);
    updB -> resize (40,40);
    connect(updB, &QPushButton::clicked, [this](){

    QString filename = QFileDialog::getOpenFileName(this, tr("Choose"), "", tr("image (*.png *.jpg *.jpeg *.bmp *.gif )"));

        if (QString::compare(filename, QString()) != 0)
        {
            QImage image;
            bool valid = image.load(filename);

            if (valid)
            {
                image = image.scaledToWidth(updL->width(), Qt::SmoothTransformation);
                updL->setPixmap(QPixmap::fromImage(image));
            }

        }
    });


}

thirdpilot::~thirdpilot(){

}

void thirdpilot::mousePressEvent(QMouseEvent *event){

}
void thirdpilot::timeout(){
    resize(1925,1025);
    resize(1924,1025);
}
