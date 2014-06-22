#include "aqleds.h"
#include "ui_aqleds.h"
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>

AqLeds::AqLeds(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AqLeds)
{
    ui->setupUi(this);
    m_device = new AqLed(this);
    connect(m_device,SIGNAL(connected(QString)),this,SLOT(connected(QString)));
    connect(m_device,SIGNAL(disconnected()),this,SLOT(disconnected()));

    rtc_check_timer.setInterval(500);
    connect(&rtc_check_timer,SIGNAL(timeout()),this,SLOT(rtc_check_timeout()));

}

AqLeds::~AqLeds()
{
    delete ui;
    delete m_device;
}


void AqLeds::connected(QString port_name)
{

    qDebug() <<"connected to "<<port_name;
    rtc_check_timer.start();

    readDeviceSettings();

}

void AqLeds::on_btSyncSystemTime_clicked()
{
    if (!m_device->syncRTCToSystemTime())
    {
        QMessageBox::warning(NULL, tr("error"),
                             tr("Unable to sync system time to device"));
    }
}

void AqLeds::readDeviceSettings()
{
    readChannelLamps();
    readChannelRamp();
    readRampTimes();
    readTurboMode();
}

void AqLeds::readTurboMode()
{
    if (m_device->isConnected())
    {
        bool turbo = m_device->getTurboMode();
        ui->cbTurboMode->setChecked(turbo);
    }
}
void AqLeds::writeTurboMode()
{
    if (m_device->isConnected())
    {
        bool turbo = ui->cbTurboMode->isChecked();
        m_device->setTurboMode(turbo);
    }
}

void AqLeds::on_cbTurboMode_toggled()
{
    writeTurboMode();
}

void AqLeds::disconnected()
{
    rtc_check_timer.stop();
}


void AqLeds::on_cbIUnderstand_clicked(bool checked)
{
    ui->btVoltagesSend->setEnabled(checked);
    ui->btVoltagesRestore->setEnabled(checked);
    ui->ch1vmaxLineEdit->setEnabled(checked);
    ui->ch1vminLineEdit->setEnabled(checked);
    ui->ch2vmaxLineEdit->setEnabled(checked);
    ui->ch2vminLineEdit->setEnabled(checked);
    ui->ch3vmaxLineEdit->setEnabled(checked);
    ui->ch3vminLineEdit->setEnabled(checked);
}

void AqLeds::rtc_check_timeout()
{
    int moment_of_day;
    if (!m_device->isConnected()) return;

    AqLed::st_rtc rtc_data;
    if(m_device->getRT(&rtc_data, &moment_of_day))
    {
        QString rtc_str;
        rtc_str.sprintf("%02d:%02d:%02d %d/%d/%d",
                        rtc_data.h,rtc_data.m,rtc_data.s,
                        rtc_data.day, rtc_data.month,
                        rtc_data.year);
        ui->lbRTCTime->setText(rtc_str);
        ui->lbPlannerMoment->setText(m_device->momentOfDayStr(moment_of_day));
    }

    QLabel* lbVolts[]={ui->lbc1Volts, ui->lbc2Volts, ui->lbc3Volts};
    QLabel* lbCurrent[]={ui->lbc1Current, ui->lbc2Current, ui->lbc3Current};
    QLabel* lbWatts[]={ui->lbc1Watts, ui->lbc2Watts, ui->lbc3Watts};

    for (int channel=0;channel<3;channel++)
    {
        float ich, vch;
        m_device->getChannelCurrentAndVolts(channel+1,&ich,&vch);
        QString strVolts,strCurrent,strWatts;
        strVolts.sprintf("%2.2f V",vch);
        strCurrent.sprintf("%2.2f A",ich);
        strWatts.sprintf("%3.1f W",ich*vch);
        lbVolts[channel]->setText(strVolts);
        lbCurrent[channel]->setText(strCurrent);
        lbWatts[channel]->setText(strWatts);
    }
}



int AqLeds::getSelectedRampChannel()
{
    if (ui->rbChannel1->isChecked()) return 1;
    if (ui->rbChannel2->isChecked()) return 2;
    if (ui->rbChannel3->isChecked()) return 3;
    return 0;
}

float AqLeds::getMomentBrightness(int moment)
{
    QSlider *slider[]={ui->slSunrise, ui->slDay, ui->slSunset, ui->slNight};
    return (float) slider[moment]->value() /
            (float) slider[moment]->maximum();
}

void AqLeds::writeChannelRamp()
{
    if (!m_device->isConnected()) return;
    float val[4];
    float value;
    int moment;

    int ch = getSelectedRampChannel();

    for (moment=0; moment<4; moment++)
    {
        val[moment] = getMomentBrightness(moment);
    }

    if (!m_device->setChannelRamp(ch, val[0], val[1], val[2], val[3]))
    {
        QMessageBox::warning(NULL, tr("Unable to set channel ramp data"),
                             tr("Unable to set channel ramp data "
                                "for channel %1").arg(ch));
        return;
    }


    //set the global ramp
    value = (float) ui->slGlobal->value() /
            (float) ui->slGlobal->maximum();

    if (!m_device->setChannelRamp(0, value))
    {
        QMessageBox::warning(NULL,
                             tr("Unable to set global brightness"),
                             tr("Unable to set global brightness"));
       return;
    }
}

bool AqLeds::readChannelRamp()
{
    if (!m_device->isConnected()) return false;

    float val[4];
    float value;
    int moment;
    QSlider *slider[]={ui->slSunrise, ui->slDay, ui->slSunset, ui->slNight};
    int ch = getSelectedRampChannel();
    bool res = m_device->getChannelRamp(ch, &val[0], &val[1], &val[2], &val[3]);

    if (!res)
    {
        QMessageBox::warning(NULL,
                             tr("error"),
                             tr("Error reading channel %1 ramp").arg(ch));
        return false;
    }
    for (moment=0; moment<4; moment++)
    {
        value = val[moment] * (float) slider[moment]->maximum();
        slider[moment]->setValue((int)value);
    }

    if (!m_device->getChannelRamp(0, &val[0], &val[1], &val[2], &val[3]))
    {
        QMessageBox::warning(NULL,
                             tr("Error"),
                             tr("Unable to read global brightness"));
    }

    value = val[0] * (float) ui->slGlobal->maximum();
    ui->slGlobal->setValue(value);

    return true;
}


QLineEdit* AqLeds::vmaxLines(int index)
{
    QLineEdit* lines[]={ui->ch1vmaxLineEdit, ui->ch2vmaxLineEdit,
                            ui->ch3vmaxLineEdit};
    return lines[index];
}

QLineEdit* AqLeds::vminLines(int index)
{

    QLineEdit* lines[]={ui->ch1vminLineEdit, ui->ch2vminLineEdit,
                            ui->ch3vminLineEdit};
    return lines[index];
}


void AqLeds::readChannelLamps()
{
    if (!m_device->isConnected()) return;

    for (int i=0;i<3;i++)
    {
        float vmax,vmin;
        QString vmaxStr, vminStr;
        if (!m_device->getChannelLamp(i+1,&vmin,&vmax))
        {
            QMessageBox::warning(NULL, tr("Unable to read channel lamp data"),
                                 tr("Unable to read channel lamp data "
                                    "for channel %1 (vmin/vmax)").arg(i+1));
            return;
        }

        vminStr.sprintf("%02.2f", vmin);
        vmaxStr.sprintf("%02.2f", vmax);

        vminLines(i)->setText(vminStr);
        vmaxLines(i)->setText(vmaxStr);
    }
}

void AqLeds::writeChannelLamps()
{
    float vmax[3];
    float vmin[3];
    bool ok1,ok2;

    for (int i=0;i<3;i++)
    {
        vmin[i] = vminLines(i)->text().toFloat(&ok2);
        vmax[i] = vmaxLines(i)->text().toFloat(&ok1);
        if ((!ok1)||(!ok2))
        {
            QMessageBox::warning(NULL, tr("Unable to parse float"),
                                 tr("On channel %1").arg(i+1));
            return;
        }

    }

    for (int i=0;i<3;i++)
    {
         qDebug() << vmin[i] << " " << vmax[i];
        if(!m_device->setChannelLamp(i+1,vmin[i],vmax[i]))
        {
            QMessageBox::warning(NULL, tr("Unable to write channel lamp data"),
                                        tr("Unable to write channel lamp data "
                                           "for channel %1 (vmin/vmax)").arg(i+1));

            return;

        }
    }
}

bool AqLeds::checkVoltagesAreOk()
{
    int i;
    for (i=0;i<3;i++)
    {
        if (vmaxLines(i)->text().toFloat() <
                vminLines(i)->text().toFloat() )
        {
            QMessageBox::warning(NULL,
                                 tr("Voltage error"),
                                 tr("Channel %1 vmax is smaller than vmin").arg(i+1)
                                 );
            return false;
        }

    }
    return true;
}



void AqLeds::on_rbChannel1_clicked()
{
    readChannelRamp();
}

void AqLeds::on_rbChannel2_clicked()
{
    readChannelRamp();
}

void AqLeds::on_rbChannel3_clicked()
{
    readChannelRamp();
}

void AqLeds::on_slSunrise_sliderMoved(int value)
{
    (void)value;
    writeChannelRamp();
    m_device->setChannelOverride(this->getSelectedRampChannel(),
                                 5,
                                 this->getMomentBrightness(0));
}

void AqLeds::on_slDay_sliderMoved(int value)
{
    (void)value;
    writeChannelRamp();
    m_device->setChannelOverride(this->getSelectedRampChannel(),
                                 5,
                                 this->getMomentBrightness(1));
}

void AqLeds::on_slSunset_sliderMoved(int value)
{
    (void)value;
    qDebug() << ui->slSunset->value() << value;
    writeChannelRamp();
    m_device->setChannelOverride(this->getSelectedRampChannel(),
                                 5,
                                 this->getMomentBrightness(2));
}

void AqLeds::on_slNight_sliderMoved(int value)
{
    (void)value;
    writeChannelRamp();
    m_device->setChannelOverride(this->getSelectedRampChannel(),
                                 5,
                                 this->getMomentBrightness(3));
}

void AqLeds::on_slGlobal_sliderMoved(int value)
{
    (void)value;
    writeChannelRamp();
}


void AqLeds::on_btVoltagesRestore_clicked()
{
    readChannelLamps();
}

void AqLeds::on_btVoltagesSend_clicked()
{
    if (checkVoltagesAreOk())
    {
        writeChannelLamps();
    }
}



void AqLeds::on_btWriteConfig_clicked()
{
    m_device->saveConfig();
}

void AqLeds::on_btReloadConfig_clicked()
{
    m_device->reloadConfig();
    readDeviceSettings();
}

void AqLeds::on_btExportConfig_clicked()
{

}

void AqLeds::on_btImportConfig_clicked()
{

}



void AqLeds::on_sunriseLineEdit_textChanged(QString text)
{
    ui->btRampTimesSend->setEnabled(true);
}

void AqLeds::on_dayLineEdit_textChanged(QString text)
{
    ui->btRampTimesSend->setEnabled(true);
}

void AqLeds::on_sunsetLineEdit_textChanged(QString text)
{
     ui->btRampTimesSend->setEnabled(true);
}

void AqLeds::on_nightLineEdit_textChanged(QString text)
{
     ui->btRampTimesSend->setEnabled(true);
}


// Ramp times ////////////////////////////////////////////
QLineEdit* AqLeds::momentLines(int moment)
{
    QLineEdit* lines[]={ui->sunriseLineEdit,
                        ui->dayLineEdit,
                        ui->sunsetLineEdit,
                        ui->nightLineEdit};
    return lines[moment];
}

bool AqLeds::setMoment_H_M(int moment, int hour, int minute)
{
    QString strH_M;

    strH_M.sprintf("%02d:%02d",hour,minute);
    momentLines(moment)->setText(strH_M);

}


bool AqLeds::getMoment_H_M(int moment, int* hour, int* minute)
{
    QString moment_names[]={tr("SUNRISE"),tr("DAY"),
                             tr("SUNSET"),tr("NIGHT")};
    QString strH_M = momentLines(moment)->text();

    QStringList h_m = strH_M.split(QChar(':'));

    if (h_m.length()!=2)
    {
       QMessageBox::warning(NULL,
                             tr("Error parsing time"),
                             tr("Error parsing time for %1").arg(moment_names[moment]));
        return false;
    }

    *hour = h_m.at(0).toInt();
    *minute = h_m.at(1).toInt();

    if (*hour<0 || *hour>23 || *minute<0 || *minute>59) return false;

    return true;
}



void AqLeds::on_btRampTimesRestore_clicked()
{
    if (readRampTimes())
    {
        ui->btRampTimesSend->setEnabled(false);
    }
}

void AqLeds::on_btRampTimesSend_clicked()
{

    if (checkRampTimesAreOk())
    {
        if (writeRampTimes())
        {
            ui->btRampTimesSend->setEnabled(false);
        }
    }
}


bool AqLeds::checkRampTimesAreOk()
{
    int h_m_bcd[4];

    for (int m=0;m<4;m++)
    {
        int hour, minute;
        if (!getMoment_H_M(m, &hour, &minute))
            return false;
        h_m_bcd[m] = hour*100 + minute;
    }

    // TODO: think about some algorithm to
    //       validate the input hours, that they
    //       run all after each other.
    return true;
}

bool AqLeds::writeRampTimes()
{
    AqLed::st_timetable timetable;

    getMoment_H_M(0, &timetable.sunrise_h, &timetable.sunrise_m);
    getMoment_H_M(1, &timetable.day_h,     &timetable.day_m);
    getMoment_H_M(2, &timetable.sunset_h,  &timetable.sunset_m);
    getMoment_H_M(3, &timetable.night_h,   &timetable.night_m);

    if (m_device->isConnected())
    {
        if (!m_device->setTimeTable(&timetable))
        {
            QMessageBox::warning(NULL,
                                 tr("Unable to set timetable"),
                                 tr("Timetable set operation failed"));
        }
        else
        {
            return true;
        }
    }

    return false;
}

bool AqLeds::readRampTimes()
{
    AqLed::st_timetable timetable;

    if (!m_device->isConnected()) return false;

    if (m_device->getTimeTable(&timetable))
    {

        setMoment_H_M(0, timetable.sunrise_h, timetable.sunrise_m);
        setMoment_H_M(1, timetable.day_h,     timetable.day_m);
        setMoment_H_M(2, timetable.sunset_h,  timetable.sunset_m);
        setMoment_H_M(3, timetable.night_h,   timetable.night_m);
        return true;
    }
    else
    {
        QMessageBox::warning(NULL,
                             tr("Unable to set timetable"),
                             tr("Timetable set operation failed"));
        return false;

    }
}

void AqLeds::on_menuActionUpdate_triggered()
{
    qDebug() << "UPDATE!";

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open AqLEDs firmware update"),
                                                     "",
                                                     tr("Files (*.aqf)"));

    if (fileName.endsWith(".aqf"))
    {
        Updater *upd = m_device->getUpdater();
        if (!upd)
        {
            QMessageBox::warning(NULL,
                                 tr("The AqLeds is disconnected"),
                                 tr("The AqLeds is disconnected, try again when it's connected")
                          );
            return;
        }

        if (!upd->readFile(fileName))
        {
            QMessageBox::warning(NULL,
                                 tr("read error"),
                                 tr("the file couldn't be read"));
            delete upd;
            return;
        }

        // PASS TO A PROGRESS DIALOG
        upd->start();
    }
}





