#include "writter.h"
#include <QDebug>

Writter::Writter(QObject *parent) : QObject(parent) {

}

Writter::Writter(struct WriteParams _p, QObject * parent) : QObject(parent) {
    params.file = _p.file;
    params.buffer = _p.buffer;
    params.receiveCounter = _p.receiveCounter;
    params.writeCounter = _p.writeCounter;
    params.delay = _p.delay;
    params.done = _p.done;
    params.received = _p.received;
}

void Writter::write() {
    char tmp[1441];
    tmp[1440] = '\0';
    while (1) {
        if (((*params.writeCounter < *params.receiveCounter) && (*params.delay == 0)) || (*params.delay > 0)) {
            strncpy(tmp, &params.buffer[*params.writeCounter * 1440], 1440);
            (*params.file) << tmp;

            (*params.writeCounter)++;
            if ((*params.writeCounter) == 0) {
                (*params.delay)--;
                params.writeCounter = 0;
            }

        } else {
            if (!*params.received)
                usleep(500);
            else {
                break;
            }
        }
    }

    (*params.done) = true;
    this->thread()->quit();
}


