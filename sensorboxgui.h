#ifndef SENSORBOXGUI_H
#define SENSORBOXGUI_H

#include <QMainWindow>
#include "serialport.h"
#include <QCheckBox>
#include <QComboBox>
#include <QToolButton>
#include <QSpinBox>
#include <QLCDNumber>
#include <QTimer>
#include <functional>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class SensorBoxGUI; }
QT_END_NAMESPACE

class SensorBoxGUI : public QMainWindow
{
    Q_OBJECT

public:
    SensorBoxGUI(QWidget *parent = nullptr);
    void initSerial(void);
    ~SensorBoxGUI();

private slots:
    void onSetConfigClicked(bool en);
    void onStartButtonClicked(bool en);
    void onStopButtonClicked(bool en);
    void onSerialReceived(void);
    void serialConnectSuccess(bool);

    void measProcessStateMachine(void);

private:
    Ui::SensorBoxGUI *ui;

    typedef enum
    {
        WRITE_CONFIG_FUNC = 0x01,
        WRITE_TIME_FUNC   = 0x02,
        START_MEAS_FUNC   = 0x03,
        STOP_MEAS_FUNC    = 0x04,
        REQ_SAMPLE        = 0x05,
        UPLINK_SAMPLE     = 0x10,
        UPLINK_STOP       = 0x11

    } funcCode_t;

    enum errorCode_t
    {
        SYS_OK          = 0xCC,
        SYS_WRONG_CMD   = 0xCD,
        ADC_TIMEOUT_ERR = 0xAD,
        ADC_CONF_ERR    = 0xAE,

    };

    enum measProcessState_t
    {
        DISABLE_PORT_1,
        SET_PORT_0_CONFIG,
        REQ_SAMPLE_PORT_0,
        SET_PORT_0_CONFIG_SUPPLY,
        REQ_SAMPLE_PORT_0_SUPPLY,
        DISABLE_PORT_0,
        SET_PORT_1_CONFIG,
        REQ_SAMPLE_PORT_1,
        SET_PORT_1_CONFIG_SUPPLY,
        REQ_SAMPLE_PORT_1_SUPPLY,
        MEAS_IDLE
    };

    enum measProcessRadioState_t
    {
        RADIO_DISABLE_PORT_1,
        RADIO_SET_PORT_0_CONFIG_SUPPLY,
        RADIO_REQ_SAMPLE_PORT_0_SUPPLY,
        RADIO_SET_PORT_0_CONFIG_OUTPUT,
        RADIO_REQ_SAMPLE_PORT_0_OUTPUT,
        RADIO_DISABLE_PORT_0,
        RADIO_SET_PORT_1_CONFIG_SUPPLY,
        RADIO_REQ_SAMPLE_PORT_1_SUPPLY,
        RADIO_SET_PORT_1_CONFIG_OUTPUT,
        RADIO_REQ_SAMPLE_PORT_1_OUTPUT,
    };

    enum portNo_t
    {
      PORT_0  = 0,
      PORT_1  = 1

    };


    QWidget *createPortSettingsWidget(void);
    QWidget *createControlsWidget(void);
    QWidget *createDisplayResultsWidget(void);

    bool configErrorChecker(errorCode_t errCode);
    void doSampleProcessing(portNo_t portNo, uint16_t adcSample, uint16_t adcValueSupply);

    QStringList portItemsList{"PT1000/PTC1000", "NTC 5K", "NTC 10K", "Voltage", "Current", "Radiometric"};

    QCheckBox *port0EnChkBox{};
    QCheckBox *port1EnChkBox{};

    QComboBox *port0Combo{};
    QComboBox *port1Combo{};
    QCheckBox *timeEnChkBox{};
    QSpinBox *timeSpinBox{};

    QToolButton *startBtn{};
    QToolButton *stopBtn{};

    QLCDNumber *port0LCD{};
    QLCDNumber *port0LCD_supply{};
    QLCDNumber *port1LCD{};
    QLCDNumber *port1LCD_supply{};

    QByteArray prepTx{};
    QByteArray rcvdArr{};

    measProcessState_t measProcessState{};
    measProcessRadioState_t measProcessRadioState{};

    SerialPort *serialPort{};
    QString comPort{"COM9"};
    QSerialPort::BaudRate br{QSerialPort::Baud9600};

    bool port1ToBeSent = false;
    bool stoppingActive = false;

    uint16_t lastSample       = 0;
    uint16_t lastSupplySample = 0;

    const double currentSourceValue{409.6e-6};
    const double A_PT1000{3.9083e-3};
    const double B_PT1000{-5.7750e-7};
    const double R0{1000.0};

    const double equivalResistanceNTC{10090.0};
    const double ntc5k_T1{298.15};

    enum measType_t
    {
        MEAS_PT1000,
        MEAS_NTC5K,
        MEAS_NTC10K,
        MEAS_VOLTAGE,
        MEAS_CURRENT,
        MEAS_RADIOMETRIC
    };

    enum config_t
    {
        CONFIG_PT1000       = 0x00,
        CONFIG_NTC_5K       = 0x01,
        CONFIG_NTC_10K      = 0x02,
        CONFIG_VOLTAGE      = 0x03,
        CONFIG_CURRENT      = 0x04,
        CONFIG_RADIO_SUPPLY = 0x05,
        CONFIG_RADIO_OUTPUT = 0x06,
        CONFIG_DISABLE      = 0x0D
    };

    QStringList adc2valueConverter(const measType_t &measType, uint16_t adcValue, uint16_t adcValueSupply);

    QLabel *port0DisplLbl{};
    QLabel *port1DisplLbl{};

    bool supplySample = false;
};

#endif // SENSORBOXGUI_H
