#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QCameraImageCapture>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QCamera *camera;
    QCameraImageCapture *capture;
    QPixmap standartpic;
    QPixmap mappixmap;

private:
    Ui::MainWindow *ui;
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
private slots:
    void on_pushButton_clicked();
    void onProcessCature(int requestId, const QImage &img);
};

#endif // MAINWINDOW_H
