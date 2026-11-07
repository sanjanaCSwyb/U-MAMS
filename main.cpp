#include "umams.h"
#include "figma_ui.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");
    FigmaUi::loadFont();

    QFile styleFile(":/umams.qss");

    qDebug() << "QSS exists:" << styleFile.exists();

    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());

        qDebug() << "QSS loaded successfully";
        qDebug() << "QSS size:" << styleSheet.size();

        a.setStyleSheet(styleSheet);

        styleFile.close();
    }
    else
    {
        qDebug() << "QSS FAILED TO OPEN";
    }

    UMAMS w;
    w.show();

    return a.exec();
}