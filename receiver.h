#ifndef RECEIVER_H
#define RECEIVER_H

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include "pthread.h"
#include <ctime>
#include <unistd.h>

#include <QObject>
#include <QThread>

using namespace std;

struct ReceiveParams {
    SOCKET * serverSocket;

    int * bytesReceived;
    long long * totalBytes;
    long long * totalDatagrams;

    char * buffer;

    unsigned short * receiveCounter;
    int * delay;
    bool * term;
};

class Receiver : public QObject
{
    Q_OBJECT

    bool triggered;
    struct ReceiveParams params;
public:
    explicit Receiver(QObject *parent = nullptr);
    Receiver(struct ReceiveParams, QObject *parent = nullptr);

public slots:
    void receive();

signals:
    void startReceiving();
};

#endif // RECEIVER_H
