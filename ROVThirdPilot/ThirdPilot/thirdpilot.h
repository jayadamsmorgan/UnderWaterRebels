#ifndef THIRDPILOT_H
#define THIRDPILOT_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QResizeEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QPaintEvent>
#include "cameraview.h"
#include <QCameraViewfinder>
#include <QMainWindow>
#include <QPixmap>

class QCamera;
class QCameraViewfinder;
class QCameraImageCapture;
class QVBoxLayout;
class QPushButton;
class QObject;

class thirdpilot : public QMainWindow
{
    Q_OBJECT

public:
    thirdpilot(QWidget *parent = 0);
    ~thirdpilot();

protected:
  void mousePressEvent(QMouseEvent *event) override;

private:
    QLabel *image;
    CameraView *mCameraV;
    QCamera *mCamera;
    QPoint position;
    QLabel *updL;
    QVBoxLayout *mLayout;
    QPushButton *StartB;
    QPushButton *StopB;
    QPushButton *updB;
    void onMouseEvent(const QPoint &pos);



public slots:
    void timeout();
};

#endif // THIRDPILOT_H
