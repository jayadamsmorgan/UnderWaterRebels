#ifndef DUDP_H
#define DUDP_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class DUdp : public QUdpSocket
{
    Q_OBJECT

public:
    DUdp(QHostAddress adr,quint16 pr);

    void sendPacket(QByteArray out);
    QUdpSocket *socket;
    QHostAddress address;
    quint16  port;
    bool state;

public slots:
    QByteArray readPacket();

};

#endif // DUDP_H
