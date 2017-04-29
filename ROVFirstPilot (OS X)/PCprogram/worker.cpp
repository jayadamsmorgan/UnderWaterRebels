#include "worker.h"

QHostAddress kulebyaka("192.168.1.177");
quint16 port = 8000;
double koefs[9];
int axs[3];
int cam2id;
bool leak;
int yawsetpoint,pitchsetpoint;

Worker::Worker(QObject *parent) : QObject(parent)
{
    Veichle = new DUdp(kulebyaka,port);

    event = new SDL_Event;
    SDL_Init(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    joy = SDL_JoystickOpen(0);

    yawsetpoint = 0;
    pitchsetpoint = 0;
    kulpack = new int[6];
    speedMode = 0;
    cam2id = 0;
    leak = false;
    AP = false;
    AD = false;
    AY = false;

    qDebug() <<"we entered worker \n";


    QFile kfile("ROVsettings.txt");
    if(kfile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&kfile);

        for(int i = 0; i < 9;++i)
        {
          stream >> koefs[i] ;

        }

    }

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
    for(int i = 0;i < 3;++i)
    {
        int tmp = (int)(uchar)input[i*2];
        tmp = tmp<<8|((int)((uchar)(input[i*2+1])));
        axs[i] = (int)tmp/100;
    }
    depth = (int)((char)input[6]);
    depth = depth<<8|((int)((uchar)(input[7])));
    yawsetpoint = (int)((uchar)input[8]);
    yawsetpoint = yawsetpoint<<8|((int)((uchar)(input[9])));
    yawsetpoint  = yawsetpoint/100;
    pitchsetpoint = (int)((char)input[10]);
    pitchsetpoint = pitchsetpoint<<8|((int)((uchar)(input[11])));
    pitchsetpoint = (int)pitchsetpoint*1;
    leak = input[12]&(uchar)1;
}

void Worker::uinformation()
{


    emit updateInfo(cam2id,depth,speedMode,axs[0],axs[1],axs[2],yawsetpoint,pitchsetpoint,Veichle->state,AY,AD,AP,leak);

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
    //camera rotation
    if(SDL_JoystickGetHat(joy,0) == 1)
        button = button<<1|1;
    else
      button = button<<1|0;

    if(SDL_JoystickGetHat(joy,0) == 4)
        button = button<<1|1;
    else
      button = button<<1|0;

    //main manipulator
    button = button<<1|SDL_JoystickGetButton(joy,0);
    button = button<<1|SDL_JoystickGetButton(joy,1);
    //bottom manipulator
    button = button<<1|SDL_JoystickGetButton(joy,8);
    button = button<<1|SDL_JoystickGetButton(joy,9);

    //multiplexer
    if(SDL_JoystickGetButton(joy,2))
    {
        cam2id++;
        if(cam2id >= 2)
            cam2id = 0;
    }

    if(cam2id == 0)
    {
        button = button<<1|0;
        button = button<<1|1;
    }
    else if(cam2id == 1)
    {
        button = button<<1|1;
        button = button<<1|0;
    }

    output.append(button);

    button = 0;

    //button = button<<1|SDL_JoystickGetButton(joy,4);
    //button = button<<1|SDL_JoystickGetButton(joy,5);
    //button = button<<1|SDL_JoystickGetButton(joy,6);

    if(SDL_JoystickGetButton(joy,7))
    {
        if(speedMode == 2) speedMode = 0;
        else ++speedMode;
    }
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


        AY  = AY^SDL_JoystickGetButton(joy,4);
        button = button<<1|AY;
        AD = AD^SDL_JoystickGetButton(joy,5);
        button = button<<1|AD;
        AP =  AP^SDL_JoystickGetButton(joy,6);
        button = button<<1|AP;

        button = button<<1|SDL_JoystickGetButton(joy,10);


        output.append(button);

    for(int i = 0; i < 9;++i)
    {
        int tmp = (int)(koefs[i]*100);
        //qDebug()<< tmp;
        output.append(((char)(tmp>>8))|0);
        output.append(((char)(tmp))|0);
        //qDebug()<<(int)output[output.size()-1] <<  " " << (int)output[output.size()-2];
    }
   // qDebug() << output.size();

}


void Worker::onUpdateJoy()
{
    SDL_JoystickClose(joy);

    SDL_Quit();
    qDebug() << "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    event = new SDL_Event;
    SDL_Init(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    joy = SDL_JoystickOpen(0);
}
