#ifndef THERDPILOT_H
#define THERDPILOT_H

#include <QWidget>
#include <QPushButton>
#include <QWidget>
#include <QTimer>
#include <QResizeEvent>
#include "paintscene.h"


namespace Ui{
class TherdPilot;
}

class QSpinBox;
class QCamera;
class QCameraViewfinder;
class QCameraImageCapture;
class QVBoxLayout;
class QPushButton;
class QObject;

class TherdPilot : public QWidget
{
    Q_OBJECT

public:
    explicit TherdPilot(QWidget *parent = 0);
    ~TherdPilot();
protected:

   // void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::TherdPilot *ui;
    QCamera *mCamera;
    QCameraViewfinder *mCameraViewfinder;
    QCameraImageCapture *mCameraImageCapture;
    QVBoxLayout *mLayout;
    QPushButton *StartB;
    QPushButton *StopB;
    QPushButton *SavePB;
    void confSpin(QSpinBox *spinBox, int min, int max) const;
    void onMouseEvent(const QPoint &pos);
    QTimer *timer;
    paintScene *scene;
    void resizeEvent(QResizeEvent * event);
private slots:
    void slotTimer();

};

#endif // THERDPILOT_H
