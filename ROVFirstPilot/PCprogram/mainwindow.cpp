#include "mainwindow.h"
#include "ui_mainwindow.h"

bool camstate = false;
bool cam2state = false;
bool tstarted = false;
bool fstarted = false;
long long crtime = 0;
long long mstime = 0;
int cam2num = 0;
int tcamid = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
        camstate = false;
    cam2state = false;
    timer  = new QTime();
    okauto = new QPalette();
    okauto->setColor(QPalette::WindowText,QColor(250,250,250));
    noauto = new QPalette();
    noauto->setColor(QPalette::WindowText,QColor(30,30,30));

    connect(ui->replug,SIGNAL(triggered(bool)),this,SLOT(replugJoy(bool)));
    connect(ui->cameras,SIGNAL(triggered(bool)),this,SLOT(replugCams(bool)));
    connect(ui->debugw,SIGNAL(triggered(bool)),this,SLOT(openDebugWindow(bool)));

    setpalprop();
    qDebug() << QCameraInfo::availableCameras().size();
    if(QCameraInfo::availableCameras().size() != 0)
    {
        camstate = true;
        camera = new QCamera(QCameraInfo::availableCameras().at(0));
        camera->setViewfinder(ui->camera);
        ui->camera->setAspectRatioMode(Qt::IgnoreAspectRatio);
        camera->start();

    }

    if(QCameraInfo::availableCameras().size() >= 3)
    {
        cam2state = true;
        camera2 = new QCamera(QCameraInfo::availableCameras().at(1));
        camera2->setViewfinder(ui->camera2);
        ui->camera2->setAspectRatioMode(Qt::IgnoreAspectRatio);
        camera2->start();
    }
   // qDebug()<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
}

void MainWindow::onupdateInfo(int cam2id, int depth, int speed, int yaw, int pitch, int roll, int setyaw, int setpitch, bool connection, bool ayaw, bool apitch, bool adepth, bool leak)
{
    ui->depth->setText("Depth: " + QString::number(depth));
    ui->setdepth->setText("-> " + QString::number(setpitch));
    //ui->ping->setText("Ping: " + QString::number(ping));
    cam2num = cam2id;
    //qDebug()<<yaw << pitch << roll;
    ui->yawlab->setText("Yaw: " + QString::number(yaw));
    ui->pitchlab->setText("Pitch: " + QString::number(pitch));
    ui->rollab->setText("Roll: " + QString::number(roll));
    switch(speed)
    {
        case 0:
            ui->speed->setText("Speed: Low");
        break;
        case 1:
            ui->speed->setText("Speed: Medium");
        break;
        case 2:
            ui->speed->setText("Speed: High");
        break;
        case 3:
            ui->speed->setText("Speed: Turbo");
        break;

    }

    if(connection) ui->connection->setText("Connection: Yes");
    else ui->connection->setText("Connection:  No");

    if(leak) ui->leak->setText("Leak: Yes");
        else ui->leak->setText("Leak:  No");


    if(ayaw) ui->ayaw->setPalette(*okauto);
    else ui->ayaw->setPalette(*noauto);
    if(adepth) ui->apitch->setPalette(*okauto);
    else ui->apitch->setPalette(*noauto);
    if(apitch) ui->adepth->setPalette(*okauto);
    else ui->adepth->setPalette(*noauto);

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
    ui->connection->setPalette(labPal);
    ui->connection->setAutoFillBackground(true);
    ui->speed->setPalette(labPal);
    ui->speed->setAutoFillBackground(true);
    ui->depth->setPalette(labPal);
    ui->depth->setAutoFillBackground(true);
   // ui->label_4->setPalette(labPal);
   // ui->label_4->setAutoFillBackground(true);
    ui->ayaw->setPalette(labPal);
    ui->ayaw->setAutoFillBackground(true);
    ui->apitch->setPalette(labPal);
    ui->apitch->setAutoFillBackground(true);
    ui->adepth->setPalette(labPal);
    ui->adepth->setAutoFillBackground(true);
    ui->setdepth->setPalette(labPal);
    ui->setdepth->setAutoFillBackground(true);
    QPalette butPal;
    butPal.setColor(QPalette::ButtonText,Qt::white);
    ui->ptimer->setPalette(butPal);
   // ui->ping->setPalette(labPal);
   // ui->ping->setAutoFillBackground(true);
   // ui->label_10->setPalette(labPal);
   // ui->label_10->setAutoFillBackground(true);

}

void MainWindow::paintEvent(QPaintEvent *)
{
    if((fstarted) && (!tstarted))
    {
        long long deltatime = (timer->elapsed() - mstime)/1000;
        int mins = 14 - deltatime/60;
        int secs = 59 - deltatime%60;
        if((mins <=3))
        {
            QPalette redpal;
            redpal.setColor(QPalette::WindowText,Qt::red);
            ui->timer->setPalette(redpal);
            if(mins == 0 && secs == 0)
            {
            ui->timer->setText("00:00");
            }
            else
            {
                ui->timer->setText(QString::number(mins) + ":" + QString::number(secs));
            }
        }
        else if(mins <=7)
        {
            QPalette yellpal;
            yellpal.setColor(QPalette::WindowText,Qt::yellow);
            ui->timer->setPalette(yellpal);
            ui->timer->setText(QString::number(mins) + ":" + QString::number(secs));

        }
        else
        {
            if(secs >= 10)
                ui->timer->setText(QString::number(mins) + ":" + QString::number(secs));
            else
                ui->timer->setText(QString::number(mins) + ":0" + QString::number(secs) );
        }
    }


    //update();

    int camcount = QCameraInfo::availableCameras().size();

    if((camcount > 0 )  )
    {
        if(!camstate)
        {
        camstate = true;
        camera = new QCamera(QCameraInfo::availableCameras().at(1 - tcamid ));
        camera->setViewfinder(ui->camera);
        ui->camera->setAspectRatioMode(Qt::IgnoreAspectRatio);
        camera->start();
        }
    }
    else
    {camstate = false;}

    if((camcount >= 2) && (!cam2state))
    {

        cam2state = true;

        camera2 = new QCamera(QCameraInfo::availableCameras().at(tcamid));
        camera2->setViewfinder(ui->camera2);
        ui->camera2->setAspectRatioMode(Qt::IgnoreAspectRatio);
        camera2->start();

    }
    else
    {cam2state = false;}


}

void MainWindow::replugCams(bool trig)
{
    if(tcamid == 0) tcamid == 1;
    else if(tcamid == 1) tcamid = 0;

}

void MainWindow::replugJoy(bool trig)
{
    emit updateJoy();
}

void MainWindow::openDebugWindow(bool trig)
{

}

void MainWindow::on_ptimer_clicked()
{

    if((!tstarted) && fstarted)
    {

        ui->ptimer->setText("restart");
        tstarted = true;
    }
    else if((!fstarted) && (!tstarted))
    {//start timer
        QPalette whitepal;
        whitepal.setColor(QPalette::WindowText,Qt::white);
        ui->timer->setPalette(whitepal);
        ui->ptimer->setText("stop");
        timer->start();
        mstime = timer->elapsed();
        crtime = 900;
        fstarted = true;
    }
    else if(tstarted && fstarted)
    {
        ui->ptimer->setText("start");
        fstarted = false;
        tstarted = false;

    }
}

