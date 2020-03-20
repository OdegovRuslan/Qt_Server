#include "led.h"

Led::Led(QWidget * parent) : QLabel(parent)
{
    state = DISABLE_STATE;
    setFixedSize(20, 20);
    setState(DISABLE_STATE);
    setText("");
    processingTimer.setInterval(1000);
    processingTimer.start();
}

void Led::setState(LedState state)
{
    this->state = state;
    switch (state) {
        case LedState::OK_STATE:
            setStyleSheet(QString(baseStyle + okState));
            break;
        case LedState::ERROR_STATE:
            setStyleSheet(QString(baseStyle + errorState));
            break;
        case LedState::PROCESSING_1_STATE:
            setStyleSheet(QString(baseStyle + processing1State));
            break;
        case LedState::PROCESSING_2_STATE:
            setStyleSheet(QString(baseStyle + processing2State));
            break;
        case LedState::DISABLE_STATE:
            setStyleSheet(QString(baseStyle + disableState));
            break;
        default:
            break;
    }
}

void Led::startProcessing()
{
    connect(&processingTimer, &QTimer::timeout, this, &Led::processing);
}

void Led::processing()
{
    state = (state == PROCESSING_1_STATE) ? PROCESSING_2_STATE : PROCESSING_1_STATE;
    setState(state);
}

void Led::stopProcessing()
{
    disconnect(&processingTimer, &QTimer::timeout, this, &Led::processing);
}

