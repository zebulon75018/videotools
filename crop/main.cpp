#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    // si on veut ouvrir une vidéo en paramètre:
    if (argc > 1) {
        w.openVideo(QString::fromLocal8Bit(argv[1]));
    }

    return a.exec();
}

