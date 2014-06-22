#include "aqled.h"
#include "qextserialport.h"
#include <qDebug>
#include <QSettings>
#include <QDateTime>
#include <comm/q6_binary.h>

AqLed::AqLed(QObject *parent) :
    QObject(parent)
{
    fillKnownPorts();

    m_timer.setInterval(1000);
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(timerTimeout()));
    m_timer.start();
}


Updater* AqLed::getUpdater()
{
    Updater *updater = NULL;
    if (m_state == ST_CONNECTED)
    {
        updater = new Updater(protocol.getPort());
    }
    return updater;
}

void AqLed::fillKnownPorts()
{
    m_known_ports.clear();

    foreach ( QextPortInfo info, QextSerialEnumerator::getPorts() )
    {
            this->m_known_ports.append( info.portName );
    }

    m_state = ST_DISCONNECTED;
}

QString AqLed::discoverNewPorts()
{

    foreach ( QextPortInfo info, QextSerialEnumerator::getPorts() )
    {
            if ( !m_known_ports.contains(info.portName) )
            {
                fillKnownPorts();
                return info.portName;
            }
            //qDebug() << info.portName;
    }

    fillKnownPorts();
    return "";
}

bool AqLed::checkPortStillAvailable()
{
    foreach ( QextPortInfo info, QextSerialEnumerator::getPorts() )
    {
            if ( m_port_name == info.portName )
                return true;
    }

    return false;
}


void AqLed::timerTimeout()
{
    QString port;
    QSettings settings;

    switch(m_state)
    {
        case ST_DISCONNECTED:
         {
            port = discoverNewPorts();
            if (port=="")
            {
                port = settings.value("port").toString();
            }
            if ((port!="")&&(protocol.connectToPort(port)))
            {
                m_state=ST_CONNECTING;
                m_port_name = port;
            }
            break;
         }

        case ST_CONNECTING:
        {
            if (ping())
            {
                m_state=ST_CONNECTED;
                settings.setValue("port",m_port_name);
                emit connected(m_port_name);

            }
            else
            {
                m_state=ST_DISCONNECTED;
                protocol.disconnectFromPort();
                emit disconnected();
                qDebug() << " NO PING, DISCONNECTED! ";
            }
            break;
        }

        case ST_CONNECTED:
            if (!checkPortStillAvailable())
            {
                m_state = ST_DISCONNECTED;
                protocol.disconnectFromPort();
                emit disconnected();
            }
            break;
    }
}


bool AqLed::isConnected()
{
    return m_state==ST_CONNECTED;
}

bool AqLed::ping()
{
    int res;
    uint8_t ping_rply[256];
    uint8_t ping_pkt[]={CMD_PING};

    protocol.sendPacket(ping_pkt,1);
    res = protocol.recvPacket(ping_rply,256);

    return (*ping_rply=='p')&&(res==1);

}

bool AqLed::setRTC(st_rtc* rtc_data)
{
    uint8_t setRTC_pkt[]={CMD_RTC,
                          OP_WRITE,
                          rtc_data->h,
                          rtc_data->m,
                          rtc_data->s,
                          rtc_data->day,
                          rtc_data->month,
                          rtc_data->year-2000,
                          rtc_data->dow
                         };
    int res = protocol.sendCall(setRTC_pkt,9);

    return (res==0); // we expect an ACK reply with no data

}
bool AqLed::syncRTCToSystemTime()
{
    AqLed::st_rtc rtc;
    QDateTime dateTime = QDateTime::currentDateTime();

    rtc.day = dateTime.date().day();
    rtc.month = dateTime.date().month();
    rtc.year = dateTime.date().year();
    rtc.dow = dateTime.date().dayOfWeek();
    rtc.h = dateTime.time().hour();
    rtc.m = dateTime.time().minute();
    rtc.s = dateTime.time().second();

    return setRTC(&rtc);
}

bool AqLed::getRTC(st_rtc* rtc_data)
{
    int res;
    uint8_t getRTC_rply[256];
    uint8_t getRTC_pkt[]={CMD_RTC,OP_READ};
    uint8_t *p = getRTC_rply;

    res = protocol.sendCall(getRTC_pkt,2,getRTC_rply,256);
    if (res>=7)
    {
        rtc_data->h     =   *p++;
        rtc_data->m     =   *p++;
        rtc_data->s     =   *p++;
        rtc_data->day   =   *p++;
        rtc_data->month =   *p++;
        rtc_data->year  =   *p++ + 2000;
        rtc_data->dow   =   *p++;

        return true;
    }
    return false;
}


QString AqLed::momentOfDayStr(int moment)
{
    QString moment_names[]={tr("SUNRISE"),tr("DAY"),
                             tr("SUNSET"),tr("NIGHT")};
    return moment_names[moment];
}

bool AqLed::getRT(st_rtc* rtc_data, int* moment_of_day)
{
    int res;
    uint8_t getRTC_rply[256];
    uint8_t getRTC_pkt[]={CMD_RTC,OP_READ_RT};
    uint8_t *p = getRTC_rply;

    res = protocol.sendCall(getRTC_pkt,2,getRTC_rply,256);
    if (res>=7)
    {
        rtc_data->h     =   *p++;
        rtc_data->m     =   *p++;
        rtc_data->s     =   *p++;
        rtc_data->day   =   *p++;
        rtc_data->month =   *p++;
        rtc_data->year  =   *p++ + 2000;
        rtc_data->dow   =   *p++;

        if (moment_of_day) *moment_of_day = *p++;

        return true;
    }
    return false;
}
bool AqLed::setTimeTable(st_timetable* timetable)
{
    uint8_t setTimeTable_pkt[]={CMD_TIMETABLE,OP_WRITE,
                                timetable->sunrise_h,
                                timetable->sunrise_m,
                                timetable->day_h,
                                timetable->day_m,
                                timetable->sunset_h,
                                timetable->sunset_m,
                                timetable->night_h,
                                timetable->night_m
                         };

    int res = protocol.sendCall(setTimeTable_pkt,sizeof(setTimeTable_pkt));

    return (res==0); // we expect an ACK reply with no data
}

bool AqLed::getTimeTable(st_timetable* timetable)
{
    uint8_t getTimeTable_pkt[]={CMD_TIMETABLE,OP_READ};

    int res;
    uint8_t getTimeTable_rply[256];
    uint8_t *p = getTimeTable_rply;

    res = protocol.sendCall(getTimeTable_pkt,2,getTimeTable_rply,256);

    if (res==8)
    {
        timetable->sunrise_h  = *p++;
        timetable->sunrise_m  = *p++;
        timetable->day_h      = *p++;
        timetable->day_m      = *p++;
        timetable->sunset_h   = *p++;
        timetable->sunset_m   = *p++;
        timetable->night_h    = *p++;
        timetable->night_m    = *p++;
        return true;
    }
    return false;
}


bool AqLed::getChannelCurrentAndVolts(int ch, float* ich, float* vch)
{
    int res;
    uint8_t get_reply[256];
    uint8_t get_pkt[]={CMD_CHANNEL_CURRENT_AND_VOLTS,OP_READ,ch};

    res = protocol.sendCall(get_pkt,sizeof(get_pkt),get_reply,256);

    if (res==4)
    {
        *ich = Q6_extract(get_reply);
        *vch = Q6_extract(get_reply+2);
        return true;
    }
    return false;
}

float AqLed::getInputVolts()
{
    int res;
    uint8_t get_rply[256];
    uint8_t get_pkt[]={CMD_INPUT_VOLTS,OP_READ};
    float result;

    res = protocol.sendCall(get_pkt,sizeof(get_pkt),get_rply,256);

    if (res==2)
    {
        result = Q6_extract(get_rply);
        return result;
    }
    return -100.0;
}

bool AqLed::setChannelLamp(int ch, float vmin, float vmax)
{
    int res;
    uint8_t setChannelLamp_pkt[]={CMD_CHANNEL_LAMP,
                                  OP_WRITE,
                                  ch,
                                  0, 0,
                                  1, 1};

    Q6_store(setChannelLamp_pkt+3,vmin);
    Q6_store(setChannelLamp_pkt+5,vmax);

    res = protocol.sendCall(setChannelLamp_pkt,
                            sizeof(setChannelLamp_pkt));

    return (res==0); // we expect an ACK reply with no data
}

bool AqLed::getChannelLamp(int ch, float* vmin, float *vmax)
{
    int res;
    uint8_t reply[16];
    uint8_t getChannelLamp_pkt[]={CMD_CHANNEL_LAMP,
                                  OP_READ,
                                  ch};

    res = protocol.sendCall(getChannelLamp_pkt,
                            sizeof(getChannelLamp_pkt), reply, 16);

    //qDebug() << "getChannelLamp: res=" << res;
    if (res==4)
    {
        *vmin = Q6_extract(reply+0);
        *vmax = Q6_extract(reply+2);
        return true;
    }

    return false;

}

bool AqLed::setChannelRamp(int channel, float sunrise, float day,
                                        float sunset, float night)
{
    int res;
    uint8_t setChannelRamp_pkt[]={CMD_CHANNEL_RAMP,
                                  OP_WRITE,
                                  channel,
                                  0, 0,
                                  1, 1,
                                  2, 2,
                                  3, 3};

    Q6_store(setChannelRamp_pkt+3,sunrise);
    Q6_store(setChannelRamp_pkt+5,day);
    Q6_store(setChannelRamp_pkt+7,sunset);
    Q6_store(setChannelRamp_pkt+9,night);

    res = protocol.sendCall(setChannelRamp_pkt,
                            sizeof(setChannelRamp_pkt));

    return (res==0); // we expect an ACK reply with no data

}

bool AqLed::getChannelRamp(int ch, float* sunrise, float* day,
                                   float* sunset, float* night)
{
    int res;
    uint8_t getChannelLamp_pkt[]={CMD_CHANNEL_RAMP,
                                  OP_READ,
                                  ch};

    uint8_t reply[128];

    res = protocol.sendCall(getChannelLamp_pkt,
                            sizeof(getChannelLamp_pkt),
                            reply,
                            128);

    if (res==2*4+1)
    {
        *sunrise = Q6_extract(reply+1);
        *day     = Q6_extract(reply+3);
        *sunset  = Q6_extract(reply+5);
        *night   = Q6_extract(reply+7);

        return true;
    }

    return false;
}

bool AqLed::setChannelOverride(int channel, int seconds, float brightness)
{
    int res;
    uint8_t setChannelOverride_pkt[]={CMD_CHANNEL_OVERRIDE,
                                     OP_WRITE,
                                     channel,
                                     0, 0,
                                     0, 0};

    Q6_store(setChannelOverride_pkt+3, brightness);
    int16_store(setChannelOverride_pkt+5,seconds);

    res = protocol.sendCall(setChannelOverride_pkt,
                            sizeof(setChannelOverride_pkt));

    return (res==0);
}

bool AqLed::setTurboMode(bool turbo)
{
    int res;
    uint8_t setTurboMode_pkt[]={CMD_TURBO_MODE,
                                OP_WRITE,
                                turbo?0x01:0x00 };

    res = protocol.sendCall(setTurboMode_pkt,sizeof(setTurboMode_pkt));

    return (res==0);
}

bool AqLed::getTurboMode()
{
    int res;
    uint8_t reply[16];
    uint8_t getTurboMode_pkt[]={CMD_TURBO_MODE,
                                OP_READ};

    res = protocol.sendCall(getTurboMode_pkt,
                            sizeof(getTurboMode_pkt),
                            reply,
                            16);

    if (res==1 && reply[0]!=0x00) return true;
    else return (res==0);
}


bool AqLed::saveConfig()
{
    int res;
    uint8_t saveConfig_pkt[]={CMD_SAVE_CONFIG,
                                OP_WRITE };

    res = protocol.sendCall(saveConfig_pkt,
                            sizeof(saveConfig_pkt));

    return (res==0);
}

bool AqLed::reloadConfig()
{
    int res;
    uint8_t reloadConfig_pkt[]={CMD_RELOAD_CONFIG,
                                OP_WRITE };

    res = protocol.sendCall(reloadConfig_pkt,
                            sizeof(reloadConfig_pkt));

    return (res==0);
}


