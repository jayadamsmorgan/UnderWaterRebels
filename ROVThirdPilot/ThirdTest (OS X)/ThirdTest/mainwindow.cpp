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
float knownsize = 25;
float northrad= 0;
double koef = 1;
int dNorth = 0;
int stpos = 1;
vector<vector<float>> conFuncs(3);



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    captured  = false;
    setMouseTracking(true);
    camera = new QCamera(QCameraInfo::availableCameras().at(0));
    capture = new QCameraImageCapture(camera);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    connect(capture,SIGNAL(imageCaptured(int,QImage)),this,SLOT(onProcessCature(int,QImage)));
    camera->setViewfinder(ui->camera);
    camera->start();

    ui->knownsize->setText(QString::number(knownsize));
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
     koef  =  knownsize/sqrt(pow(points[0].x() - points[1].x(),2) + pow(points[0].y()- points[1].y(),2));
     //qDebug()<<"red length pix" <<sqrt(pow(points[0].x() - points[1].x(),2) + pow(points[0].y()- points[1].y(),2)) << "actual red" << knownsize;
     ui->koef->setText(QString::number(koef));
    }
    if(points.size() >= 4)//north angle calculation
    {
      int aNorth,bNorth,cNorth;
      aNorth = points[3].y()-points[2].y();
      bNorth = points[2].x()-points[3].x();
      cNorth =  (points[2].x()*(aNorth))-(points[2].y()*((-1)*bNorth));
        //qDebug() << points[2] <<" "<<points[3]<<" "<< aNorth<<" "<<bNorth<<" "<<cNorth;
        //xo = 0; yo = ((-1)*cNorth)/bNorth;
        //xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;
      float length = sqrt(pow(points[3].x() - points[2].x(),2) + pow(points[3].y()- points[2].y(),2));


      if(( points[3].x() > points[2].x()) && (points[3].y() < points[2].y())) // 1 quatre
      {
          dNorth = 1;
          northrad = asin((points[2].y() - points[3].y())/length);
          northrad =  northrad;
      }
      else if(( points[3].x() < points[2].x()) && (points[3].y() < points[2].y()))//2 quatre
      {
          dNorth = 2;
          northrad = asin(( points[2].y() - points[3].y() )/length);
          northrad = 3.14159265 - northrad ;
      }
      else if(( points[3].x() < points[2].x()) && (points[3].y() > points[2].y()))//3 quatre
      {
          dNorth = 3;
          northrad = asin((points[2].y() - points[3].y())/length);
          northrad = 3.14159265 - northrad ;
      }
      else if(( points[3].x()> points[2].x()) && (points[3].y() > points[2].y()))//4quatre
      {
          dNorth = 4;
          northrad = asin((points[2].y() - points[3].y())/length);
          northrad = 6.2831853 + northrad;
      }
       // qDebug()<< northrad << dNorth ;
    }

    if(points.size() >=6)//1t line calculaiton
    {
        int aNorth,bNorth,cNorth;
        float xo;float yo;float xh; float yh;
        aNorth = points[5].y()-points[4].y();
        bNorth = points[4].x()-points[5].x();
        cNorth =  (points[4].x()*(aNorth))-(points[4].y()*((-1)*bNorth));


        vector<float> tmp;
        tmp.resize(7);

        //xo = 0; yo = ((-1)*cNorth)/bNorth;
        //xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;

        tmp[0]= points[4].x(); tmp[1]= points[4].y();
        tmp[2]= points[5].x(); tmp[3]= points[5].y();
        tmp[4] =sqrt(pow((tmp[2]-tmp[0]),2) + pow((tmp[3]-tmp[1]),2));

        if(( points[5].x() > points[4].x()) && (points[5].y() < points[4].y())) // 1 quatre
        {
            tmp[6] = 1;
            tmp[5] = asin((abs(points[5].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) ) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if(dNorth == 2){tmp[5] = tmp[5] + 7.8539816339 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){tmp[5] = tmp[5] - northrad + 7.8539816339;}
            else if(dNorth == 4){tmp[5] = tmp[5] + northrad + 1.57079632;}

        }
        else if(( points[5].x() < points[4].x()) && (points[5].y() < points[4].y()))//2 quatre
        {
            tmp[6] = 2;
            tmp[5] = 3.14159265 - asin((abs(points[5].y()-points[4].y()))/tmp[4]);

            if((dNorth == 1)) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 2)){tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){qDebug()<<tmp[5];tmp[5] = tmp[5] - northrad + 7.8539816339;}

        }
        else if(( points[5].x() < points[4].x()) && (points[5].y() > points[4].y()))//3 quatre
        {
            tmp[6] = 3;
            tmp[5] = 3.14159265 + asin((abs(points[5].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) { tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3)){tmp[5] = tmp[5] - northrad + 1.57079632;}
            else if((dNorth == 4)){tmp[5] = tmp[5] - northrad + 7.8539816339;}
        }
        else if(( points[5].x()> points[4].x()) && (points[5].y() > points[4].y()))//4quatre
        {
            tmp[6] = 4;
            tmp[5] = 6.2831853 - asin((abs(points[5].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){tmp[5] = tmp[5] - northrad + 1.57079632;}

        }
            qDebug() << tmp[5] << tmp[6];

            tmp[4] *= koef;

            conFuncs[0] = tmp;
           //qDebug() <<"blue  line "<<tmp[4];

    }
    if(points.size() >=7)//2d line calculation
    {
        int aNorth,bNorth,cNorth;
        float xo;float yo;float xh; float yh;
        aNorth = points[6].y()-points[4].y();
        bNorth = points[4].x()-points[6].x();
        cNorth =  (points[4].x()*(aNorth))-(points[4].y()*((-1)*bNorth));


        vector<float> tmp;
        tmp.resize(7);

      //  xo = 0; yo = ((-1)*cNorth)/bNorth;
       // xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;

        tmp[0]= points[4].x(); tmp[1]= points[4].y();
        tmp[2]= points[6].x(); tmp[3]= points[6].y();
        tmp[4] =sqrt(pow((tmp[2]-tmp[0]),2) + pow((tmp[3]-tmp[1]),2));


        if(( points[6].x() > points[4].x()) && (points[6].y() < points[4].y())) // 1 quatre
        {
            tmp[6] = 1;
            tmp[5] = asin((abs(points[6].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) ) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if(dNorth == 2){tmp[5] = tmp[5] + 7.8539816339 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){tmp[5] = tmp[5] - northrad + 7.8539816339;}
            else if(dNorth == 4){tmp[5] = tmp[5] + northrad + 1.57079632;}

        }
        else if(( points[6].x() < points[4].x()) && (points[6].y() < points[4].y()))//2 quatre
        {
            tmp[6] = 2;
            tmp[5] = 3.14159265 - asin((abs(points[6].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){qDebug()<<tmp[5];tmp[5] = tmp[5] - northrad + 7.8539816339;}

        }
        else if(( points[6].x() < points[4].x()) && (points[6].y() > points[4].y()))//3 quatre
        {
            tmp[6] = 3;
            tmp[5] = 3.14159265 + asin((abs(points[6].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) { tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3)){tmp[5] = tmp[5] - northrad + 1.57079632;}
            else if((dNorth == 4)){tmp[5] = tmp[5] - northrad + 7.8539816339;}
        }
        else if(( points[6].x()> points[4].x()) && (points[6].y() > points[4].y()))//4quatre
        {
            tmp[6] = 4;
            tmp[5] = 6.2831853 - asin((abs(points[6].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){tmp[5] = tmp[5] - northrad + 1.57079632;}

        }
            qDebug() << tmp[5] << tmp[6];

            tmp[4] *= koef;

        conFuncs[1] = tmp;
    }
    if(points.size() >=8)//3d line calculation
    {
        int aNorth,bNorth,cNorth;
        float xo;float yo;float xh; float yh;
        aNorth = points[7].y()-points[4].y();
        bNorth = points[4].x()-points[7].x();
        cNorth =  (points[4].x()*(aNorth))-(points[4].y()*((-1)*bNorth));


        vector<float> tmp;
        tmp.resize(7);

       // xo = 0; yo = ((-1)*cNorth)/bNorth;
       // xh = ui->resultlab->width(); yh = ui->resultlab->height()-yo ;

        tmp[0]= points[4].x(); tmp[1]= points[4].y();
        tmp[2]= points[7].x(); tmp[3]= points[7].y();
        tmp[4] =sqrt(pow((tmp[2]-tmp[0]),2) + pow((tmp[3]-tmp[1]),2));


        if(( points[7].x() > points[4].x()) && (points[7].y() < points[4].y())) // 1 quatre
        {
            tmp[6] = 1;
            tmp[5] = asin((abs(points[7].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) ) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if(dNorth == 2){tmp[5] = tmp[5] + 7.8539816339 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){tmp[5] = tmp[5] - northrad + 7.8539816339;}
            else if(dNorth == 4){tmp[5] = tmp[5] + northrad + 1.57079632;}

        }
        else if(( points[7].x() < points[4].x()) && (points[7].y() < points[4].y()))//2 quatre
        {
            tmp[6] = 2;
            tmp[5] = 3.14159265 - asin((abs(points[7].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){qDebug()<<tmp[5];tmp[5] = tmp[5] - northrad + 7.8539816339;}

        }
        else if(( points[7].x() < points[4].x()) && (points[7].y() > points[4].y()))//3 quatre
        {
            tmp[6] = 3;
            tmp[5] = 3.14159265 + asin((abs(points[7].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) { tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3)){tmp[5] = tmp[5] - northrad + 1.57079632;}
            else if((dNorth == 4)){tmp[5] = tmp[5] - northrad + 7.8539816339;}
        }
        else if(( points[7].x()> points[4].x()) && (points[7].y() > points[4].y()))//4quatre
        {
            tmp[6] = 4;
            tmp[5] = 6.2831853 - asin((abs(points[7].y()-points[4].y()))/tmp[4]);
            if((dNorth == 1) || (dNorth == 2)) {tmp[5] = tmp[5] + 1.57079632 - northrad;}
            else if((dNorth == 3) || (dNorth == 4)){tmp[5] = tmp[5] - northrad + 1.57079632;}

        }
           // qDebug() << tmp[5] << tmp[6];

            tmp[4] *= koef;


        conFuncs[2] = tmp;
    }


    //draw the blank

    QPixmap mappix  = mappixmap;
    mappix = mappix.scaled(ui->resultlab->width()-10,ui->resultlab->height()-10,Qt::KeepAspectRatio);

    float resultkoef = (((float)mappix.width())/425);
    ui->resultlab->setPixmap(mappix);

    painter.begin((QPixmap*)ui->resultlab->pixmap());

    painter.setPen(QPen(Qt::red,5));
    painter.drawPoint(mappix.width()/2,mappix.height()/2);
    painter.setPen(QPen(Qt::red,3));
    painter.drawRect(mappix.width()/2-30,mappix.height()/2-45,60,90);

   if(points.size() >= 6)
   {
       float wid;
       float heig;
       if(((conFuncs[0][5] >= 0) && (conFuncs[0][5] <= 1.570796)) || ((conFuncs[0][5] > 6.2831853) && (conFuncs[0][5] <= 7.8539816339)) )//1 blank quatre
       {

            wid = cos(conFuncs[0][5])*conFuncs[0][4]*resultkoef;;
            heig = (-1)*sin(conFuncs[0][5])*conFuncs[0][4]*resultkoef;
       }
       else if(((conFuncs[0][5] > 1.570796) && (conFuncs[0][5] <= 3.14159265)) || ((conFuncs[0][5] > 7.8539816339) && (conFuncs[0][5] <= 9.42477796)) )//2 blank quatre
       {

            wid = cos(conFuncs[0][5])*conFuncs[0][4]*resultkoef;;
            heig = (-1)*sin(conFuncs[0][5])*conFuncs[0][4]*resultkoef;;
       }
       else if(((conFuncs[0][5] > 3.14159265) && (conFuncs[0][5] <= 4.71238898)) || ((conFuncs[0][5] > 9.42477796) && (conFuncs[0][5] <= 10.99557428)) )//3 blank quatre
       {
            //qDebug()<<"asaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaassssssssssssssss";
            wid = cos(conFuncs[0][5])*conFuncs[0][4]*resultkoef;
            heig = (-1)*sin(conFuncs[0][5])*conFuncs[0][4]*resultkoef;
       }
       else if(((conFuncs[0][5] > 4.71238898) && (conFuncs[0][5] <= 6.2831853)) || ((conFuncs[0][5] > 10.99557428) && (conFuncs[0][5] < 12.5663706)) )//4 blank quatre
       {
            //qDebug()<<"asaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaassssssssssssssssdddddddddddddddddddddddddddddddddddddddddddd";
            wid = cos(conFuncs[0][5])*conFuncs[0][4]*resultkoef;
            heig = (-1)*sin(conFuncs[0][5])*conFuncs[0][4]*resultkoef;
       }




        painter.setPen(QPen(Qt::blue,5));
        painter.drawPoint(mappix.width()/2 + wid,mappix.height()/2 + heig);
        painter.setPen(QPen(Qt::blue,3));
        painter.drawRect(mappix.width()/2-30 + wid,mappix.height()/2-45 + heig,60,90);
        painter.setPen(QPen(Qt::black,2));
        painter.drawLine(mappix.width()/2,mappix.height()/2,mappix.width()/2 + wid,mappix.height()/2 + heig);
        painter.drawText((mappix.width()/2+mappix.width()/2 + wid)/2 + 10,(mappix.height()/2 + mappix.height()/2 + heig)/2,QString::number(conFuncs[0][4]));
        painter.setPen(QPen(Qt::black,6));
        painter.drawText(mappix.width()/2-25 + wid,mappix.height()/2-50 + heig,directionDetermine(conFuncs[0][5]));
        ui->cont1->setText(QString::number(conFuncs[0][4]) + " cm, " + directionDetermine(conFuncs[0][5]));

   }
   if(points.size() >= 7)
   {
       float wid;
       float heig;
       if(((conFuncs[1][5] >= 0) && (conFuncs[1][5] <= 1.570796)) || ((conFuncs[1][5] > 6.2831853) && (conFuncs[1][5] <= 7.8539816339)) )//1 blank quatre
       {

            wid = cos(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
            heig = (-1)*sin(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
       }
       else if(((conFuncs[1][5] > 1.570796) && (conFuncs[1][5] <= 3.14159265)) || ((conFuncs[1][5] > 7.8539816339) && (conFuncs[1][5] <= 9.42477796)) )//2 blank quatre
       {
            wid = cos(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
            heig = (-1)*sin(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
       }
       else if(((conFuncs[1][5] > 3.14159265) && (conFuncs[1][5] <= 4.71238898)) || ((conFuncs[1][5] > 9.42477796) && (conFuncs[1][5] <= 10.99557428)) )//3 blank quatre
       {
            wid = cos(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
            heig = (-1)*sin(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
       }
       else if(((conFuncs[1][5] > 4.71238898) && (conFuncs[1][5] <= 6.2831853)) || ((conFuncs[1][5] > 10.99557428) && (conFuncs[1][5] < 12.5663706)) )//4 blank quatre
       {
            wid = cos(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
            heig = (-1)*sin(conFuncs[1][5])*conFuncs[1][4]*resultkoef;
       }




        painter.setPen(QPen(Qt::blue,5));
        painter.drawPoint(mappix.width()/2 + wid,mappix.height()/2 + heig);
        painter.setPen(QPen(Qt::blue,3));
        painter.drawRect(mappix.width()/2-30 + wid,mappix.height()/2-45 + heig,60,90);
        painter.setPen(QPen(Qt::black,2));
        painter.drawLine(mappix.width()/2,mappix.height()/2,mappix.width()/2 + wid,mappix.height()/2 + heig);
        painter.drawText((mappix.width()/2+mappix.width()/2 + wid)/2 + 10,(mappix.height()/2 + mappix.height()/2 + heig)/2,QString::number(conFuncs[1][4]));
        painter.setPen(QPen(Qt::black,6));
        painter.drawText(mappix.width()/2-25 + wid,mappix.height()/2-50 + heig,directionDetermine(conFuncs[1][5]));
        ui->cont2->setText(QString::number(conFuncs[1][4]) + " cm, " + directionDetermine(conFuncs[1][5]));
   }
   if(points.size() >= 8)
   {
       float wid;
       float heig;
       if(((conFuncs[2][5] >= 0) && (conFuncs[2][5] <= 1.570796)) || ((conFuncs[2][5] > 6.2831853) && (conFuncs[2][5] <= 7.8539816339)) )//1 blank quatre
       {

            wid = cos(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
            heig = (-1)*sin(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
       }
       else if(((conFuncs[2][5] > 1.570796) && (conFuncs[2][5] <= 3.14159265)) || ((conFuncs[2][5] > 7.8539816339) && (conFuncs[2][5] <= 9.42477796)) )//2 blank quatre
       {
            wid = cos(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
            heig = (-1)*sin(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
       }
       else if(((conFuncs[2][5] > 3.14159265) && (conFuncs[2][5] <= 4.71238898)) || ((conFuncs[2][5] > 9.42477796) && (conFuncs[2][5] <= 10.99557428)) )//3 blank quatre
       {
            wid = cos(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
            heig = (-1)*sin(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
       }
       else if(((conFuncs[2][5] > 4.71238898) && (conFuncs[2][5] <= 6.2831853)) || ((conFuncs[2][5] > 10.99557428) && (conFuncs[2][5] < 12.5663706)) )//4 blank quatre
       {
            wid = cos(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
            heig = (-1)*sin(conFuncs[2][5])*conFuncs[2][4]*resultkoef;
       }




        painter.setPen(QPen(Qt::blue,5));
        painter.drawPoint(mappix.width()/2 + wid,mappix.height()/2 + heig);
        painter.setPen(QPen(Qt::blue,3));
        painter.drawRect(mappix.width()/2-30 + wid,mappix.height()/2-45 + heig,60,90);
        painter.setPen(QPen(Qt::black,2));
        painter.drawLine(mappix.width()/2,mappix.height()/2,mappix.width()/2 + wid,mappix.height()/2 + heig);

        painter.drawText((mappix.width()/2+mappix.width()/2 + wid)/2 + 10,(mappix.height()/2 + mappix.height()/2 + heig)/2,QString::number(conFuncs[2][4] ));
        painter.setPen(QPen(Qt::black,6));
        painter.drawText(mappix.width()/2-25 + wid,mappix.height()/2-50 + heig,directionDetermine(conFuncs[2][5]));
        ui->cont3->setText(QString::number(conFuncs[2][4]) + " cm, " + directionDetermine(conFuncs[2][5]));
   }
    //qDebug() << xo <<" "<< yo <<" "<< xh <<" "<<yh;

    //painter.drawLine((int)xo,(int)yo,(int)xh,(int)yh);

    painter.end();
    //
   }

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //qDebug()<<100/stpos;
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
        int deltax = 2;

        int deltay = 15;//pixtmp->physicalDpiY()+3;//+ui->imagelab->geometry().y()+25;
        //qDebug()<<deltay;
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

    //pixm = pixm.scaled(ui->imagelab->width()-5,ui->imagelab->height()-5,Qt::KeepAspectRatio);
    //standartpic = standartpic.scaled(ui->imagelab->width()-5,ui->imagelab->height()-5,Qt::KeepAspectRatio);
    //QLabel lab;
    ui->imagelab->setPixmap(pixm);

    captured = true;
    points.clear();
    //ui->horizontalLayout_2->addWidget(&lab);
    //lab.setPixmap(pixm);

}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    standartpic = standartpic.scaled(ui->imagelab->width()-3,ui->imagelab->height()-3,Qt::KeepAspectRatio);

}



QString MainWindow::directionDetermine(float angle)
{

    angle = (angle*180)/(3.14159265);
    if(angle > 360)
    {
        angle  = angle-360;
    }

    if((angle <= 22.5) || (angle >= 337.5))
    {
        return "E";
    }
    else if((angle > 22.5) && (angle <= 67.5))
    {
        return "NE";
    }
    else if((angle > 67.5) && (angle <= 112.5))
    {
        return "N";
    }
    else if((angle > 112.5) && (angle <= 157.5))
    {
        return "NW";
    }
    else if(((angle > 157.5) && (angle <= 202.5)))
    {
        return "W";
    }
    else if((angle > 202.5) && (angle <= 247.5))
    {
        return "SW";
    }
    else if((angle > 247.5) && (angle <= 292.5))
    {
        return "S";
    }
    else if(((angle > 292.5) && (angle <= 337.5)))
    {
        return "SE";
    }

    return "unknown direction";
}

void MainWindow::on_knownsize_editingFinished()
{
    knownsize = ui->knownsize->text().toFloat();

}
