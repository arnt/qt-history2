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

#include "textbutton.h"
#include "demoitemanimation.h"
#include "demotextitem.h"
#include "colors.h"
#include "menumanager.h"

#define BUTTON_WIDTH 180
#define BUTTON_HEIGHT 19

class ButtonBackground : public DemoItem
{
public:
    TextButton::BUTTONTYPE type;
    bool highlighted;
    bool pressed;

    ButtonBackground(TextButton::BUTTONTYPE type, bool highlighted, bool pressed, QGraphicsScene *scene, QGraphicsItem *parent) : DemoItem(scene, parent)
    {
        this->type = type;
        this->highlighted = highlighted;
        this->pressed = pressed;
        useSharedImage(QString(__FILE__) + static_cast<int>(type) + highlighted + pressed);
    }

protected:
    QImage *createImage(const QMatrix &matrix) const
    {
        QRect scaledRect = matrix.mapRect(QRect(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT));
        QImage *image = new QImage(scaledRect.width(), scaledRect.height(), QImage::Format_ARGB32_Premultiplied);
        image->fill(QColor(0, 0, 0, 0).rgba());
        QPainter painter(image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        
        if (Colors::useEightBitPalette){
            if (this->pressed)
                painter.setBrush(QColor(100, 100, 100));
            else if (this->highlighted)
                painter.setBrush(QColor(120, 120, 120));
            else
                painter.setBrush(QColor(110, 110, 110));
        }
        else {
            QLinearGradient outlinebrush(0, 0, 0, scaledRect.height());
            QLinearGradient brush(0, 0, 0, scaledRect.height());
            
            brush.setSpread(QLinearGradient::PadSpread);
            QColor highlight(255, 255, 255, 70);
            QColor shadow(0, 0, 0, 70);
            QColor sunken(220, 220, 220, 30);
            QColor normal1(255, 255, 245, 60);
            QColor normal2(255, 255, 235, 10);

            if (this->type == TextButton::PANEL){
                normal1 = QColor(200, 170, 160, 50);
                normal2 = QColor(50, 10, 0, 50);
            }

           if (pressed) {
               outlinebrush.setColorAt(0.0f, shadow);
               outlinebrush.setColorAt(1.0f, highlight);
               brush.setColorAt(0.0f, sunken);
               painter.setPen(Qt::NoPen);                    
           } else {
               outlinebrush.setColorAt(1.0f, shadow);
               outlinebrush.setColorAt(0.0f, highlight);
               brush.setColorAt(0.0f, normal1);                    
               if (!this->highlighted)
                   brush.setColorAt(1.0f, normal2);
               painter.setPen(QPen(outlinebrush, 1));
           }
           painter.setBrush(brush);
        }
        painter.drawRoundRect(0, 0, scaledRect.width(), scaledRect.height(), 10, 90);
        return image;
    }
};

TextButton::TextButton(const QString &text, ALIGNMENT align, int userCode, QGraphicsScene *scene, QGraphicsItem *parent, BUTTONTYPE type)
    : DemoItem(scene, parent)
{
    this->menuName = text;
    this->alignment = align;
    this->buttonType = type;
    this->userCode = userCode;

    this->setAcceptsHoverEvents(true);
    this->setCursor(Qt::PointingHandCursor);
}

void TextButton::prepare()
{
    if (!this->prepared){
        this->prepared = true;
        this->setupHoverText();
        this->setupScanItem();
        this->setupButtonBg();
    }
}

TextButton::~TextButton()
{
    if (this->prepared){
        if (Colors::useButtonBalls)
            delete this->scanAnim;
    }
}

QRectF TextButton::boundingRect() const
{
    return QRectF(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT);
};

void TextButton::setupHoverText()
{
    DemoTextItem *textItem = new DemoTextItem(this->menuName, Colors::buttonFont(), Colors::buttonText, -1, this->scene(), this);
    textItem->setZValue(zValue() + 2);
    textItem->setPos(16, 0);
}

void TextButton::setupScanItem()
{
    if (Colors::useButtonBalls){
        ScanItem *scanItem = new ScanItem(0, this);
        scanItem->setZValue(zValue() + 1);
        
        this->scanAnim = new DemoItemAnimation(scanItem);
        this->scanAnim->timeline->setLoopCount(1);
        
        float x = 1;
        float y = 1.5f;
        float stop = BUTTON_WIDTH - scanItem->boundingRect().width() - x;
        if (this->alignment == LEFT){
            this->scanAnim->setDuration(2500);
            this->scanAnim->setPosAt(0.0, QPointF(x, y));
            this->scanAnim->setPosAt(0.5, QPointF(x, y));
            this->scanAnim->setPosAt(0.7, QPointF(stop, y));
            this->scanAnim->setPosAt(1.0, QPointF(x, y));
            scanItem->setPos(QPointF(x, y));
        }
        else {
            this->scanAnim->setPosAt(0.0, QPointF(stop, y));
            this->scanAnim->setPosAt(0.5, QPointF(x, y));
            this->scanAnim->setPosAt(1.0, QPointF(stop, y));
            scanItem->setPos(QPointF(stop, y));
        }
    }
}

void TextButton::setupButtonBg()
{
    this->bgOff = new ButtonBackground(this->buttonType, false, false, this->scene(), this);
    this->bgOn = new ButtonBackground(this->buttonType, true, false, this->scene(), this);
    this->bgPressed = new ButtonBackground(this->buttonType, true, true, this->scene(), this);
}

void TextButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if (this->locked)
        return;

    if (Colors::noAnimations && Colors::useButtonBalls){
        // wait a bit in the beginning
        // to enhance the effect. Have to this here
        // so that the adaption can be dynamic
        this->scanAnim->setDuration(1000);
        this->scanAnim->setPosAt(0.2, this->scanAnim->posAt(0));
    }

    if (MenuManager::instance()->window->fpsMedian > 10
        || Colors::noAdapt
        || Colors::noTimerUpdate){
        if (Colors::useButtonBalls)
            this->scanAnim->play(true, true);
    }
   this->bgOn->setRecursiveVisible(true);
   this->bgOff->setRecursiveVisible(false);
   this->bgPressed->setRecursiveVisible(false);
}

void TextButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if (Colors::noAnimations && Colors::useButtonBalls)
        this->scanAnim->stop();
    this->bgOn->setRecursiveVisible(false);
    this->bgOff->setRecursiveVisible(true);
    this->bgPressed->setRecursiveVisible(false);
}

void TextButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    this->bgPressed->setRecursiveVisible(true);
    this->bgOn->setRecursiveVisible(false);
    this->bgOff->setRecursiveVisible(false);
}

void TextButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    this->bgPressed->setRecursiveVisible(false);
    this->bgOn->setRecursiveVisible(false);
    this->bgOff->setRecursiveVisible(true);

    if (!this->locked && this->boundingRect().contains(event->pos())){
        MenuManager::instance()->itemSelected(this->userCode, this->menuName);
    }
}

void TextButton::animationStarted(int)
{
    this->bgOn->setRecursiveVisible(false);
    this->bgPressed->setRecursiveVisible(false);    
}



