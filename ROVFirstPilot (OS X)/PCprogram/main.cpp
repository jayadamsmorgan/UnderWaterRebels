#include "mainwindow.h"
#include <QApplication>
#include <QTimer>
#include <worker.h>
#include <QThread>
#undef main

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;


    Worker worker;
    QThread thread;
    QTimer timer;

    QObject::connect(&timer,SIGNAL(timeout()),&worker,SLOT(onTimeOut()));
    QObject::connect(&worker,SIGNAL(updateInfo(int,int,int,int,int,int,int,int,bool,bool,bool,bool,bool)),&w,SLOT(onupdateInfo(int,int,int,int,int,int,int,int,bool,bool,bool,bool,bool)));
    timer.start(200);

    timer.moveToThread(&thread);
    worker.moveToThread(&thread);

    thread.start();

    w.show();

    return a.exec();
}
