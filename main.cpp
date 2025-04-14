#include "YGIS.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    YGIS w;
    w.setFixedSize(800, 600);
    w.show();
    return a.exec();
}
