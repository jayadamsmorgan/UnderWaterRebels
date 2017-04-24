#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QWidget>
#include <QCameraViewfinder>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>

class CameraView : public QCameraViewfinder
{
    Q_OBJECT
public:
    explicit CameraView(QWidget *parent = 0);

signals:

public slots:
private slots:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QLabel *rcont;//растояние от 1 до 2 контейнера
    QLabel *XY; QLabel *xy; //координаты
    QLabel *x1; QLabel *y1; //1 точка
    QLabel *x2; QLabel *y2; //2 точка
    QLabel *x3; QLabel *y3; //3 точка
    QLabel *x4; QLabel *y4; //4 точка
    QLabel *x5; QLabel *y5; //5 точка
    QLabel *x6; QLabel *y6; //6 точка
    QLabel *x8; QLabel *y7; //7 точка
    QLabel *x7; QLabel *y8; //8 точка
    QLabel *x9; QLabel *y9; //1 точка
    QLabel *x10; QLabel *y10; //2 точка
    QLabel *x11; QLabel *y11; //3 точка
    QLabel *x12; QLabel *y12; //4 точка
    QLabel *x13; QLabel *y13; //5 точка
    QLabel *x14; QLabel *y14;//6 точка
    QLabel *x15; QLabel *y15; //7 точка
    QLabel *x16; QLabel *y16; //8 точка
    QLabel *rv; QLabel *rv1; //расстояние от 1 точки до 2
    QLabel *rv2; QLabel *rv3; //расстояние от 3 точки до 4
    QLabel *ravno; QLabel *ravno1; //расстояние от 3 точки до 4
    QLabel *ravno2; QLabel *ravno3;
    QLabel *rv2xy; QLabel *rv2xy1; //расстояние от 5 точки до 6
    QLabel *rv3xy; QLabel *rv3xy1; //расстояние от 7 точки до 8
    QLabel *rv2xy2; QLabel *rv2xy3; //расстояние от 5 точки до 6
    QLabel *rv3xy2; QLabel *rv3xy3; //расстояние от 7 точки до 8
    QLabel *updL;
    QPushButton *updB;
    QLabel *pixcmrv;
    QLabel *pixcmrv2;
    QLabel *image;
    CameraView *mCameraV;
    QCamera *mCamera;
    QPoint position;

    QVector<QPoint> m_points;
};

#endif // CAMERAVIEW_H
