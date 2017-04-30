#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <vector>

using namespace std;

vector<QPoint> points;
bool captured;
float knownsize = 10.5;
float northrad= 0;

vector<vector<float>> conFuncs(3);



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    captured  = false;
    setMouseTracking(true);
        camera = new QCamera();
    capture = new QCameraImageCapture(camera);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    connect(capture,SIGNAL(imageCaptured(int,QImage)),this,SLOT(onProcessCature(int,QImage)));
    camera->setViewfinder(ui->camera);
    camera->start();

    QImage img(":/new/prefix1/map.PNG");


    mappixmap = QPixmap::fromImage(img);


    ui->resultlab->setPixmap(mappixmap.scaled(ui->resultlab->width(),ui->resultlab->height(),Qt::KeepAspectRatio));
    //ui->resultlab->setPixmap(mappixmap.scaled(600,1000,Qt::KeepAspectRatio));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::paintEvent(QPaintEvent *)
{

  //  qDebug()<<"lil1";
    QPainter painter;
    if(captured)
    {
    //QPixmap *pixtmp  = (QPixmap*)ui->imagelab->pixmap();
    QPixmap pixm;
    pixm = standartpic;

    ui->imagelab->setPixmap(pixm);

    painter.begin((QPixmap*)ui->imagelab->pixmap());
    //painter.setPen(QPen(Qt::black,5));
   // ui->resultlab->setPixmap(mappix->scaled(ui->resultlab->width(),ui->resultlab->height(),Qt::KeepAspectRatio));

    for(int i = 0;i < points.size();++i)
    {
        if((i == 0) || (i == 1))
            painter.setPen(QPen(Qt::red,4));
        else if((i == 2) || (i == 3))
            painter.setPen(QPen(Qt::green,4));
        else
            painter.setPen(QPen(Qt::blue,4));
       painter.drawPoint(points[i]);

       if((((i%2 == 0) &&(points.size()%2 == 0)) || ((i%2 == 0) &&(i < (points.size()-1)))) && (i <= 3))
       {
           QLine line;
           line.setP1(points[i]);
           line.setP2(points[i+1]);
           painter.drawLine(line);
           //ui->length->setText(QString::number(sqrt(pow(line.dx(),2) + pow(line.dy(),2))));

       }
       else if(i >=4)
       {
           QLine line;
           line.setP1(points[4]);
           line.setP2(points[i]);
           painter.drawLine(line);

       }

       //painter.drawLine(xo,yo,xh,yh);

    }

    painter.end();
    //processing
    if(points.size() >= 2)//koef calculation
    {
    double koef  =  knownsize/sqrt(pow(points[0].x(),2) + pow(points[1].y(),2));
    ui->koef->setText(QString::number(koef));
    }
    if(points.size() >= 4)
    {
      int aNorth,bNorth,cNorth;
      aNorth = points[3].y()-points[2].y();
      bNorth = points[2].x()-points[3].x();
      cNorth =  (points[2].x()*(aNorth))-(points[2].y()*((-1)*bNorth));
        //qDebug() << points[2] <<" "<<points[3]<<" "<< aNorth<<" "<<bNorth<<" "<<cNorth;
        //xo = 0; yo = ((-1)*cNorth)/bNorth;
        //xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;
        northrad = atan((-1)*(((cNorth - (aNorth*100))/bNorth)-points[2].y())/(100));
    }
    if(points.size() >=6)
    {
        int aNorth,bNorth,cNorth;
        float xo;float yo;float xh; float yh;
        aNorth = points[5].y()-points[4].y();
        bNorth = points[4].x()-points[5].x();
        cNorth =  (points[4].x()*(aNorth))-(points[4].y()*((-1)*bNorth));


        vector<float> tmp;
        tmp.resize(6);

        xo = 0; yo = ((-1)*cNorth)/bNorth;
        xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;

        tmp[0]= points[4].x(); tmp[1]= points[4].y();
        tmp[2]= points[5].x(); tmp[3]= points[5].y();
        tmp[4] =sqrt(pow((xh-xo),2) + pow((yh-yo),2));

        tmp[5] = atan((-1)*(float)(((float)(cNorth - (aNorth*(points[4].x() + 100)))/bNorth)-points[4].y())/(100)) - northrad;
        conFuncs[0] = tmp;
        qDebug() <<tmp[5];
    }
    if(points.size() >=7)
    {
        int aNorth,bNorth,cNorth;
        float xo;float yo;float xh; float yh;
        aNorth = points[6].y()-points[4].y();
        bNorth = points[4].x()-points[6].x();
        cNorth =  (points[4].x()*(aNorth))-(points[4].y()*((-1)*bNorth));


        vector<float> tmp;
        tmp.resize(6);

        xo = 0; yo = ((-1)*cNorth)/bNorth;
        xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;

        tmp[0]= points[4].x(); tmp[1]= points[4].y();
        tmp[2]= points[6].x(); tmp[3]= points[6].y();
        tmp[4] =sqrt(pow((xh-xo),2) + pow((yh-yo),2));

        tmp[5] = atan((-1)*(float)(((float)(cNorth - (aNorth*(points[4].x() + 100)))/bNorth)-points[4].y())/(100)) - northrad;
        conFuncs[1] = tmp;
    }
    if(points.size() >=8)
    {
        int aNorth,bNorth,cNorth;
        float xo;float yo;float xh; float yh;
        aNorth = points[7].y()-points[4].y();
        bNorth = points[4].x()-points[7].x();
        cNorth =  (points[4].x()*(aNorth))-(points[4].y()*((-1)*bNorth));


        vector<float> tmp;
        tmp.resize(6);

        xo = 0; yo = ((-1)*cNorth)/bNorth;
        xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;

        tmp[0]= points[4].x(); tmp[1]= points[4].y();
        tmp[2]= points[7].x(); tmp[3]= points[7].y();
        tmp[4] =sqrt(pow((xh-xo),2) + pow((yh-yo),2));

        tmp[5] = atan((-1)*(float)(((float)(cNorth - (aNorth*(points[4].x() + 100)))/bNorth)-points[4].y())/(100)) - northrad;
        conFuncs[2] = tmp;
    }


    //draw the blank

    QPixmap mappix  = mappixmap;
    mappix = mappix.scaled(ui->resultlab->width()-10,ui->resultlab->height()-10,Qt::KeepAspectRatio);
    ui->resultlab->setPixmap(mappix);
    /*
    if(points.size()>=4 && bNorth != 0)
    {
        xh = mappix.width(); yh = ((-1)*(cNorth + mappix.width()*aNorth))/bNorth ;
    }
    */
    painter.begin((QPixmap*)ui->resultlab->pixmap());
    painter.setPen(QPen(Qt::red,10));
    //qDebug() << xo <<" "<< yo <<" "<< xh <<" "<<yh;

    //painter.drawLine((int)xo,(int)yo,(int)xh,(int)yh);

    painter.end();
    //
   }

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QPoint pointf = event->pos();
    //QPoint pointf = event->globalPos();
    if(captured)
    {
        if(event->button() == Qt::RightButton)
        {
        points.pop_back();
        this->setGeometry(this->geometry().x(),this->geometry().y(),this->geometry().width()-1,this->geometry().height());
        this->setGeometry(this->geometry().x(),this->geometry().y(),this->geometry().width()+1,this->geometry().height());
        }
        else
        {
        QPixmap *pixtmp  = (QPixmap*)ui->imagelab->pixmap();
        int deltax = 3;//pixtmp->logicalDpiX();

        int deltay = pixtmp->physicalDpiY()+3;//+ui->imagelab->geometry().y()+25;
        qDebug()<<deltay;
        pointf.setX(pointf.x()-deltax);
        pointf.setY(pointf.y()-deltay);


        points.push_back(pointf);
        this->setGeometry(this->geometry().x(),this->geometry().y(),this->geometry().width()-1,this->geometry().height());
        this->setGeometry(this->geometry().x(),this->geometry().y(),this->geometry().width()+1,this->geometry().height());
        }
    }

}

void MainWindow::on_pushButton_clicked()
{
    capture->capture();

    //QImage image;
    //image = ui->camera->grab().toImage();

}

void MainWindow::onProcessCature(int requestId, const QImage &img)
{
    //camera->stop();
    //ui->camera->close();
    QPixmap pixm;
    pixm = QPixmap::fromImage(img);
    standartpic = QPixmap::fromImage(img);
    //camera->stop();
    //ui->camera->close();


    //QLabel lab;
    ui->imagelab->setPixmap(pixm);
    captured = true;
    points.clear();
    //ui->horizontalLayout_2->addWidget(&lab);
    //lab.setPixmap(pixm);



}
