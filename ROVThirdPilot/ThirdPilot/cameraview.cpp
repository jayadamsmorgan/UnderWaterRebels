#include "cameraview.h"
#include "thirdpilot.h"
#include <QPainter>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDebug>
#include <cmath>
#include <QPen>
#include <QLabel>
#include <QFont>
#include <QFrame>
#include <QPixmap>

int mouse_press_counter=0;
double rast = 50;
double converter = 1;
int rv2xy12;
int agn;
float Dist1;
float Dist2;
float Dist3;



CameraView::CameraView(QWidget *parent) : QCameraViewfinder(parent){
    int X = 1720;
    int X2 = 1810;
    XY = new QLabel("0 \n 0",this);
    XY->resize(50,50); XY->move(X + 15,11);
    xy = new QLabel("<b>X:<br/>Y:</b>",this);
    xy->resize (50,50); xy->move (X,11);
    //контейнеры 1. 2
  { x1 = new QLabel("X1",this); x1 -> resize(0,0); x1 -> move(X,80);
    x2 = new QLabel("X2",this); x2 -> resize(0,0); x2 -> move(X,100);
    y1 = new QLabel("Y1",this); y1 -> resize(0,0); y1 -> move(X+30,80);
    y2 = new QLabel("Y2",this); y2 -> resize(0,0); y2 -> move(X+30,100);
    rv = new QLabel("0", this); rv -> resize(100,20); rv -> move(X,80);
    rv1= new QLabel("0",this);rv1 -> resize(100,20); rv1 -> move(X+30,80);
    x3 = new QLabel("X3",this); x3->resize(0,0); x3->move(X,160);
    x4 = new QLabel("X4",this); x4->resize(0,0); x4->move(X,180);
    y3 = new QLabel("Y3",this); y3->resize(0,0); y3->move(X+30,160);
    y4 = new QLabel("Y4",this); y4->resize(0,0); y4->move(X+30,180);
    ravno = new QLabel("0",this); ravno->resize(100,20); ravno->move(X,200);
    ravno1 = new QLabel("0", this); ravno1 -> resize(22,20); ravno1 -> move (X+30,200);
    QLabel *distLab = new QLabel (" <font size=5> Distance from dangerous container to: </font>", this); distLab->move(X2-10,80);
    QLabel *Cont1 = new QLabel ("<b> Cont. 1: <b/>", this); Cont1->move(X2-10,120);
    QLabel *Cont2 = new QLabel ("<b> Cont. 2: <b/>", this); Cont2->move(X2-10,140);
    QLabel *Cont3 = new QLabel ("<b> Cont. 3: <b/>", this); Cont3->move(X2-10,160);
    QLabel *Dist1 = new QLabel ( "<font size=5> 0 </font>", this); Dist1->move(X2+55,120);
    //Dist1->setText( tr( "<font size=5> 0 </font>" ) );

    //QFont dist1 = Dist1->dist1(); dist1->setPointSize(20); Dist1->setFont(dist1);

    QLabel *Dist2 = new QLabel ("0", this); Dist2->move(X2+55,140);
    QLabel *Dist3 = new QLabel ("0", this); Dist3->move(X2+55,160);
    QLabel *Dis1 = new QLabel (this);

//Distance from dangerous container to:
   // rv3xy = new QLabel("0",this); rv3xy->resize(100,20); rv3xy->move(X,360);
   // rv3xy1 = new QLabel("0", this); rv3xy1 -> resize(100,20); rv3xy1 -> move (X+30,360);
    QLabel *pixcm = new QLabel("pix = 1cm",this); pixcm->resize(100,20);pixcm->move(X+25,105);
    pixcmrv = new QLabel(this); pixcmrv->resize(100,20);pixcmrv->move(X-5,140);
    QLabel *cm = new QLabel("cm",this); cm->move(X+55,165);
    QLabel *pixel = new QLabel("pix",this); pixel->move(X+55,83);
   /* QLabel *pixelF = new QLabel(this);
    pixelF -> resize(49,25); pixelF->move(1835,120);
    pixelF -> setFrameStyle(QFrame::Panel | QFrame::Raised);*/
    QLabel *FrXy = new QLabel(this);
    FrXy -> resize(170,50); FrXy->move(1714,15);
    FrXy -> setFrameStyle(QFrame::Panel | QFrame::Raised);
   /* QLabel *Fr12 = new QLabel(this);
    Fr12 -> resize(80,80); Fr12->move(1804,80);
    Fr12 -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *Fr34 = new QLabel(this);
    Fr34 -> resize(80,81); Fr34->move(1804,144);
    Fr34 -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *FrRv = new QLabel(this);
    FrRv -> resize(49,25); FrRv->move(1835,200);
    FrRv -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *Fr56 = new QLabel(this);
    Fr56 -> resize(80,80); Fr56->move(1804,240);
    Fr56 -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *Fr78 = new QLabel(this);
    Fr78 -> resize(80,81); Fr78->move(1804,305);
    Fr78 -> setFrameStyle(QFrame::Panel | QFrame::Raised);*/}
    //контейнеры 3. 4
    {
    QLabel *pixelF1 = new QLabel(this);
    pixelF1 -> resize(49,25); pixelF1->move(1745,80);
    pixelF1 -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *Fr910 = new QLabel(this);
    Fr910 -> resize(80,50); Fr910->move(1714,80);
    Fr910 -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *Fr112 = new QLabel(this);
    Fr112 -> resize(80,81); Fr112->move(1714,144);
    Fr112 -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *FrRv = new QLabel(this);
    FrRv -> resize(49,25); FrRv->move(1745,200);
    FrRv -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    /*QLabel *Fr134 = new QLabel(this);
    Fr134 -> resize(80,80); Fr134->move(1714,240);
    Fr134 -> setFrameStyle(QFrame::Panel | QFrame::Raised);
    QLabel *Fr156 = new QLabel(this);
    Fr156 -> resize(80,81); Fr156->move(1714,305);
    Fr156 -> setFrameStyle(QFrame::Panel | QFrame::Raised);*/}
  /*  QLabel *kletka = new QLabel(this);
    kletka->move(100,100);
    QPixmap *pixelkletka  ;
    pixelkletka = new QPixmap(":/new/prefix1/image/Kletka.png");
    kletka->setPixmap(pixelkletka);*/

}


void CameraView::paintEvent(QPaintEvent *event)
{

       QCameraViewfinder::paintEvent(event);
       QPainter p( this );
       QPen pen(Qt::red);
       QPen penline(Qt::red);
       penline.setWidth(4);
       pen.setWidth(9);
       for (auto &point : m_points) {
           p.setPen(pen);
           p.drawPoint(point);
        }

}

void CameraView::mousePressEvent(QMouseEvent *event){
    event->pos().x(), event->pos().y();
    m_points.push_back(event->pos());

    position = event->pos();
    int xpos = position.x();
    int ypos = position.y();
    XY -> setText (QString::number(position.x())+"\n"+QString::number(position.y()));

if(mouse_press_counter == 0){
        x1 -> setText(QString::number(xpos));
        y1 -> setText(QString::number(ypos));

    }
if(mouse_press_counter == 1){
        x2 -> setText(QString::number(xpos));
        y2 -> setText(QString::number(ypos));

    }
if(mouse_press_counter == 1)
    {
        int rvn = sqrt(pow(x2->text().toInt() - x1->text().toInt(),2.0) + pow(y2->text().toInt() - y1->text().toInt(), 2.0));
        rv -> setText(QString::number(rvn));
        rv1 -> setText(QString::number(rvn/rast));
        converter = rv1->text().toDouble();
        pixcmrv -> setText(QString::number(converter));

    }
if(mouse_press_counter == 2){
        x3 -> setText(QString::number(xpos));
        y3 -> setText(QString::number(ypos));
    }
if(mouse_press_counter == 3){
        x4 -> setText(QString::number(xpos));
        y4 -> setText(QString::number(ypos));
    }
if(mouse_press_counter == 3)
    {
        int rvno =  sqrt(pow(x4->text().toInt() - x3->text().toInt(),2.0) + pow(y4->text().toInt() - y3->text().toInt(), 2.0));
        ravno -> setText(QString::number(rvno));
        ravno1 -> setText(QString::number(rvno/converter));

        agn = ravno1->text().toInt();
    }
{/*if(mouse_press_counter == 4)
{
    x5 -> setText(QString::number(xpos));
    y5 -> setText(QString::number(ypos));
}
if(mouse_press_counter == 5)
{
    x6 -> setText(QString::number(xpos));
    y6 -> setText(QString::number(ypos));
}

if(mouse_press_counter == 6)
{
    x7 -> setText(QString::number(xpos));
    y7 -> setText(QString::number(ypos));
}
if(mouse_press_counter == 7)
{
    x8 -> setText(QString::number(xpos));
    y8 -> setText(QString::number(ypos));
}

if(mouse_press_counter == 8)
{
    x9 -> setText(QString::number(xpos));
    y9 -> setText(QString::number(ypos));
}
if(mouse_press_counter == 9)
{
    x10 -> setText(QString::number(xpos));
    y10 -> setText(QString::number(ypos));
}

if(mouse_press_counter == 10)
{
    x11 -> setText(QString::number(xpos));
    y11 -> setText(QString::number(ypos));
}
if(mouse_press_counter == 11)
{
    x12 -> setText(QString::number(xpos));
    y12 -> setText(QString::number(ypos));
}

if(mouse_press_counter == 12)
{
    x13 -> setText(QString::number(xpos));
    y13 -> setText(QString::number(ypos));
}
if(mouse_press_counter == 13)
{
    x14 -> setText(QString::number(xpos));
    y14 -> setText(QString::number(ypos));
}
if(mouse_press_counter == 14)
{
    x15 -> setText(QString::number(xpos));
    y15 -> setText(QString::number(ypos));
}
if(mouse_press_counter == 15)
{
    x16 -> setText(QString::number(xpos));
    y16 -> setText(QString::number(ypos));

}*/}
if(mouse_press_counter == 16)
{

}
if(mouse_press_counter == 17)
{

}
if(mouse_press_counter == 17)
{


}
if(mouse_press_counter <=22)
{
    mouse_press_counter++;
}

else
    {
        mouse_press_counter = 0;
    }
}
//mouse_press_counter++;
