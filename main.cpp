#include "umams.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    UMAMS w;
    w.show();

    return a.exec();
}