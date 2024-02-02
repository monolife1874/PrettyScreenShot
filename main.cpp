#include "QGrabScreenImage.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGrabScreenImage w;
    w.show();
    return a.exec();
}
