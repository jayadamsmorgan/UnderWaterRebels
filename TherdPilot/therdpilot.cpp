#include "therdpilot.h"
#include "ui_therdpilot.h"
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPushButton>


TherdPilot::TherdPilot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TherdPilot)
{



   { ui->setupUi(this);
    ui->xPos->setReadOnly(true);
    ui->yPos->setReadOnly(true);
    ui->xPos->setValue(cursor().pos().x());
    ui->yPos->setValue(cursor().pos().y());
    confSpin(ui->xPos, -INT_MAX, +INT_MAX);
    confSpin(ui->yPos, -INT_MAX, +INT_MAX);
    }
    // Координаты указателя

    scene = new paintScene(this);
    ui->graphicsView->setScene(scene);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &TherdPilot::slotTimer);
    timer->start(100);

    mCamera = new QCamera(this);
    mCameraViewfinder = new QCameraViewfinder(this);
    mCameraImageCapture = new QCameraImageCapture(mCamera,this);
    mLayout = new QVBoxLayout;

    mCamera->setViewfinder(mCameraViewfinder);
    mLayout->addWidget(mCameraViewfinder);
    mLayout->setMargin(0);
   // ui->graphicsView->setLayout(mLayout);


    StartB = new QPushButton ("Start Camera", this);
    StartB ->move(1140,86);
    StartB ->resize(110,21);
    connect(StartB, &QPushButton::clicked, [&](){
        mCamera->start();
    });

    StopB = new QPushButton ("Stop camera", this);
    StopB ->move(1140,115);
    StopB ->resize(110,21);
    connect(StopB, &QPushButton::clicked, [&](){
        mCamera->stop();
    });

    SavePB = new QPushButton("Save Picture", this);
    SavePB ->move(1140,144);
    SavePB ->resize(110,21);
    connect(SavePB, &QPushButton::clicked, [&](){
        auto filename = QFileDialog::getSaveFileName(this,"Picture","/","Image (*.jpg;*.jpeg)");
        if (filename.isEmpty()){       return;   }
        mCameraImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
        QImageEncoderSettings imageEncoderSettings;
        imageEncoderSettings.setCodec("image/jpeg");
        imageEncoderSettings.setResolution(1920, 1080);
        mCameraImageCapture->setEncodingSettings(imageEncoderSettings);
        mCamera->setCaptureMode(QCamera::CaptureStillImage);
        mCamera->start();
        mCamera->searchAndLock();
        mCameraImageCapture->capture(filename);
        mCamera->unlock();
    });
}

TherdPilot::~TherdPilot(){
    delete ui;
}
void TherdPilot::slotTimer()
{
    timer->stop();
    scene->setSceneRect(0,0, ui->graphicsView->width() - 20, ui->graphicsView->height() - 20);
}

void TherdPilot::resizeEvent(QResizeEvent *event)
{
    timer->start(100);
    QWidget::resizeEvent(event);
}

void TherdPilot::mousePressEvent(QMouseEvent *event){
    onMouseEvent(event->pos());
}

/*void TherdPilot::mouseMoveEvent(QMouseEvent *event){
    onMouseEvent(event->pos());
}*/
void TherdPilot::confSpin(QSpinBox *spinBox, int min, int max) const{
    spinBox->setMinimum(min);
    spinBox->setMaximum(max);
}
void TherdPilot::onMouseEvent(const QPoint &pos){
    ui->xPos->setValue(pos.x());
    ui->yPos->setValue(pos.y());
}



