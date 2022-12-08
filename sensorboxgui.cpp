#include "sensorboxgui.h"
#include "ui_sensorboxgui.h"
#include "serialsettings.h"
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QToolButton>
#include <QSpinBox>
#include <QLCDNumber>
#include <QStatusBar>
#include <QDebug>
#include <QMessageBox>

#include <cmath>

SensorBoxGUI::SensorBoxGUI(QWidget *parent) : QMainWindow(parent), ui(new Ui::SensorBoxGUI)
{
    ui->setupUi(this);

    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    this->setCentralWidget(mainWidget);

    mainLayout->addWidget(createPortSettingsWidget());

    QWidget *lowerWidget = new QWidget(mainWidget);
    QHBoxLayout *lowerGrid = new QHBoxLayout(lowerWidget);

    lowerGrid->addWidget(createControlsWidget());
    lowerGrid->addWidget(createDisplayResultsWidget());

    mainLayout->addWidget(lowerWidget);
    mainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QStatusBar *statusBar = new QStatusBar(this);
    this->setStatusBar(statusBar);

    serialPort = new SerialPort(this);

    this->adjustSize();
}

SensorBoxGUI::~SensorBoxGUI()
{
    delete ui;
}


void SensorBoxGUI::initSerial(void)
{
    SerialSettings settings(nullptr, serialPort, &comPort, &br);
    connect(&settings, &SerialSettings::connectionSuccesfull, this, &SensorBoxGUI::serialConnectSuccess);
    settings.setWindowFlags(settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    settings.exec();
}

void SensorBoxGUI::serialConnectSuccess(bool sc)
{
    if(sc){
        connect(serialPort, &QSerialPort::readyRead, this, &SensorBoxGUI::onSerialReceived);
    }else{
        this->close();
    }
}


void SensorBoxGUI::onSetConfigClicked(bool en)
{
    Q_UNUSED(en);
}

void SensorBoxGUI::onStartButtonClicked(bool en)
{
    Q_UNUSED(en);
    this->statusBar()->showMessage("Measurement in progress...");
    measProcessState = DISABLE_PORT_1;
    this->measProcessStateMachine();
}

void SensorBoxGUI::onStopButtonClicked(bool en)
{
    Q_UNUSED(en);
    this->statusBar()->showMessage("Measurement stopped");
    port0LCD->display(QString::number(0));
    port1LCD->display(QString::number(0));
    lastSample = 0;
    measProcessState = MEAS_IDLE;
    this->measProcessStateMachine();
}

void SensorBoxGUI::measProcessStateMachine(void)
{
    QByteArray txData;

    switch (measProcessState) {


    case SensorBoxGUI::MEAS_IDLE:
    {
        break;
    }
    case SensorBoxGUI::SET_PORT_0_CONFIG_SUPPLY:
    {
        txData.append((uint8_t)WRITE_CONFIG_FUNC);
        txData.append((uint8_t)PORT_0);
        txData.append((uint8_t)CONFIG_RADIO_SUPPLY);

        measProcessState = REQ_SAMPLE_PORT_0_SUPPLY;
        break;
    }
    case SensorBoxGUI::REQ_SAMPLE_PORT_0_SUPPLY:
    {
        txData.append(REQ_SAMPLE);
        txData.append((uint8_t)0x00);
        txData.append((uint8_t)0x00);

        supplySample = true;

        measProcessState = DISABLE_PORT_0;
        break;
    }
    case SensorBoxGUI::SET_PORT_1_CONFIG_SUPPLY:
    {
        txData.append((uint8_t)WRITE_CONFIG_FUNC);
        txData.append((uint8_t)PORT_1);
        txData.append((uint8_t)CONFIG_RADIO_SUPPLY);

        measProcessState = REQ_SAMPLE_PORT_1_SUPPLY;
        break;
    }
    case SensorBoxGUI::REQ_SAMPLE_PORT_1_SUPPLY:
    {
        txData.append(REQ_SAMPLE);
        txData.append((uint8_t)0x00);
        txData.append((uint8_t)0x00);

        supplySample = true;

        measProcessState = DISABLE_PORT_1;
        break;
    }

    case SensorBoxGUI::DISABLE_PORT_1:
    {
        this->doSampleProcessing(PORT_1, lastSample, lastSupplySample);

        txData.append((uint8_t)WRITE_CONFIG_FUNC);
        txData.append((uint8_t)PORT_1);
        txData.append(0x0D);

        if (stoppingActive)
        {
            measProcessState = MEAS_IDLE;
            stoppingActive = false;
        }
        else
        {
            measProcessState = SET_PORT_0_CONFIG;
        }
        break;
    }
    case SensorBoxGUI::SET_PORT_0_CONFIG:
    {
        txData.append((uint8_t)WRITE_CONFIG_FUNC);
        txData.append((uint8_t)PORT_0);

        measType_t measType = (measType_t)port0Combo->currentIndex();

        if (measType == MEAS_NTC10K || measType == MEAS_NTC5K)
        {
            txData.append((uint8_t)MEAS_NTC5K);
        }
        else
        {
            txData.append((uint8_t)measType);
        }

        measProcessState = REQ_SAMPLE_PORT_0;
        break;
    }
    case SensorBoxGUI::REQ_SAMPLE_PORT_0:
    {
        txData.append(REQ_SAMPLE);
        txData.append((uint8_t)0x00);
        txData.append((uint8_t)0x00);

        if ((measType_t)port0Combo->currentIndex() == MEAS_RADIOMETRIC)
        {
            measProcessState = SET_PORT_0_CONFIG_SUPPLY;
        }
        else
        {
            measProcessState = DISABLE_PORT_0;
        }

        break;
    }
    case SensorBoxGUI::DISABLE_PORT_0:
    {
        this->doSampleProcessing(PORT_0, lastSample, lastSupplySample);

        txData.append((uint8_t)WRITE_CONFIG_FUNC);
        txData.append((uint8_t)PORT_0);
        txData.append(0x0D);

        if (stoppingActive)
        {
            measProcessState = DISABLE_PORT_1;
        }
        else
        {
            measProcessState = SET_PORT_1_CONFIG;
        }
        break;
    }
    case SensorBoxGUI::SET_PORT_1_CONFIG:
    {
        txData.append((uint8_t)WRITE_CONFIG_FUNC);
        txData.append((uint8_t)PORT_1);

        measType_t measType = (measType_t)port1Combo->currentIndex();

        if (measType == MEAS_NTC10K || measType == MEAS_NTC5K)
        {
            txData.append((uint8_t)MEAS_NTC5K);
        }
        else
        {
            txData.append((uint8_t)measType);
        }

        measProcessState = REQ_SAMPLE_PORT_1;
        break;
    }
    case SensorBoxGUI::REQ_SAMPLE_PORT_1:
    {
        txData.append((uint8_t)REQ_SAMPLE);
        txData.append((uint8_t)0x00);
        txData.append((uint8_t)0x00);

        if ((measType_t)port1Combo->currentIndex() == MEAS_RADIOMETRIC)
        {
            measProcessState = SET_PORT_1_CONFIG_SUPPLY;
        }
        else
        {
            measProcessState = DISABLE_PORT_1;
        }

        break;
    }

    }

    serialPort->write(txData);
}

void SensorBoxGUI::onSerialReceived()
{
    rcvdArr.append(serialPort->readAll());

    if (rcvdArr.size() == 3)
    {
        errorCode_t errCode   = (errorCode_t)static_cast<uint8_t>(rcvdArr[0]);
        funcCode_t funcCodeUp = (funcCode_t)static_cast<uint8_t>(rcvdArr[0]);

        if (configErrorChecker(errCode))
        {
            if (funcCodeUp == UPLINK_SAMPLE)
            {
                uint8_t sampleLow  = static_cast<uint16_t>(rcvdArr.at(1));
                uint8_t sampleHigh = static_cast<uint16_t>(rcvdArr.at(2));

                if (supplySample)
                {
                    supplySample = false;
                    lastSupplySample = sampleLow | (sampleHigh << 8);
                }
                else
                {
                    lastSample = sampleLow | (sampleHigh << 8);
                }
            }

            rcvdArr.clear();
            this->measProcessStateMachine();
        }
    }
}

bool SensorBoxGUI::configErrorChecker(errorCode_t errCode)
{
    bool retVal = false;

    switch (errCode) {

    default:
    case SensorBoxGUI::SYS_OK:
    {
        retVal = true;
        break;
    }
    case SensorBoxGUI::SYS_WRONG_CMD:
    {
        this->statusBar()->showMessage("Wrong configuration command!");
        QMessageBox::critical(this, "Configuration failed", "Wrong configuration command!");
        break;
    }
    case SensorBoxGUI::ADC_TIMEOUT_ERR:
    {
        this->statusBar()->showMessage("ADC timeout error!");
        QMessageBox::critical(this, "Configuration failed", "ADC timeout error!");
        break;
    }
    case SensorBoxGUI::ADC_CONF_ERR:
    {
        this->statusBar()->showMessage("ADC communicaion error!");
        QMessageBox::critical(this, "Configuration failed", "ADC communicaion error!");
        break;
    }

    }

    return retVal;
}

void SensorBoxGUI::doSampleProcessing(portNo_t portNo, uint16_t adcSample, uint16_t adcValueSupply)
{
    QStringList valAndUnit{};

    switch (portNo) {

    case SensorBoxGUI::PORT_0:
    {
        valAndUnit = this->adc2valueConverter((measType_t)port0Combo->currentIndex(), adcSample, adcValueSupply);

        qDebug() << "Port0 value: " << valAndUnit;
        port0LCD->display(valAndUnit.at(0));
        port0DisplLbl->setText(valAndUnit.at(1));
        break;
    }
    case SensorBoxGUI::PORT_1:
    {
        valAndUnit = this->adc2valueConverter((measType_t)port1Combo->currentIndex(), adcSample, adcValueSupply);

        qDebug() << "Port1 value: " << valAndUnit;
        port1LCD->display(valAndUnit.at(0));
        port1DisplLbl->setText(valAndUnit.at(1));
        break;
    }

    }
}

QStringList SensorBoxGUI::adc2valueConverter(const measType_t &measType, uint16_t adcValue, uint16_t adcValueSupply)
{
    QString lcdNumberString{};
    QString portLblString{};

    double voltage    = (double)adcValue * 125e-6;
    double voltSupply = (double)adcValueSupply * 125e-6;

    switch (measType){

    case SensorBoxGUI::MEAS_PT1000:
    {
        double resistance  = voltage/currentSourceValue;
        double temperature = ((-R0 * A_PT1000) + std::sqrt((std::pow(R0, 2) * std::pow(A_PT1000, 2)) - (4*R0*B_PT1000*(R0 - resistance))))/(2*R0*B_PT1000);
        double tempRounded = std::ceil(temperature * 10.0) / 10.0;
        tempRounded += 0.2;
        lcdNumberString = QString("%1").arg(tempRounded, 0, 'f', 1);
        portLblString = "Temperature [°C]";
        break;
    }
    case SensorBoxGUI::MEAS_NTC5K:
    {
        const double ntc_R1{5e3};
        const double B_NTC{3889.0};
        double resistance  = (voltage * equivalResistanceNTC)/((currentSourceValue * equivalResistanceNTC) - voltage);
        double A = B_NTC/log(ntc_R1/resistance);
        double temperature = (A*ntc5k_T1)/(A-ntc5k_T1) - 273.15;
        double tempRounded = std::ceil(temperature * 10.0) / 10.0;
        lcdNumberString = QString("%1").arg(tempRounded, 0, 'f', 1);
        portLblString = "Temperature [°C]";
        break;
    }
    case SensorBoxGUI::MEAS_NTC10K:
    {
        const double ntc_R1{10e3};
        const double B_NTC{3435.0};
        double resistance  = (voltage * equivalResistanceNTC)/((currentSourceValue * equivalResistanceNTC) - voltage);
        double A = B_NTC/log(ntc_R1/resistance);
        double temperature = (A*ntc5k_T1)/(A-ntc5k_T1) - 273.15;
        double tempRounded = std::ceil(temperature * 10.0) / 10.0;
        lcdNumberString = QString("%1").arg(tempRounded, 0, 'f', 1);
        portLblString = "Temperature [°C]";
        break;
    }
    case SensorBoxGUI::MEAS_VOLTAGE:
    {
        const double R1 = 9090;
        const double R2 = 1e3;

        double voltOut = voltage * (R1+R2)/R2;

        double voltOutRounded = std::ceil(voltOut * 10.0) / 10.0;

        lcdNumberString = QString("%1").arg(voltOutRounded, 0, 'f', 2);
        portLblString = "Voltage [V]";

        break;
    }
    case SensorBoxGUI::MEAS_CURRENT:
    {
        break;
    }
    case SensorBoxGUI::MEAS_RADIOMETRIC:
    {
        (void)0;
        break;
    }

    }

    return QStringList() << lcdNumberString << portLblString;
}

/**********************************************************************************************
Widgets inits
**********************************************************************************************/

QWidget *SensorBoxGUI::createPortSettingsWidget()
{
    QGroupBox *portSettWidget = new QGroupBox("Port settings", this);
    QGridLayout *portSettGrid = new QGridLayout(portSettWidget);

    port0Combo = new QComboBox(portSettWidget);
    port1Combo = new QComboBox(portSettWidget);

    QLabel *port0Lbl = new QLabel("Port 0: ", portSettWidget);
    QLabel *port1Lbl = new QLabel("Port 1: ", portSettWidget);

    port0EnChkBox = new QCheckBox(portSettWidget);
    port1EnChkBox = new QCheckBox(portSettWidget);

    port0EnChkBox->setChecked(true);
    port1EnChkBox->setChecked(true);

    connect(port0EnChkBox, &QCheckBox::stateChanged, port0Combo, &QSpinBox::setEnabled);
    connect(port1EnChkBox, &QCheckBox::stateChanged, port1Combo, &QSpinBox::setEnabled);

    port0Combo->addItems(portItemsList);
    port0Combo->setFixedWidth(200);
    port1Combo->addItems(portItemsList);
    port1Combo->setFixedWidth(200);

    portSettGrid->addWidget(port0Lbl, 0, 0);
    portSettGrid->addWidget(port0EnChkBox, 0, 1);
    portSettGrid->addWidget(port0Combo, 0, 2);

    portSettGrid->addWidget(port1Lbl, 0, 3);
    portSettGrid->addWidget(port1EnChkBox, 0, 4);
    portSettGrid->addWidget(port1Combo, 0, 5);

    return portSettWidget;
}

QWidget *SensorBoxGUI::createControlsWidget()
{
    QGroupBox *controlsWidget = new QGroupBox("Controls", this);
    QGridLayout *controlsGrid = new QGridLayout(controlsWidget);

    QToolButton *setCfgBtn = new QToolButton(controlsWidget);
    startBtn  = new QToolButton(controlsWidget);
    stopBtn  = new QToolButton(controlsWidget);

    timeEnChkBox = new QCheckBox("Use timer", controlsWidget);
    timeSpinBox = new QSpinBox(controlsWidget);

    timeEnChkBox->setEnabled(false);

    timeSpinBox->setRange(0, 65535);
    timeSpinBox->setEnabled(false);
    timeSpinBox->setSuffix(" sec");
    timeSpinBox->setFixedSize(70, 20);

    connect(timeEnChkBox, &QCheckBox::stateChanged, timeSpinBox, &QSpinBox::setEnabled);
    connect(setCfgBtn,    &QToolButton::clicked, this, &SensorBoxGUI::onSetConfigClicked);
    connect(startBtn,     &QToolButton::clicked, this, &SensorBoxGUI::onStartButtonClicked);
    connect(stopBtn,      &QToolButton::clicked, this, &SensorBoxGUI::onStopButtonClicked);

    setCfgBtn->setText("Set Config");
    setCfgBtn->setFixedSize(70, 20);
    setCfgBtn->setVisible(false);

    startBtn->setText("Start");
    startBtn->setFixedSize(70, 20);
    startBtn->setEnabled(true);

    stopBtn->setText("Stop");
    stopBtn->setFixedSize(70, 20);
    stopBtn->setEnabled(true);

    controlsGrid->addWidget(setCfgBtn, 0, 0);
    controlsGrid->addWidget(startBtn, 1, 0);
    controlsGrid->addWidget(timeSpinBox, 1, 1);
    controlsGrid->addWidget(timeEnChkBox, 1, 2);
    controlsGrid->addWidget(stopBtn, 2, 0);

    return controlsWidget;
}

QWidget *SensorBoxGUI::createDisplayResultsWidget()
{
    QGroupBox *displWidget = new QGroupBox("Results", this);
    displWidget->setMinimumSize(300, 150);
    QGridLayout *displGrid = new QGridLayout(displWidget);

    port0DisplLbl = new QLabel("Port 0 value: ", displWidget);
    port1DisplLbl = new QLabel("Port 1 value: ", displWidget);

    port0LCD = new QLCDNumber(10, displWidget);
    port1LCD = new QLCDNumber(10, displWidget);

    port0LCD->setMode(QLCDNumber::Dec);
    port0LCD->setSegmentStyle(QLCDNumber::Filled);
    port0LCD->display(QString::number(0));

    QPalette palette0 = port0LCD->palette();
    palette0.setColor(QPalette::Normal, QPalette::WindowText, Qt::black);
    palette0.setColor(QPalette::Normal, QPalette::Window, Qt::green);
    port0LCD->setAutoFillBackground(true);
    port0LCD->setPalette(palette0);


    QPalette palette1 = port1LCD->palette();
    palette1.setColor(QPalette::Normal, QPalette::WindowText, Qt::black);
    palette1.setColor(QPalette::Normal, QPalette::Window, Qt::green);
    port1LCD->setAutoFillBackground(true);
    port1LCD->setPalette(palette1);

    port1LCD->setMode(QLCDNumber::Dec);
    port1LCD->setSegmentStyle(QLCDNumber::Filled);
    port1LCD->display(QString::number(0));

    displGrid->addWidget(port0DisplLbl, 0, 0);
    displGrid->addWidget(port0LCD, 0, 1);
    displGrid->addWidget(port1DisplLbl, 1, 0);
    displGrid->addWidget(port1LCD, 1, 1);

    return displWidget;
}
