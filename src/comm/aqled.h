#ifndef AQLED_H
#define AQLED_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <qextserialenumerator.h>
#include <QStringList>

#include "protocol.h"
#include <stdint.h>

#define OP_READ 'r'
#define OP_READ_RT 'R'
#define OP_WRITE 'w'

#define CMD_PING                'p'
#define CMD_RTC                 't'
#define CMD_TIMETABLE           'T'
#define CMD_CHANNEL_CURRENT_AND_VOLTS     'i'
#define CMD_CHANNEL_LAMP        'l'
#define CMD_CHANNEL_RAMP        'r'
#define CMD_TURBO_MODE          '!'
#define CMD_INPUT_VOLTS         'v'
#define CMD_CHANNEL_OVERRIDE    's'
#define CMD_SAVE_CONFIG         'S'
#define CMD_RELOAD_CONFIG       'R'

class AqLed : public QObject
{
    Q_OBJECT

    QStringList m_known_ports;

    typedef enum {
        SUNRISE = 0,
        DAY = 1,
        SUNSET = 2,
        NIGHT = 3
    } AqLedTimeOfDay;

    typedef enum {
        CH_GLOBAL = 0,
        CH1 = 1,
        CH2 = 2,
        CH3 = 3
    } AqLedChannel;

    typedef enum {
        ST_DISCONNECTED,
        ST_CONNECTING,
        ST_CONNECTED
    } AqLedState;



private:

   AqLedState m_state;
   QString m_port_name;
   QTimer m_timer;
   Protocol protocol;

   void fillKnownPorts();
   QString discoverNewPorts();
   bool checkPortStillAvailable();


public:

    typedef struct {
       int sunrise_h, sunrise_m;
       int day_h, day_m;
       int sunset_h, sunset_m;
       int night_h,night_m;

    } st_timetable;

    typedef struct {
        int h, m, s;
        int day, month, year;
        int dow;
    } st_rtc;

    explicit AqLed(QObject *parent = 0);

   bool isConnected();

    bool ping();

    bool setRTC(st_rtc* rtc_data);
    bool getRTC(st_rtc* rtc_data);
    bool getRT(st_rtc* rtc_data, int* moment_of_day=NULL);

    bool setTimeTable(st_timetable* timetable);
    bool getTimeTable(st_timetable* timetable);

    bool getChannelCurrentAndVolts(int ch, float* ich, float* vch);
    float getInputVolts();

    bool setChannelLamp(int ch, float vmin, float vmax);
    bool getChannelLamp(int ch, float* vmin, float *vmax);

    bool setChannelRamp(int ch, float sunrise, float day=0, float sunset=0, float night=0);
    bool getChannelRamp(int ch, float *sunrise, float* day, float* sunset, float* night);

    bool setChannelOverride(int ch, int seconds, float brightness);

    bool setTurboMode(bool turbo);
    bool getTurboMode();

    bool saveConfig();
    bool reloadConfig();

    QString momentOfDayStr(int moment);
     bool syncRTCToSystemTime();

signals:

   void connected(QString port);
   void disconnected();

    
public slots:

   void timerTimeout();
    
};

#endif // AQLED_H
