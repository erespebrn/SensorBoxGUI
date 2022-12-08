#include "serialport.h"
#include <QDebug>



SerialPort::SerialPort(QObject *parent) : QSerialPort(parent)
{

}

bool SerialPort::connectSerial(QString com, QSerialPort::BaudRate br)
{
    this->setPortName(com);
    this->setBaudRate(br);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);
    this->open(QSerialPort::ReadWrite);

    return this->isOpen();
}

void SerialPort::disconnectSerial()
{
    this->close();
}
