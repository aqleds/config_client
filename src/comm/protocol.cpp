#include "protocol.h"
#include <stdint.h>
#include <qdebug.h>


Protocol::Protocol(QObject *parent) :
    QObject(parent)
{
    m_state = ST_WAIT_STX;
}

QextSerialPort* Protocol::getPort()
{
    return m_port;
}

/* we use a tiny CRC16 that can work in 8 bit micro controllers */
uint8_t crc16tab_hi[16] = {
        0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
        0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1
};
uint8_t crc16tb_lo[16] = {
        0x00, 0x21, 0x42, 0x63, 0x84, 0xA5, 0xC6, 0xE7,
        0x08, 0x29, 0x4A, 0x6B, 0x8C, 0xAD, 0xCE, 0xEF
};


void Protocol::crc16_init( void )
{
    crc16hi = 0xFF;
    crc16lo = 0xFF;
}


void Protocol::crc16_update_4_bits( unsigned char val )
{
    unsigned char	t;

    t = crc16hi >> 4;

    t = t ^ val;

    crc16hi = (crc16hi << 4) | (crc16lo >> 4);
    crc16lo = crc16lo << 4;

    crc16hi = crc16hi ^ crc16tab_hi[t];
    crc16lo = crc16lo ^ crc16tb_lo[t];
}


void Protocol::crc16_update( unsigned char val )
{
    crc16_update_4_bits( val >> 4 );
    crc16_update_4_bits( val & 0x0F );
}

uint16_t Protocol::crc16_calc ( unsigned char *data, int len )
{
    uint16_t crc16;
    crc16_init();

    while(len--)
        crc16_update(*data++);

    crc16 =  crc16hi;
    crc16 = (crc16 << 8) | crc16lo;
    return crc16;
}



int Protocol::sendPacket(uint8_t* data, int len)
{
    uint8_t data_crc[len+2];
    uint8_t buffer[len*2+4];
    uint8_t ch;
    uint8_t *p=buffer;
    uint8_t *src = data_crc;
    uint16_t crc = crc16_calc(data,len);

    if(!m_port) return -3;

    /* copy data to new buffer, and add crc16 just after */
    memcpy(data_crc,data,len);

    data_crc[len]  =crc>>8;
    data_crc[len+1]=crc&0xff;

    len+=2;

    /* our protocol frames starts by STX character */
    *p++=STX;

    /* escape the packet data to prevent control characters*/
    while(len--)
    {

        ch = *src++;
        switch(ch)
        {
            case STX:       *p++=ESC; *p++=PSTX;    break;
            case ETX:       *p++=ESC; *p++=PETX;    break;
            case RESETC:    *p++=ESC; *p++=PRESET;  break;
            case ESC:       *p++=ESC; *p++=ESC;     break;
            default:        *p++=ch;
        }

    }
    /* ETX marks the end of a packet */
    *p++=ETX;

    return m_port->write((const char*)buffer,p-buffer);

}
int Protocol::getChar(char *out_char)
{
    char in_char;

    // if there is data availabe in the serial port buffer
    // get it and add it to our internal buffer
    if (m_port->bytesAvailable())
    {
        QByteArray tmp_data = m_port->readAll();
        m_data.append(tmp_data);
    }

    // if we have no data in our internal buffer,
    // try to get at least one byte
    if (m_data.length()==0)
    {

        if(m_port->getChar(&in_char))
        {
            *out_char = in_char;
            return 1;
        }
    }

    if (m_data.length()>0)
    {
        *out_char = m_data.at(0);
        m_data.remove(0,1);
        return 1;
    }
    return -1;
}

int Protocol::recvPacket(uint8_t* data, int max_len)
{

    char ch;
    bool unscape = false;
    bool waiting_stx = true;

    int p=0;

    if(!m_port) /* if the port is closed just return with error */
    {
        return -3;
    }

    while (true)
    {
        if (p>=max_len) /* we exceeded max packet length */
        {
            return -1000;
        }

        /* get a character or exit */
        if (this->getChar(&ch)<0) return 0;

        /* start of packet, set buffer pointer to 0 */
        if (ch==STX) { p=0; waiting_stx = false; continue;}

        /* yet waiting for start of packet, continue */
        if (waiting_stx) continue;

        /* end of packet */
        if (ch==ETX) break;

        /* at this point what we have is data */
        if (unscape)
        {
            switch(ch)
            {
                case PSTX: data[p++]=STX; break;
                case PETX: data[p++]=ETX; break;
                case PRESET: data[p++]=RESETC; break;
                default: data[p++]=ch;
            }
            unscape = false;
            continue;
        }

        /* if it's an ESC then next character is unscaped, continue */
        if (ch==ESC) { unscape = true; continue; }

        data[p++]=ch;

    }

    /* smaller than crc?.. discard as bad packet */
    if (p<2) return -1;

    /* check crc at end of packet */

    uint16_t crc = crc16_calc(data,p-2);
    uint16_t crcpacket = (data[p-2]<<8) | data[p-1];

    if (crcpacket!=crc) return -2;

    return p-2; /* return length excluding crc16 */

}

void Protocol::clearRxQueue()
{
    m_data.clear();
    if (m_port->bytesAvailable())
    {
        m_port->readAll();
    }
}

int Protocol::sendCall(uint8_t *data, int len, uint8_t *rply, int max_len)
{
    uint8_t rply_buffer[PROTOCOL_MAX_BUFFER];
    int retries=10;

    while (retries)
    {
        qDebug() << "[sendCall " << (char)data[0] << (char)data[1] << data[2] <<"]";
        int send_result = sendPacket(data,len);
        if (send_result<0)
        {
            return -1;
        }

        int recv_result = recvPacket(rply_buffer,PROTOCOL_MAX_BUFFER);
        qDebug() << "[sendCall recvResult=" << recv_result <<" ]";
        if (recv_result>2)
        {
            if ((data[0]==rply_buffer[0]) && // CMD
                (data[1]==rply_buffer[1]) && // OP (read/write)
                    (rply_buffer[2]==ACK))
            {
                qDebug() << "[sendCall ACK OK]";
                int payload_size = recv_result-3;
                if (rply)
                {
                    if (payload_size>max_len)
                    {
                        return -4; // provided buffer size is exhausted
                    }
                    memcpy(rply,rply_buffer+3,payload_size);
                }
                return payload_size;
            }
            else
            {
                 //qDebug() << "[sendCall BAD ACK: "<< (char)rply_buffer[0] <<
                 //            (char)rply_buffer[1] << (char)rply_buffer[2] << " ]";
                // we received a valid packet, wrong call
                // so we clean the reception queue for just
                // in case there are more old packets
                clearRxQueue();
            }

        }
        retries--;
        //qDebug() << "RETRY..." <<retries;
    }
    return -5; // retry count exceeded
}
bool Protocol::connectToPort(QString port)
{
    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 100};
    m_port = new QextSerialPort(port, settings, QextSerialPort::Polling);

    return m_port->open(QIODevice::ReadWrite);
}

void Protocol::disconnectFromPort()
{
    if (!m_port) return;
    m_port->close();
    m_port=NULL;
}

