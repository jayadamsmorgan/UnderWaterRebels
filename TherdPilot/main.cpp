#include "therdpilot.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");
    TherdPilot w;
    w.show();
    return a.exec();
}
