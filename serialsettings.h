#ifndef SERIALSETTINGS_H
#define SERIALSETTINGS_H

#include <QDialog>
#include <QSerialPortInfo>
#include "serialport.h"
#include "sensorboxgui.h"
#include <QCloseEvent>
namespace Ui {
class SerialSettings;
}

class SerialSettings : public QDialog
{
    Q_OBJECT

public:
    explicit SerialSettings(SensorBoxGUI *parent = nullptr, SerialPort *ar = nullptr, QString *com = nullptr, QSerialPort::BaudRate *br = nullptr);
    ~SerialSettings();

signals:
    void connectionSuccesfull(bool);

private slots:
    void on_applyButton_clicked();
    void on_cancelButton_clicked();

    void closeEvent(QCloseEvent *ev) override;

private:
    Ui::SerialSettings *ui;

    SerialPort *arduino;
    SensorBoxGUI *parent{};

    QString *comPort{};
    QSerialPort::BaudRate *baudRate{};

    QString configFile_path{};

    bool success{};
};

#endif // SERIALSETTINGS_H
