#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name();

    a.setFont(QFont("Calibri", 14));

    QTranslator ts;
    QTranslator ts1;

    qDebug() << "Locale: " << locale;

    //if (locale.startsWith("zh"))
    //{
       ts.load("MJ_Demo_zh_CN.qm");
       ts1.load("qt_zh_CN.qm");

        a.installTranslator(&ts1);
        a.installTranslator(&ts);
    //}

    MainWindow w;


    w.show();

    return a.exec();
}
