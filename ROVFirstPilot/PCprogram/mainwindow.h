#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QPixmap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setpalprop();
    QCamera *camera;
    QPixmap *red;
    QPixmap *green;
    QPalette *okauto;
    QPalette *noauto;
private:
    Ui::MainWindow *ui;
public slots:
    void onupdateInfo(int ping,int depth,int speed,bool connection,bool yaw,bool pitch,bool roll);

};

#endif // MAINWINDOW_H
