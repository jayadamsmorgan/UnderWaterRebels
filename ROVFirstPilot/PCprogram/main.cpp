#include <worker.h>
#include "mainwindow.h"
#include <QApplication>
#include <QTimer>
#include <QThread>
#undef main

int main(int argc, char *argv[])
{
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);

    QApplication a(argc, argv);
    MainWindow w;


    Worker worker;
    QThread thread;
    QTimer timer;

    QObject::connect(&timer,SIGNAL(timeout()),&worker,SLOT(onTimeOut()));
    QObject::connect(&worker,SIGNAL(updateInfo(int,int,int,int,int,int,int,int,bool,bool,bool,bool,bool)),&w,SLOT(onupdateInfo(int,int,int,int,int,int,int,int,bool,bool,bool,bool,bool)));
    QObject::connect(&w,SIGNAL(updateJoy()),&worker,SLOT(onUpdateJoy()));
    timer.start(25);

    timer.moveToThread(&thread);
    worker.moveToThread(&thread);

    thread.start();

    w.show();

    return a.exec();
}
