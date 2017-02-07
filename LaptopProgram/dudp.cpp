#include "dudp.h"

DUdp::DUdp(QHostAddress adr, quint16 pr)
{
    socket = new QUdpSocket(this);
    address.setAddress(adr.toString());
    port = pr;

    state = socket->bind(address,port);
    connect(socket,SIGNAL(readyRead()), this, SLOT(readPacket()));
}

void DUdp::sendPacket(QByteArray out)
{
    socket->writeDatagram(out,address,port);
    //state = socket->bind(address,port);
}

QByteArray DUdp::readPacket()
{

    QByteArray input;
    input.resize(socket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(input.data(),input.size(),
                             &sender, &senderPort);
    if(input.size() > 0)
    {
        state = true;
    }
    else
    {
        state = false;
    }
return input;

}
