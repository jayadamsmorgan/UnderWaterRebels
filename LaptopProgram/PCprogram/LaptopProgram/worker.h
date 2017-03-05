#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <SDL.h>
#include "dudp.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);


    DUdp *Veichle;
    QByteArray input;
    QByteArray output;
    int *kulpack;
    char speedMode;
    bool AP,AD,AY;

    SDL_Joystick *joy;
    SDL_Event *event;

    void formPacket();
    void parsePacket();

signals:

public slots:
    void onTimeOut();
};

#endif // WORKER_H
