#ifndef AQLEDS_H
#define AQLEDS_H

#include <QMainWindow>
#include "comm/aqled.h"
#include <QLineEdit>

namespace Ui {
class AqLeds;
}

class AqLeds : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit AqLeds(QWidget *parent = 0);
    ~AqLeds();
    
private:
    Ui::AqLeds *ui;

    // vmin/vmax helpers
    QLineEdit* vminLines(int index);
    QLineEdit* vmaxLines(int index);
    float getMomentBrightness(int moment);

    // H/M helpers (sunrise, day, sunset,  night..)
    QLineEdit* momentLines(int moment);
    bool setMoment_H_M(int moment, int hour, int minute);
    bool getMoment_H_M(int moment, int* hour, int* minute);

    // the device and timers...
    AqLed *m_device;
    QTimer rtc_check_timer;

    // voltage read/write
    bool checkVoltagesAreOk();
    void readChannelLamps();
    void writeChannelLamps();

    // ramp timetables read/write
    bool checkRampTimesAreOk();
    bool writeRampTimes();
    bool readRampTimes();

    // channel ramps read/write
    int getSelectedRampChannel();
    bool readChannelRamp();
    void writeChannelRamp();
    void readDeviceSettings();
    void writeTurboMode();
    void readTurboMode();

public slots:

    void connected(QString port_name);
    void disconnected();

    void rtc_check_timeout();

    void on_rbChannel1_clicked();
    void on_rbChannel2_clicked();
    void on_rbChannel3_clicked();

    void on_slSunrise_sliderMoved(int value);
    void on_slDay_sliderMoved(int value);
    void on_slSunset_sliderMoved(int value);
    void on_slNight_sliderMoved(int value);
    void on_slGlobal_sliderMoved(int value);

    void on_cbIUnderstand_clicked(bool checked);

    void on_btVoltagesRestore_clicked();
    void on_btVoltagesSend_clicked();

    void on_btRampTimesRestore_clicked();
    void on_btRampTimesSend_clicked();

    void on_sunriseLineEdit_textChanged(QString text);
    void on_dayLineEdit_textChanged(QString text);
    void on_sunsetLineEdit_textChanged(QString text);
    void on_nightLineEdit_textChanged(QString text);

    void on_btWriteConfig_clicked();
    void on_btReloadConfig_clicked();
    void on_btExportConfig_clicked();
    void on_btImportConfig_clicked();

    void on_cbTurboMode_toggled();

    void on_btSyncSystemTime_clicked();
    void on_menuActionUpdate_triggered();
};

#endif // AQLEDS_H
