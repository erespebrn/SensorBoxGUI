#include "sensorboxgui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SensorBoxGUI w;
    w.show();
    w.initSerial();
    return a.exec();
}
