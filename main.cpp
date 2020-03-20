#include "server.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{    



    QApplication a(argc, argv);
    Server w;
    w.show();
    return a.exec();
}
