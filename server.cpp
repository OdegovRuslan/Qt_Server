#include "server.h"
#include "ui_server.h"

Server::Server(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Server)
{
    ui->setupUi(this);

    QString regExpTimeLine = "^[0-9]{2}:[0-5][0-9]:[0-5][0-9]$";
    QRegExp timeRegExp(regExpTimeLine);
    timeValidator.setRegExp(timeRegExp);

    ui->timeEdit->setValidator(&timeValidator);

    timeStr = "";
    time = 0;
    initialized = false;

    bytesReceived = 0;
    totalBytes = 0;
    totalDatagrams = 0;

    delay = 0;
    receiveCounter = 0;
    writeCounter = 0;              //bug

    buffer = new char[STORAGE_SIZE];

    received = false;
    done = true;
    rcv_stop = false;

    connect(&viewChangeTimer, &QTimer::timeout, this, &Server::changeTime);

    connect(&receiveTimer, &QTimer::timeout, this, &Server::stop);

    viewChangeTimer.setInterval(1000);
    viewChangeTimer.start();

    struct ReceiveParams rParams = {
        &serverSocket,
        &bytesReceived,
        &totalBytes,
        &totalDatagrams,
        buffer,
        &receiveCounter,
        &delay,
        &rcv_stop
    };

    struct WriteParams wParams = {
        &file,
        buffer,
        &receiveCounter,
        &writeCounter,
        &delay,
        &done,
        &received,
    };

    fileName = QDir::currentPath() + "/output";
    ui->fileNameLineEdit->setText(fileName);
    receiver = new Receiver(rParams);
    writter = new Writter(wParams);

    receiver->moveToThread(&receivingThread);
    writter->moveToThread(&writtingThread);

    fileDialogWindow = new QFileDialog(this);
    connect(fileDialogWindow, &QFileDialog::fileSelected, this, &Server::selectFile);

    connect(&receivingThread, &QThread::started, ui->receiveState, &Led::startProcessing);
    connect(&receivingThread, &QThread::started, receiver, &Receiver::receive);
    connect(receiver, &Receiver::startReceiving, writter, &Writter::write);
    connect(receiver, &Receiver::startReceiving, ui->writeState, &Led::startProcessing);
    connect(receiver, &Receiver::startReceiving, this, &Server::writtingStarted);

    connect(&receivingThread, &QThread::finished, ui->receiveState, &Led::stopProcessing);
    connect(&writtingThread, &QThread::finished, ui->writeState, &Led::stopProcessing);
    connect(&writtingThread, &QThread::finished, this, &Server::writtingFinished);

    ui->stopButton->setDisabled(true);
    ui->infoEdit->setFocus();

    ui->infoEdit->append("Вас приветствует программа приёма данных Ethernet!");
    ui->infoEdit->append("Для успешной работы сервера необходимо, чтобы ваш сетевой адаптер был настроен верно:\nIP-адрес: 192.168.1.12");
    ui->infoEdit->append("---------------------------------------------------------");
    ui->infoEdit->append("Алгоритм работы:");
    ui->infoEdit->append("1. Выберите файл для записи");
    ui->infoEdit->append("2. Введите время работы сервера");
    ui->infoEdit->append("3. Введите порт приложения");
    ui->infoEdit->append("4. Нажмите кнопку \"Старт\"");
    ui->infoEdit->append("Для остановки приёма нажмите кнопку\"Стоп\"");
    ui->infoEdit->append("Когда сервер заканчивает приём, дождитесь, пока запишутся все данные в файл");
    ui->infoEdit->append("Кнопка \"Сброс\" сбрасывает параметры сервера");
    ui->infoEdit->append("---------------------------------------------------------");
}

Server::~Server()
{
    delete ui;
}

int Server::initServer()
{
    WSADATA ws;

    int err = WSAStartup(MAKEWORD (2,2), &ws);
    if (err != 0){
        ui->infoEdit->append("Ошибка инициализации Win API! Код ошибки: " + QString::number(WSAGetLastError()));
        return -1;
    }
    ui->infoEdit->append("Win API успшено инициализировано!");

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET){
        ui->infoEdit->append("Ошибка создания сокета! Код ошибки: " + QString::number(WSAGetLastError()));
        ui->sockState->setState(ERROR_STATE);
        WSACleanup();
        return -2;
    }
    ui->infoEdit->append("Сокет успешно создан!");
    ui->sockState->setState(OK_STATE);
    cout << "Socket initializing" << endl;

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port.toInt());
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

    if (bind(serverSocket, (sockaddr *) &server_addr, sizeof(server_addr)) == SOCKET_ERROR){
        ui->infoEdit->append("Ошибка привязки сокета к IP-адресу и порту! Код ошибки: " + QString::number(WSAGetLastError()));
        ui->bindState->setState(ERROR_STATE);
        closesocket (serverSocket);
        return -3;
    }
    ui->infoEdit->append("Сокет успешно привязан к IP-адресу и порту");
    ui->bindState->setState(OK_STATE);
    cout << "Binding OK" << endl;

    initialized = true;
    return 0;
}

void Server::start()
{

}

void Server::stop()
{
    receiveTimer.stop();
    received = true;
    initialized = false;
    rcv_stop = true;
    WSACancelBlockingCall();
    closesocket(serverSocket);
    receivingThread.exit();
    ui->receiveState->setState(OK_STATE);
    ui->infoEdit->append("---------------------------------------------------------");
    ui->infoEdit->append("Данные получены!");
    ui->infoEdit->append("Байт: " + QString::number(totalBytes));
    ui->infoEdit->append("Датаграм: " + QString::number(totalDatagrams));

    ui->startButton->setDisabled(false);
    ui->resetButton->setDisabled(false);
    ui->stopButton->setDisabled(true);
    ui->timeEdit->setReadOnly(false);

    timeStr = "";
    validateTimeStr();
}

void Server::changeTime()
{
    if (!initialized || time == 0) return;

    time--;

    int t = time;

    QString hours = QString::number(t / 3600);
    t -= hours.toInt() * 3600;
    QString mins = QString::number(t / 60);
    t -= mins.toInt() * 60;
    QString secs = QString::number(t);

    if (hours.length() == 1) hours = "0" + hours;
    if (mins.length() == 1) mins = "0" + mins;
    if (secs.length() == 1) secs = "0" + secs;

    timeStr = hours + ":" + mins + ":" + secs;

    ui->timeEdit->setText(timeStr);
}

void Server::selectFile(const QString &file)
{
    fileName = file;
    ui->fileNameLineEdit->setText(fileName);
}


void Server::on_startButton_clicked()
{
    if (!done) {
        ui->infoEdit->append("Сервер еще не готов к работе! Подождите, когда завершится запись данных в файл!");
        return;
    }
    reset();
    ui->infoEdit->setFocus();
    received = false;
    rcv_stop = false;
    validateTimeStr();
    if (!time) {
        ui->infoEdit->append("Пожалуйста, укажите время, отличное от 00:00:00");
        return;
    }

    fileName = ui->fileNameLineEdit->text();
    file.open(fileName.toUtf8().data());
    qDebug() << fileName;
    if (!file.is_open()) {
        ui->infoEdit->append("Невозможно открыть файл для записи. Пожалуйста, выберите другой файл!");
        ui->fileState->setState(ERROR_STATE);
        return;
    }
    file << "Begin";
    ui->infoEdit->append("Открываю файл для записи!");
    ui->fileState->setState(OK_STATE);
    port = ui->portEdit->text();
    initServer();
    done = false;
    if (!initialized) {
        done = true;
        return;
    }

    receiveTimer.start(time * 1000);

    receivingThread.start();
    writtingThread.start();

    ui->infoEdit->append("Начинаю принимать данные!");

    ui->startButton->setDisabled(true);
    ui->resetButton->setDisabled(true);
    ui->stopButton->setDisabled(false);
    ui->timeEdit->setReadOnly(true);
}

void Server::on_stopButton_clicked()
{
    ui->infoEdit->append("Остановка работы...");
    stop();
    ui->infoEdit->setFocus();
}

void Server::on_resetButton_clicked()
{
    ui->infoEdit->setFocus();
    if(!done || writtingThread.isRunning()) return;

    reset();

    timeStr = "";
    validateTimeStr();
    ui->portEdit->setText("49001");
}

void Server::checkFile()
{
    setlocale(LC_ALL, "rus");
    ifstream file;
    file.open(fileName.toUtf8().data(), ios_base::in);
    FILE * fp = fopen("output_num_lose.txt", "w");
    FILE * pr = fopen("output_num_repeat.txt", "w");

    if(!file.is_open())
        cout << "nnn!\n";

    char *tmp = new char[9];
    char *buffer = new char[1441];

    file.read(tmp, 8);
    int nmin = atoi(tmp);
    int n =0;
    int counter1 = 0;
    int counter2 = 0;
    int counter3 = 0;

    file.close();
    file.open(fileName.toUtf8().data(), ios_base::in);

    while(file.eof() == 0){
        file.read(tmp, 8);
        file.read(buffer, 1432);
        int num = atoi(tmp);
        if(num > n)
            n = num;
    }

    int *br = new int[n+2];
    memset(br, 0, n+2);
    file.close();
    file.open(fileName.toUtf8().data(), ios_base::in);

    if(!file.is_open())
        cout << "asd!\n";

    while(file.eof() == 0){
        file.read(tmp, 8);
        file.read(buffer, 1432);
        int num = atoi(tmp);
        br[num]++;
    }

    br[n] = br[n] - 1;

    for(int i = nmin; i <= n; i++){

        if(br[i] == 0){
            fprintf(fp, "[%d, ", i);
            while(br[i] == 0){
                counter1++;
                if(br[i+1] == 0){
                    i++;
                }else{break;}
            }
            fprintf(fp, "%d]\n", i);
        }

        if(br[i] > 1){
            fprintf(pr, "[%d, ", i);
            while(br[i] > 1){
                counter2 = counter2 + br[i] - 1;
                if(br[i+1] > 0){
                    i++;
                }else{break;}
            }
            fprintf(pr, "%d]\n", i);
        }

        if(br[i] == 1){
            counter3++;
        }
    }
    delete [] tmp;
    delete [] buffer;
    delete [] br;
    file.close();
    fclose(fp);
    fclose(pr);
}

void Server::validateTimeStr()
{
    int pos = 0;
    if (timeValidator.validate(timeStr, pos) != QValidator::Acceptable) {
        for (int curLength = timeStr.length(); curLength < 8; curLength++) {
            if(curLength == 2 || curLength == 5)
                timeStr += ":";
            else
                timeStr += "0";
        }
        ui->timeEdit->setText(timeStr);
    }
    QStringList timeValues = timeStr.split(":");
    time = timeValues[0].toInt() * 3600 + timeValues[1].toInt() * 60 + timeValues[2].toInt();
}

void Server::setIndicatorsDefaultState()
{
    ui->fileState->setState(DISABLE_STATE);
    ui->sockState->setState(DISABLE_STATE);
    ui->bindState->setState(DISABLE_STATE);
    ui->receiveState->setState(DISABLE_STATE);
    ui->writeState->setState(DISABLE_STATE);
    ui->checkState->setState(DISABLE_STATE);
}

void Server::reset()
{
    initialized = false;

    bytesReceived = 0;
    totalBytes = 0;
    totalDatagrams = 0;
    delay = 0;
    receiveCounter = 0;
    writeCounter = 0;
    received = false;
    rcv_stop = false;

    setIndicatorsDefaultState();
}

void Server::on_timeEdit_textChanged(const QString &arg1)
{
    if ((timeStr.length() < arg1.length()) && (arg1.length() == 2 || arg1.length() == 5))
        ui->timeEdit->setText(arg1 + ":");
    timeStr = arg1;

}

void Server::on_selectFileButton_clicked()
{
    fileDialogWindow->show();
    ui->infoEdit->setFocus();
}

void Server::writtingFinished()
{
    ui->writeState->setState(OK_STATE);
    ui->infoEdit->append("Данные записаны на диск!");
    ui->infoEdit->append("Работа завершена!");
    ui->infoEdit->append("---------------------------------------------------------");
    file.close();
}

void Server::writtingStarted()
{
    ui->infoEdit->append("Начинаю запись в файл!");
}
