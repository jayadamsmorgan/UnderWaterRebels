#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <SDL.h>
#include <QFile>
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
    int depth;
    bool AP,AD,AY;
    int ping;
    SDL_Joystick *joy;
    SDL_Event *event;

    void formPacket();
    void parsePacket();
    void uinformation();
signals:
    void updateInfo(int cam2id,int depth,int speed,int yaw,int pitch,int roll,int yawsetpoint,int pitchsetpoint,bool connection,bool ayaw,bool apitch,bool adepth,bool leak);

public slots:
    void onTimeOut();
    void onUpdateJoy();
};

#endif // WORKER_H
