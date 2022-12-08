#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QSerialPort>
#include <QObject>

class SerialPort : public QSerialPort
{
    Q_OBJECT

public:
    SerialPort(QObject *parent = nullptr);
    bool available(void);
    bool connectSerial(QString, QSerialPort::BaudRate);
    void disconnectSerial(void);

signals:
    void receivedBytes(QByteArray);

private:
    QByteArray tempBuf{};
};

#endif // SERIALPORT_H
