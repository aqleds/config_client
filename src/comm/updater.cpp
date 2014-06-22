#include "updater.h"
#include <QFile>

Updater::Updater(QextSerialPort* port, QObject *parent) :
    QThread(parent)
{
    m_port = port;
}


bool Updater::readFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return false;
    m_data = file.readAll();
    return true;
}

bool Updater::detect()
{
    char buffer[100];
    int retries=5;
    while (retries--)
    {
        buffer[0]=COMMAND_READ_ID;
        m_port->write(buffer,1);
        m_port->setTimeout(500);
        this->sleep(500);
        if (m_port->read(buffer,2)==2)
        {
            if ((buffer[0] == 0x03) && ((unsigned char)buffer[1] == 0xC0))
                return true;
        }
        // clear buffer
        m_port->read(buffer,100);
    }
    return false;
}

void Updater::run()
{
    char cmd,buffer[100];
    const char *data = m_data.constData();
    emit status_update("Starting");
    emit status_percent(0);
    int addr, res, p=0;

    if (detect())
    {
        while ( p < m_data.length())
        {
            m_port->setTimeout(1000);
            emit status_update("Flashing");
            cmd = data[p];
            if ((p+3)<m_data.length())
            {
                addr = (data[p+1]<<16) +
                        (data[p+2]<<8) +
                        (data[p+3]);
            }

            if (cmd == COMMAND_WRITE_PM)
            {
                m_port->write(data+p,4 + FLASH_BLOCK_SIZE);
                p += (4+FLASH_BLOCK_SIZE);
                res = m_port->read(buffer,1);
                if (res!=1 || buffer[0]!=COMMAND_ACK)
                {
                    emit status_update("Protocol error");
                }
            } else if (cmd == COMMAND_WRITE_CM)
            {
                m_port->write(data+p, 1 + 8*3);
                p += (1+8*3);
                res = m_port->read(buffer,1);
                if (res!=1 || buffer[0]!=COMMAND_ACK)
                {
                    emit status_update("Protocol error");
                }

            } else if (cmd == COMMAND_READ_ID)
            {
                p++;
            } else if (cmd == COMMAND_RESET)
            {
                m_port->write(&cmd,1);
                p++;
            } else
            {
                emit status_update("File error...");
            }
        }

    }
    else
    {
        emit status_update("Device not detected");
    }
    emit finished();
}
