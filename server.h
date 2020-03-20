#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include "pthread.h"
#include <ctime>
#include <unistd.h>

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QRegExpValidator>
#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QDebug>

#include <receiver.h>
#include <writter.h>
#include <led.h>

#define IP_ADDR "192.168.0.29"
#define IP_PORT 49001
#define BUFFER_SIZE 1441
#define STORAGE_SIZE 213700000

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QMainWindow
{
    Q_OBJECT

    int initServer();

    void checkFile();
    void validateTimeStr();
    void setIndicatorsDefaultState();
    void reset();

public:
    Server(QWidget *parent = nullptr);
    ~Server();

public slots:
    void start();
    void stop();
    void changeTime();
    void selectFile(const QString &);

private slots:

    void on_startButton_clicked();

    void on_stopButton_clicked();

    void on_resetButton_clicked();

    void on_timeEdit_textChanged(const QString &arg1);

    void on_selectFileButton_clicked();

    void writtingFinished();

    void writtingStarted();

private:

    ofstream file;
    QFileDialog * fileDialogWindow;

    Ui::Server *ui;

    bool initialized;

    SOCKET serverSocket;

    int bytesReceived;
    long long totalBytes;
    long long totalDatagrams;

    int delay;
    unsigned short receiveCounter;
    unsigned short writeCounter;

    char * buffer;

    bool received;
    bool done;
    bool rcv_stop;

    int time;

    QString timeStr;
    QString fileName;
    QString port;

    QTimer receiveTimer;
    QTimer viewChangeTimer;

    QThread writtingThread;
    QThread receivingThread;

    Receiver * receiver;
    Writter * writter;

    QRegExpValidator timeValidator;
};
#endif // SERVER_H
