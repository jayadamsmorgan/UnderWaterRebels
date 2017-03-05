#include "worker.h"

QHostAddress kulebyaka("192.168.8.177");
quint16 port = 8000;

Worker::Worker(QObject *parent) : QObject(parent)
{
    Veichle = new DUdp(kulebyaka,port);

    event = new SDL_Event;
    SDL_Init(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    joy = SDL_JoystickOpen(0);

    kulpack = new int[6];
    speedMode = 0;
    AP = 0;
    AD = 0;
    AY = 0;
    qDebug() <<"we entered worker \n";
}


void Worker::onTimeOut()
{

    input.clear();
    output.clear();

    formPacket();
    Veichle->sendPacket(output);

    input = Veichle->readPacket();
    parsePacket();
    uinformation();

}

void Worker::parsePacket()
{


}

void Worker::uinformation()
{
    ping = 0;
    depth = 0;

    emit updateInfo(ping,depth,speedMode,Veichle->state,AY,AP,AD);

}
void Worker::formPacket()
{

    SDL_PollEvent(event);

    int Axis = 0;

    for(int i = 0;i < 5;i++)
    {
        Axis=  SDL_JoystickGetAxis(joy, i);
        output.append((qint8)((int)((((float)Axis)/32767)*100)));
    }

    char button = 0;

    button = button<<1|SDL_JoystickGetButton(joy,8);
    button = button<<1|SDL_JoystickGetButton(joy,9);

    button = button<<1|SDL_JoystickGetButton(joy,0);
    button = button<<1|SDL_JoystickGetButton(joy,1);

    if(SDL_JoystickGetHat(joy,0) == 1)
        button = button<<1|1;
    else
      button = button<<1|0;

    if(SDL_JoystickGetHat(joy,0) == 4)
        button = button<<1|1;
    else
      button = button<<1|0;

    output.append(button);

    button = 0;

    AY  = AY^SDL_JoystickGetButton(joy,4);
    button = button<<1|AY;
    AD = AD^SDL_JoystickGetButton(joy,5);
    button = button<<1|AD;
    AP =  AP^SDL_JoystickGetButton(joy,6);
    button = button<<1|AP;
    //button = button<<1|SDL_JoystickGetButton(joy,4);
    //button = button<<1|SDL_JoystickGetButton(joy,5);
    //button = button<<1|SDL_JoystickGetButton(joy,6);

    if(SDL_JoystickGetButton(joy,7))
    {
        if(speedMode == 2) speedMode = 0;
        else ++speedMode;

        switch(speedMode)
        {
            case 0:
                button = button<<1|1;
                button = button<<1|0;
                button = button<<1|0;
            break;
            case 1:
                button = button<<1|0;
                button = button<<1|1;
                button = button<<1|0;
            break;
            case 2:
                button = button<<1|0;
                button = button<<1|0;
                button = button<<1|1;
            break;
            default:
                button = button<<1|0;
                button = button<<1|0;
                button = button<<1|0;
        }
    }

    output.append(button);
    //for(int i = 0; i < output.size();++i)
    //{

}

