#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include <QByteArray>
#include "qextserialport.h"
#include <stdint.h>

#define ESC    0x1b
#define STX    0x02
#define ETX    0x03
#define NAK    0x15
#define RESETC 0x09

#define ENQ    0x05
#define ACK    0x06
#define UNK    0x07

#define PSTX   0x04
#define PETX   0x05
#define PRESET 0x06


#define PROTOCOL_MAX_BUFFER 64

class Protocol : public QObject
{
    Q_OBJECT
private:

    typedef enum {
                        ST_WAIT_STX,
                        ST_WAIT_DATA,
                        ST_WAIT_ESC_DATA,
                        ST_STOPPED


    } t_protocol_state;

    t_protocol_state m_state;

    QextSerialPort* m_port;

    unsigned char crc16hi;
    unsigned char crc16lo;

    QByteArray m_data;

    int getChar(char *out_char);

    void crc16_init( void );
    void crc16_update_4_bits( unsigned char val );
    void crc16_update( unsigned char val );
    uint16_t crc16_calc ( unsigned char *data, int len );
    void clearRxQueue();



public:
    explicit Protocol(QObject *parent = 0);
    bool connectToPort(QString port);
    void disconnectFromPort();
    int sendPacket(uint8_t* data, int len);
    int recvPacket(uint8_t* data, int max_len);
    int sendCall(uint8_t* data, int len, uint8_t *rply=NULL, int max_len=0);

signals:
    
public slots:
    
};

#endif // PROTOCOL_H
