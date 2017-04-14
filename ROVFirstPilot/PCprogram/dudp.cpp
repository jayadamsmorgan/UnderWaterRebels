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
    qDebug() << (int)out[0]<< (int)out[1]<< (int)out[2]<< (int)out[3]<< (int)out[4]<< (int)out[5]<< (int)out[6]<< (int)out[7];
    //qDebug() << AY <<" " <<AD<<" "<<AP;
    qDebug() << "\n";
    state = socket->bind(address,port);
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
