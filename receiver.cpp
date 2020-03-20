#include "receiver.h"
#include <QDebug>

Receiver::Receiver(QObject *parent) : QObject(parent) {

}

Receiver::Receiver(struct ReceiveParams _p, QObject * parent) : QObject(parent) {
    params.serverSocket = _p.serverSocket;
    params.bytesReceived = _p.bytesReceived;
    params.totalBytes = _p.totalBytes;
    params.totalDatagrams = _p.totalDatagrams;
    params.buffer = _p.buffer;
    params.receiveCounter = _p.receiveCounter;
    params.delay = _p.delay;
    params.term = _p.term;
    triggered = false;
}

void Receiver::receive() {
    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    while(!(*params.term)) {
            (*params.bytesReceived) = recvfrom(*params.serverSocket, params.buffer + (*params.receiveCounter * 1440), 1440, 0, (sockaddr *)&client_addr, &client_addr_size);
            if(*params.bytesReceived == SOCKET_ERROR){
                if (WSAGetLastError() == 10004) break;
                cout << "Package not accepted: " << WSAGetLastError() << endl;
                continue;
            }

            if (!(*params.totalBytes)) {
                triggered = true;
                emit startReceiving();
            }

            (*params.totalBytes) += *params.bytesReceived;
            (*params.totalDatagrams)++;

            (*params.receiveCounter)++;
            if (*params.receiveCounter == 0) {
                (*params.delay)++;
                (*params.receiveCounter) = 0;
            }
    }
    if (!triggered)
        emit startReceiving();
}
