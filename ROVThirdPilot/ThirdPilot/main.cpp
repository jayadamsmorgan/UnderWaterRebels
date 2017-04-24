#include "thirdpilot.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    thirdpilot w;
    w.show();

    return a.exec();
}
