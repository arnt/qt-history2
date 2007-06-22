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

#ifndef TEXT_BUTTON_H
#define TEXT_BUTTON_H

#include <QtGui>
#include "demoitem.h"
#include "demotextitem.h"
#include "scanitem.h"

class DemoItemAnimation;
class ButtonBackground;

class TextButton : public DemoItem
{
public:
    enum ALIGNMENT {LEFT, RIGHT};
    enum BUTTONTYPE {SIDEBAR, PANEL, UP, DOWN};
    enum STATE {ON, OFF, HIGHLIGHT, DISABLED};
    
    TextButton(const QString &text, ALIGNMENT align = LEFT, int userCode = 0,
        QGraphicsScene *scene = 0, QGraphicsItem *parent = 0, BUTTONTYPE color = SIDEBAR);
    virtual ~TextButton();
    
    // overidden methods:
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = 0){};
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void animationStarted(int id = 0);
    void prepare();
    void setState(STATE state);
    void setMenuString(const QString &menu);
    void setDisabled(bool disabled);
    
private:
    void setupButtonBg();
    void setupScanItem();
    void setupHoverText();
    
    DemoItemAnimation *scanAnim;
    ButtonBackground *bgOn;
    ButtonBackground *bgOff;
    ButtonBackground *bgHighlight;
    ButtonBackground *bgDisabled;
    
    BUTTONTYPE buttonType;
    ALIGNMENT alignment;
    QString buttonLabel;
    QString menuString;
    int userCode;
    QSize logicalSize;
    
    STATE state;
};

#endif // TEXT_BUTTON_H

