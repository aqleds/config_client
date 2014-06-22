#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QByteArray>
#include <qextserialport.h>
#include <QThread>

#define COMMAND_NACK        0x00
#define COMMAND_ACK         0x01
#define COMMAND_WRITE_PM    0x03
#define COMMAND_WRITE_CM    0x07
#define COMMAND_RESET       0x08
#define COMMAND_READ_ID     0x09

#define FLASH_BLOCK_SIZE    0x600
class Updater : public QThread
{
    Q_OBJECT
private:
    QByteArray m_data;
    QextSerialPort *m_port;
    bool detect();
public:
    explicit Updater(QextSerialPort *port, QObject *parent = 0);
    bool readFile(QString filename);
    void run();
    
signals:
    void finished();
    void status_percent(int percent);
    void status_update(QString message);
    void error(QString errorMessage);

public slots:
    
};

#endif // UPDATER_H
