#include "serialsettings.h"
#include "ui_serialsettings.h"
#include <QMessageBox>
#include <QDebug>

SerialSettings::SerialSettings(SensorBoxGUI *parent, SerialPort *ar, QString *com, QSerialPort::BaudRate *br)
    : QDialog(parent), ui(new Ui::SerialSettings), arduino(ar), parent(parent), comPort(com), baudRate(br)
{
    ui->setupUi(this);

    this->setWindowTitle("Connect Serial Port");

    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()){
        ui->portComboBox->addItem(port.portName());
    }   

    ui->baudRateComboBox->addItem("9600");
    ui->baudRateComboBox->addItem("38400");
    ui->baudRateComboBox->addItem("57600");
    ui->baudRateComboBox->addItem("115200");
}

SerialSettings::~SerialSettings()
{
    delete ui;
}

void SerialSettings::on_applyButton_clicked()
{
    switch (ui->baudRateComboBox->currentIndex())
    {
        case 0:
            *baudRate = QSerialPort::Baud9600;
        break;
        case 1:
            *baudRate = QSerialPort::Baud38400;
        break;
        case 2:
            *baudRate = QSerialPort::Baud57600;
        break;
        case 3:
            *baudRate = QSerialPort::Baud115200;
        break;
    }

    if(!arduino->isOpen()){
        if(arduino->connectSerial(ui->portComboBox->currentText(), *baudRate)){
            emit connectionSuccesfull(true);

            ui->statusLabel->setText("Status: Connection successful");
            success = true;

            this->close();

        }else{
            ui->statusLabel->setText("Status: Connection failed");
            success = false;
        }
    }else{
        arduino->disconnectSerial();
        if(arduino->connectSerial(ui->portComboBox->currentText(), *baudRate)){
            emit connectionSuccesfull(true);

            ui->statusLabel->setText("Status: Connection successful");
            success = true;

            this->close();

        }else{
            ui->statusLabel->setText("Status: Connection failed");
            success = false;
        }
    }

}


void SerialSettings::on_cancelButton_clicked()
{
    this->close();

}

void SerialSettings::closeEvent(QCloseEvent *ev)
{
    if(!success){
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Connect Serial Port" ,"Serial port connection failed. The GUI will be closed.\n"
                                                                        "Do you want to continue?",
                                                                      QMessageBox::No | QMessageBox::Yes);
        if(resBtn == QMessageBox::Yes){
            emit connectionSuccesfull(false);
            ev->accept();
        }else{
            ev->ignore();
        }

    }

}

