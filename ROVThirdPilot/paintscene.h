#ifndef PAINTSCENE_H
#define PAINTSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

class paintScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit paintScene(QObject *parent = 0);
    ~paintScene();

private:
    QPointF     previousPoint;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
};

#endif // PAINTSCENE_H
