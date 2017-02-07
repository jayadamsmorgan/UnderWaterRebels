#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    setpalprop();
    if(QCameraInfo::availableCameras().size() != 0)
    {
        camera = new QCamera(QCameraInfo::availableCameras().at(0));
        camera->setViewfinder(ui->camera);
        ui->camera->setAspectRatioMode(Qt::IgnoreAspectRatio);
        camera->start();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setpalprop()
{
    QPalette labPal;
    labPal.setColor(QPalette::WindowText,Qt::white);
    ui->label->setPalette(labPal);
    ui->label->setAutoFillBackground(true);
    ui->label_2->setPalette(labPal);
    ui->label_2->setAutoFillBackground(true);

    ui->label_3->setPalette(labPal);
    ui->label_3->setAutoFillBackground(true);
    ui->label_4->setPalette(labPal);
    ui->label_4->setAutoFillBackground(true);
    ui->label_5->setPalette(labPal);
    ui->label_5->setAutoFillBackground(true);
    ui->label_6->setPalette(labPal);
    ui->label_6->setAutoFillBackground(true);
    ui->label_7->setPalette(labPal);
    ui->label_7->setAutoFillBackground(true);
    ui->label_8->setPalette(labPal);
    ui->label_8->setAutoFillBackground(true);
    ui->label_9->setPalette(labPal);
    ui->label_9->setAutoFillBackground(true);
    ui->label_10->setPalette(labPal);
    ui->label_10->setAutoFillBackground(true);

}
