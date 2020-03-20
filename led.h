#ifndef LED_H
#define LED_H

#include <QString>
#include <QLabel>
#include <QTimer>

enum LedState { OK_STATE, ERROR_STATE, PROCESSING_1_STATE, PROCESSING_2_STATE, DISABLE_STATE };

static const QString okState = "background-color: #2ED03C;";
static const QString errorState = "background-color: red;";
static const QString processing1State = "background-color: blue;";
static const QString processing2State = "background-color: darkblue;";
static const QString disableState = "background-color: #888;";
static const QString baseStyle = "border: 2px solid #fff; border-radius: 10px;";

class Led : public QLabel
{
    Q_OBJECT

    QTimer processingTimer;
    LedState state;
public:
    explicit Led(QWidget * parent = 0);


public slots:
    void setState(LedState);
    void startProcessing();
    void processing();
    void stopProcessing();
};

#endif // LED_H
