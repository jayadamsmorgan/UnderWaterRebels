#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QString directionDetermine(float angle);
    ~MainWindow();
    QCamera *camera;
    QCameraImageCapture *capture;
    QPixmap standartpic;
    QPixmap mappixmap;
    QGraphicsScene *scene;


private:
    Ui::MainWindow *ui;
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
private slots:
    void on_pushButton_clicked();
    void onProcessCature(int requestId, const QImage &img);
    void on_tabWidget_currentChanged(int index);
    void on_knownsize_editingFinished();
};

#endif // MAINWINDOW_H
