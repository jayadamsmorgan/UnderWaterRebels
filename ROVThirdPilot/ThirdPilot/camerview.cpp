#include "cameraview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <cmath>
#include <QPen>

CameraView::CameraView(QWidget *parent) : QCameraViewfinder(parent){
}

void CameraView::paintEvent(QPaintEvent *event)
{
       QCameraViewfinder::paintEvent(event);
       QPainter p( this );
       QPen pen(Qt::green);
       pen.setWidth(6);
       for (auto &point : m_points) {
           p.setPen(pen);
           p.drawEllipse(point,1,1);
        }
}

void CameraView::mousePressEvent(QMouseEvent *event){
    event->pos().x(), event->pos().y();
    m_points.push_back(event->pos());
}
