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
    TextButton::BUTTONCOLOR color;
    bool highlighted;

    ButtonBackground(TextButton::BUTTONCOLOR color, bool highlighted, QGraphicsScene *scene, QGraphicsItem *parent) : DemoItem(scene, parent)
    {
        this->color = color;
        this->highlighted = highlighted;
        useSharedImage(QString(__FILE__) + static_cast<int>(color) + highlighted);
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
            if (this->highlighted)
                painter.setBrush(QColor(220, 210, 210));
            else
                painter.setBrush(QColor(190, 180, 180));
        }
        else {
            QLinearGradient brush(0, 0, 0, scaledRect.height());
            brush.setSpread(QLinearGradient::PadSpread);
            if (this->color == TextButton::BLUE){
                brush.setColorAt(0.0, QColor(255, 255, 255, 200));
                if (!this->highlighted)
                    brush.setColorAt(1.0, QColor(255, 255, 255, 20));
            }
            else {
                brush.setColorAt(0.0, QColor(226, 255, 137, 200));
                if (!this->highlighted)
                    brush.setColorAt(1.0, QColor(226, 255, 137, 20));
           }
            painter.setBrush(brush);
        }
        painter.drawRoundRect(0, 0, scaledRect.width(), scaledRect.height(), 10, 90);
        return image;
    }
};

TextButton::TextButton(const QString &text, ALIGNMENT align, int userCode, QGraphicsScene *scene, QGraphicsItem *parent, BUTTONCOLOR color)
    : DemoItem(scene, parent)
{
    this->menuName = text;
    this->alignment = align;
    this->buttonColor = color;
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
        delete this->hoverTextAnim;
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
    float xOffset = 16;
    float yOffset = 0;
    float down = 1;
    textItem->setPos(xOffset, yOffset);
    this->hoverTextAnim = new DemoItemAnimation(textItem);
    this->hoverTextAnim->setDuration(1000);
    this->hoverTextAnim->timeline->setLoopCount(1);
    this->hoverTextAnim->setPosAt(0.0, QPointF(xOffset, yOffset));
    this->hoverTextAnim->setPosAt(0.001, QPointF(xOffset, yOffset+down));
    this->hoverTextAnim->setPosAt(0.7, QPointF(xOffset, yOffset+down));
    this->hoverTextAnim->setPosAt(1.0, QPointF(xOffset + 8, yOffset+down));
}

void TextButton::setupScanItem()
{
    ScanItem *scanItem = new ScanItem(this->scene(), this);
    scanItem->setZValue(zValue() + 1);

    this->scanAnim = new DemoItemAnimation(scanItem);
    this->scanAnim->setDuration(1000);
    this->scanAnim->timeline->setLoopCount(1);

    float x = 1;
    float y = 1.5f;
    float stop = BUTTON_WIDTH - scanItem->boundingRect().width() - x;
    if (this->alignment == LEFT){
        this->scanAnim->setPosAt(0.0, QPointF(x, y));
        this->scanAnim->setPosAt(0.5, QPointF(stop, y));
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

void TextButton::setupButtonBg()
{
    this->bgOff = new ButtonBackground(this->buttonColor, false, this->scene(), this);
    this->bgOn = new ButtonBackground(this->buttonColor, true, this->scene(), this);
}

void TextButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if (this->locked)
        return;

    if (Colors::noAnimations){
        // wait a bit in the beginning
        // to enhance the effect. Have to this here
        // so that the adaption can be dynamic
        this->scanAnim->setDuration(1000);
        this->scanAnim->setPosAt(0.2, this->scanAnim->posAt(0));
    }

    if (MenuManager::instance()->window->fpsMedian > 10
        || Colors::noAdapt
        || Colors::noTimerUpdate){
        this->hoverTextAnim->play(true, true);
        this->scanAnim->play(true, true);
    }
    this->bgOn->setRecursiveVisible(true);
    this->bgOff->setRecursiveVisible(false);
}

void TextButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if (Colors::noAnimations)
        this->scanAnim->stop();
    this->hoverTextAnim->stop();
    this->bgOn->setRecursiveVisible(false);
    this->bgOff->setRecursiveVisible(true);
}

void TextButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    if (!this->locked){
        this->hoverTextAnim->stop();
        MenuManager::instance()->itemSelected(this->userCode, this->menuName);
    }
}

void TextButton::animationStarted(int)
{
    this->bgOn->setRecursiveVisible(false);
}



