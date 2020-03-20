#ifndef WRITTER_H
#define WRITTER_H

#include <fstream>
#include <ctime>
#include <unistd.h>

#include <QString>
#include <QObject>
#include <QThread>

using namespace std;

struct WriteParams {
    ofstream * file;
    char * buffer;

    unsigned short * receiveCounter;
    unsigned short * writeCounter;
    int * delay;

    bool * done;
    bool * received;
};

class Writter : public QObject
{
    Q_OBJECT

    struct WriteParams params;
public:
    explicit Writter(QObject *parent = nullptr);
    Writter(struct WriteParams, QObject *parent = nullptr);

    void write();
signals:

};

#endif // WRITTER_H
