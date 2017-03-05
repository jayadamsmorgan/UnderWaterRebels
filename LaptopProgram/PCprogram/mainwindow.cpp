#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    red = new QPixmap(":/Red.png");
    green = new QPixmap(":/Green.png");

    okauto = new QPalette();
    okauto->setColor(QPalette::WindowText,QColor(250,250,250));
    noauto = new QPalette();
    noauto->setColor(QPalette::WindowText,QColor(250,250,250));

    ui->connection->setPixmap(red->scaled(35,35));
    setpalprop();
    if(QCameraInfo::availableCameras().size() != 0)
    {
        camera = new QCamera(QCameraInfo::availableCameras().at(0));
        camera->setViewfinder(ui->camera);
        ui->camera->setAspectRatioMode(Qt::IgnoreAspectRatio);
        camera->start();

    }
}

void MainWindow::onupdateInfo(int ping, int depth, int speed, bool connection, bool yaw, bool pitch, bool roll)
{
    ui->depth->setText("Depth: " + QString::number(depth));
    //ui->ping->setText("Ping: " + QString::number(ping));

    switch(speed)
    {
        case 0:
            ui->speed->setText("Low");
        break;
        case 1:
            ui->speed->setText("Medium");
        break;
        case 2:
            ui->speed->setText("High");
        break;

    }

    if(connection) ui->connection->setPixmap(green->scaled(35,35));
    else ui->connection->setPixmap(red->scaled(35,35));

    if(yaw) ui->yaw->setPalette(*okauto);
    else ui->yaw->setPalette(*noauto);
    if(pitch) ui->pitch->setPalette(*okauto);
    else ui->pitch->setPalette(*noauto);
    if(roll) ui->roll->setPalette(*okauto);
    else ui->roll->setPalette(*noauto);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setpalprop()
{
    QPalette labPal;
    labPal.setColor(QPalette::WindowText,Qt::white);
   // ui->label->setPalette(labPal);
   // ui->label->setAutoFillBackground(true);
    ui->label_2->setPalette(labPal);
    ui->label_2->setAutoFillBackground(true);
    ui->speed->setPalette(labPal);
    ui->speed->setAutoFillBackground(true);
    ui->depth->setPalette(labPal);
    ui->depth->setAutoFillBackground(true);
   // ui->label_4->setPalette(labPal);
   // ui->label_4->setAutoFillBackground(true);
    ui->yaw->setPalette(labPal);
    ui->yaw->setAutoFillBackground(true);
    ui->pitch->setPalette(labPal);
    ui->pitch->setAutoFillBackground(true);
    ui->roll->setPalette(labPal);
    ui->roll->setAutoFillBackground(true);
    ui->label_8->setPalette(labPal);
    ui->label_8->setAutoFillBackground(true);
   // ui->ping->setPalette(labPal);
   // ui->ping->setAutoFillBackground(true);
   // ui->label_10->setPalette(labPal);
   // ui->label_10->setAutoFillBackground(true);

}
