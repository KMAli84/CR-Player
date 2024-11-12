#include "mainwindow.h"

#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QRect screen = QGuiApplication::primaryScreen()->geometry();
    int x = (screen.width() - w.width()) / 2;
    int y = (screen.height() - w.height()) / 2;

    w.move(x, y - 45);

    w.show();
    return a.exec();
}
