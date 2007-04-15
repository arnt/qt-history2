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

#ifndef DEMO_ITEM_H
#define DEMO_ITEM_H

#include <QtGui>

class DemoItemAnimation;
class Guide;

class SharedImage
{
public:    
    SharedImage() : refCount(0), image(0), pixmap(0){};
    int refCount;
    QImage *image;
    QPixmap *pixmap;
    QMatrix matrix;
    QRectF unscaledBoundingRect;
};

class DemoItem : public QGraphicsItem
{
    
public:
    DemoItem(QGraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    virtual ~DemoItem();
    
    bool inTransition();    
    virtual void animationStarted(int id = 0){ Q_UNUSED(id); };
    virtual void animationStopped(int id = 0){ Q_UNUSED(id); };
    virtual void prepare(){};
    void setRecursiveVisible(bool visible);
    void useSharedImage(const QString &hashKey);
    void setNeverVisible(bool never = true);
    static void setMatrix(const QMatrix &matrix);
    virtual QRectF boundingRect() const; // overridden
    void setPosUsingSheepDog(const QPointF &dest, const QRectF &sceneFence);
    
    qreal opacity;
    bool locked;
    DemoItemAnimation *currentAnimation;
    bool noSubPixeling;
    
    // Used if controlled by a guide:
    void useGuide(Guide *guide, float startFrame = 0);
    void guideAdvance(float distance);
    void guideMove(float moveSpeed);
    void setGuidedPos(const QPointF &position);
    QPointF getGuidedPos();
    float startFrame;
    float guideFrame;
    Guide *currGuide;
    
protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = 0, QWidget *widget = 0); // overridden
    virtual QImage *createImage(const QMatrix &) const { return 0; }; 
    virtual bool collidesWithItem(const QGraphicsItem *, Qt::ItemSelectionMode) const { return false; };
    bool prepared;
    
private:
    SharedImage *sharedImage;
    QString hashKey;
    bool neverVisible;    
    bool validateImage();
    
    // Used if controlled by a guide:    
    void switchGuide(Guide *guide);
    friend class Guide;
    QPointF guidedPos;
    
    // The next static hash is shared amongst all demo items, and
    // has the purpose of reusing images to save memory and time 
    static QHash<QString, SharedImage *> sharedImageHash;
    static QMatrix matrix;
};

#endif // DEMO_ITEM_H

