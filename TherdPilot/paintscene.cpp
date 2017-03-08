#include "paintscene.h"

paintScene::paintScene(QObject *parent) : QGraphicsScene(parent){
}

paintScene::~paintScene(){
}

void paintScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    addEllipse(event->scenePos().x() - 10,event->scenePos().y() - 10, 10,10, QPen(Qt::NoPen), QBrush(Qt::green));
    previousPoint = event->scenePos();
}
