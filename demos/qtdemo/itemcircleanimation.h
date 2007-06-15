/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ITEM_CIRCLE_ANIMATION_H
#define ITEM_CIRCLE_ANIMATION_H

#include <QtCore>
#include <QObject>
#include <QtGui>
#include <QTimeLine>
#include <QList>
#include "demoitem.h"
#include "letteritem.h"
#include "guideline.h"
#include "guidecircle.h"

typedef QList<LetterItem *> LetterList;
class TickerEffect;

class ItemCircleAnimation : public QObject, public DemoItem
{
public:
    ItemCircleAnimation(QGraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    virtual ~ItemCircleAnimation();

    // overidden methods:
    QRectF boundingRect() const;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = 0);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    void animationStarted(int id = 0);
    void animationStopped(int id = 0);
    void prepare();
    void tick();
    void switchToNextEffect();
    void useGuideQt();
    void useGuideTt();
    void pause(bool on);

    bool tickOnPaint;
    bool paused;
    bool doIntroTransitions;

private:
    void setupLetters();
    void createLetter(char c);
    void setupGuides();
    void useGuide(Guide *guide, int firstLetter, int lastLetter);
    void swapModel();
    void setTickerScale(float s);

    int showCount;
    float scale;
    QPointF mouseMoveLastPosition;
    int letterCount;
    LetterList *letterList;
    Guide *qtGuide1;
    Guide *qtGuide2;
    Guide *qtGuide3;
    Guide *currGuide;
    TickerEffect *effect;
    QTime tickTimer;
};

#endif // ITEM_CIRCLE_ANIMATION_H



