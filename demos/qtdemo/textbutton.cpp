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
    QSize logicalSize;

    ButtonBackground(TextButton::BUTTONTYPE type, bool highlighted, bool pressed, QSize logicalSize,
        QGraphicsScene *scene, QGraphicsItem *parent) : DemoItem(scene, parent)
    {
        this->type = type;
        this->highlighted = highlighted;
        this->pressed = pressed;
        this->logicalSize = logicalSize;
        useSharedImage(QString(__FILE__) + static_cast<int>(type) + highlighted + pressed);
    }

protected:
    QImage *createImage(const QMatrix &matrix) const
    {
        if (type == TextButton::SIDEBAR || type == TextButton::PANEL)
            return createRoundButtonBackground(matrix);
        else
            return createArrowBackground(matrix);
    }
    
    QImage *createRoundButtonBackground(const QMatrix &matrix) const
    {
        QRect scaledRect;
        scaledRect = matrix.mapRect(QRect(0, 0, this->logicalSize.width(), this->logicalSize.height()));
        
        QImage *image = new QImage(scaledRect.width(), scaledRect.height(), QImage::Format_ARGB32_Premultiplied);
        image->fill(QColor(0, 0, 0, 0).rgba());
        QPainter painter(image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        
        if (Colors::useEightBitPalette){
            painter.setPen(QColor(120, 120, 120));
            if (this->pressed)
                painter.setBrush(QColor(60, 60, 60));
            else if (this->highlighted)
                painter.setBrush(QColor(100, 100, 100));
            else
                painter.setBrush(QColor(80, 80, 80));
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

        if (this->type == TextButton::PANEL)
            painter.drawRect(0, 0, scaledRect.width(), scaledRect.height());
        else
            painter.drawRoundRect(0, 0, scaledRect.width(), scaledRect.height(), 10, 90);
        return image;
    }
    
    QImage *createArrowBackground(const QMatrix &matrix) const
    {
        QRect scaledRect;
        scaledRect = matrix.mapRect(QRect(0, 0, this->logicalSize.width(), this->logicalSize.height()));
        
        QImage *image = new QImage(scaledRect.width(), scaledRect.height(), QImage::Format_ARGB32_Premultiplied);
        image->fill(QColor(0, 0, 0, 0).rgba());
        QPainter painter(image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        
        if (Colors::useEightBitPalette){
            painter.setPen(QColor(120, 120, 120));
            if (this->pressed)
                painter.setBrush(QColor(60, 60, 60));
            else if (this->highlighted)
                painter.setBrush(QColor(100, 100, 100));
            else
                painter.setBrush(QColor(80, 80, 80));
        }
        else {
            QLinearGradient outlinebrush(0, 0, 0, scaledRect.height());
            QLinearGradient brush(0, 0, 0, scaledRect.height());
            
            brush.setSpread(QLinearGradient::PadSpread);
            QColor highlight(255, 255, 255, 70);
            QColor shadow(0, 0, 0, 70);
            QColor sunken(220, 220, 220, 30);
            QColor normal1 = QColor(200, 170, 160, 50);
            QColor normal2 = QColor(50, 10, 0, 50);
 
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

        painter.drawRect(0, 0, scaledRect.width(), scaledRect.height());
        float xOff = scaledRect.width() / 2;
        float yOff = scaledRect.height() / 2;
        float sizex = 3.0f;
        float sizey = 1.5f;
        if (this->type == TextButton::BACK)
            sizey *= -1;
        QPainterPath path;
        path.moveTo(xOff, yOff + (5 * sizey));
        path.lineTo(xOff - (4 * sizex), yOff - (3 * sizey));
        path.lineTo(xOff + (4 * sizex), yOff - (3 * sizey));
        path.lineTo(xOff, yOff + (5 * sizey));
        painter.drawPath(path);
        
        return image;
    }    

};

TextButton::TextButton(const QString &text, ALIGNMENT align, int userCode,
    QGraphicsScene *scene, QGraphicsItem *parent, BUTTONTYPE type)
    : DemoItem(scene, parent)
{
    this->menuString = text;
    this->buttonLabel = text;
    this->alignment = align;
    this->buttonType = type;
    this->userCode = userCode;
    this->bgOn = 0;
    this->bgOff = 0;
    this->bgHighlight = 0;
    
    this->checkable = true;
    this->state = OFF;

    this->setAcceptsHoverEvents(true);
    this->setCursor(Qt::PointingHandCursor);

    const int w = 180;
    const int h = 19;
    if (type == SIDEBAR || type == PANEL)
        this->logicalSize = QSize(w, h);
    else
        this->logicalSize = QSize((w / 2) - 5, h * 1.5f);
}

void TextButton::setMenuString(const QString &menu)
{
    this->menuString = menu;
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
    return QRectF(0, 0, this->logicalSize.width(), this->logicalSize.height());
};

void TextButton::setupHoverText()
{
    if (this->buttonLabel.isEmpty())
        return;
        
    DemoTextItem *textItem = new DemoTextItem(this->buttonLabel, Colors::buttonFont(), Colors::buttonText, -1, this->scene(), this);
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

void TextButton::setState(STATE state)
{
    this->state = state;
    this->bgOn->setRecursiveVisible(state == ON);
    this->bgOff->setRecursiveVisible(state == OFF);
    this->bgHighlight->setRecursiveVisible(state == HIGHLIGHT);    
}

void TextButton::setupButtonBg()
{
    this->bgOn = new ButtonBackground(this->buttonType, true, true, this->logicalSize, this->scene(), this);
    this->bgOff = new ButtonBackground(this->buttonType, false, false, this->logicalSize, this->scene(), this);
    this->bgHighlight = new ButtonBackground(this->buttonType, true, false, this->logicalSize, this->scene(), this);
    this->setState(OFF);
}

void TextButton::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if (this->locked)
        return;

    if (this->state == OFF){
        this->setState(HIGHLIGHT);
        
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
    }
}

void TextButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    this->setState(OFF);

    if (Colors::noAnimations && Colors::useButtonBalls)
        this->scanAnim->stop();
}

void TextButton::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    if (this->state == HIGHLIGHT || this->state == OFF);
        this->setState(ON);
}

void TextButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (this->state == ON){
        this->setState(OFF);
        if (!this->locked && this->boundingRect().contains(event->pos())){
            MenuManager::instance()->itemSelected(this->userCode, this->menuString);
        }
    }
}

void TextButton::animationStarted(int)
{
    this->setState(OFF);
}



