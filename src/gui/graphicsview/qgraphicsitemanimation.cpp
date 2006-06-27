/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*!
    \class QGraphicsItemAnimation
    \brief The QGraphicsItemAnimation class provides simple animation
    support for QGraphicsItem.
    \since 4.2
    \ingroup multimedia
*/

#include "qgraphicsitemanimation.h"

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicsitem.h"

#include <QtCore/qtimeline.h>
#include <QtCore/qpoint.h>
#include <QtGui/qmatrix.h>

class QGraphicsItemAnimationPrivate
{
public:
    inline QGraphicsItemAnimationPrivate()
        : q(0), timeLine(0), item(0), step(0)
    { }

    QGraphicsItemAnimation *q;

    QTimeLine *timeLine;
    QGraphicsItem *item;

    QPointF startPos;
    QMatrix startMatrix;

    qreal step;

    struct Pair {
        Pair(qreal a, qreal b) : step(a), value(b) {}
        bool operator <(const Pair &other) const
        { return step < other.step; }
        qreal step;
        qreal value;
    };
    QList<Pair> xPosition;
    QList<Pair> yPosition;
    QList<Pair> rotation;
    QList<Pair> verticalScale;
    QList<Pair> horizontalScale;
    QList<Pair> verticalShear;
    QList<Pair> horizontalShear;
    QList<Pair> xTranslation;
    QList<Pair> yTranslation;

    qreal linearValueForStep(qreal step, QList<Pair> *source, qreal defaultValue = 0);
};

qreal QGraphicsItemAnimationPrivate::linearValueForStep(qreal step, QList<Pair> *source, qreal defaultValue)
{
    if (source->isEmpty())
        return defaultValue;
    step = qMin<qreal>(qMax<qreal>(step, 0), 1);

    qreal stepBefore = 0;
    qreal stepAfter = 1;
    qreal valueBefore = source->first().step == 0 ? source->first().value : defaultValue;
    qreal valueAfter = source->last().value;

    // Find the closest step and value before the given step.
    for (int i = 0; i < source->size() && step > source->at(i).step; ++i) {
        stepBefore = source->at(i).step;
        valueBefore = source->at(i).value;
    }

    // Find the closest step and value after the given step.
    for (int j = source->size() - 1; j >= 0 && step < source->at(j).step; --j) {
        stepAfter = source->at(j).step;
        valueAfter = source->at(j).value;
    }

    // Do a simple linear interpolation.
    return valueBefore + (valueAfter - valueBefore) * ((step - stepBefore) / (stepAfter - stepBefore));
}

/*!
  Constructs an animation object with the given \a parent.
*/
QGraphicsItemAnimation::QGraphicsItemAnimation(QObject *parent)
    : QObject(parent), d(new QGraphicsItemAnimationPrivate)
{
    d->q = this;
}

/*!
  Destroys the animation object.
*/
QGraphicsItemAnimation::~QGraphicsItemAnimation()
{
    delete d;
}

/*!
  Returns the item operated on by the animation object.

  \sa setItem()
*/
QGraphicsItem *QGraphicsItemAnimation::item() const
{
    return d->item;
}

/*!
  Sets the specified \a item to be used in the animation.

  \sa item()
*/
void QGraphicsItemAnimation::setItem(QGraphicsItem *item)
{
    d->item = item;
    d->startPos = d->item->pos();
}

/*!
  Returns the time line object used to control the rate at which the animation
  occurs.

  \sa setTimeLine()
*/
QTimeLine *QGraphicsItemAnimation::timeLine() const
{
    return d->timeLine;
}

/*!
  Sets the time line object used to control the rate of animation to the \a timeLine
  specified.

  \sa timeLine()
*/
void QGraphicsItemAnimation::setTimeLine(QTimeLine *timeLine)
{
    d->timeLine = timeLine;
    connect(timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(setStep(qreal)));
}

/*!
  Returns the point corresponding to the given \a step value.

  \sa setPosAt()
*/
QPointF QGraphicsItemAnimation::posAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::posAt: invalid step = %f", step);

    return QPointF(d->linearValueForStep(step, &d->xPosition, d->startPos.x()),
                   d->linearValueForStep(step, &d->yPosition, d->startPos.y()));
}

/*!
  \fn void QGraphicsItemAnimation::setPosAt(qreal step, const QPointF &point)

  Sets the position of the item at the given \a step value to the \a point specified.

  \sa posAt()
*/
void QGraphicsItemAnimation::setPosAt(qreal step, const QPointF &pos)
{
    if (step < 0.0 || step > 1.0) {
        qWarning("QGraphicsItemAnimation::setPosAt: invalid step = %f", step);
        return;
    }

    d->xPosition << QGraphicsItemAnimationPrivate::Pair(step, pos.x());
    d->yPosition << QGraphicsItemAnimationPrivate::Pair(step, pos.y());
    qSort(d->xPosition.begin(), d->xPosition.end());
    qSort(d->yPosition.begin(), d->yPosition.end());
}

/*!
  Returns the matrix used to transform the item at the specified \a step value.
*/
QMatrix QGraphicsItemAnimation::matrixAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::matrixAt: invalid step = %f", step);

    QMatrix matrix;
    if (!d->rotation.isEmpty())
        matrix.rotate(rotationAt(step));
    if (!d->verticalScale.isEmpty())
        matrix.scale(horizontalScaleAt(step), verticalScaleAt(step));
    if (!d->verticalShear.isEmpty())
        matrix.shear(horizontalShearAt(step), verticalShearAt(step));
    if (!d->xTranslation.isEmpty())
        matrix.translate(xTranslationAt(step), yTranslationAt(step));
    return matrix;
}

/*!
  Returns the angle at which the item is rotated at the specified \a step value.

  \sa setRotationAt()
*/
qreal QGraphicsItemAnimation::rotationAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::rotationAt: invalid step = %f", step);

    return d->linearValueForStep(step, &d->rotation);
}

/*!
  Sets the rotation of the item at the given \a step value to the \a angle specified.

  \sa rotationAt()
*/
void QGraphicsItemAnimation::setRotationAt(qreal step, qreal angle)
{
    if (step < 0.0 || step > 1.0) {
        qWarning("QGraphicsItemAnimation::setRotationAt: invalid step = %f", step);
        return;
    }

    d->rotation << QGraphicsItemAnimationPrivate::Pair(step, angle);
    qSort(d->rotation.begin(), d->rotation.end());
}

/*!
  Returns the horizontal translation of the item at the specified \a step value.

  \sa setTranslationAt()
*/
qreal QGraphicsItemAnimation::xTranslationAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::xTranslationAt: invalid step = %f", step);

    return d->linearValueForStep(step, &d->xTranslation);
}

/*!
  Returns the vertical translation of the item at the specified \a step value.

  \sa setTranslationAt()
*/
qreal QGraphicsItemAnimation::yTranslationAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::yTranslationAt: invalid step = %f", step);

    return d->linearValueForStep(step, &d->yTranslation);
}

/*!
  Sets the translation of the item at the given \a step value using the horizontal
  and vertical coordinates specified by \a dx and \a dy.

  \sa translationAt()
*/
void QGraphicsItemAnimation::setTranslationAt(qreal step, qreal dx, qreal dy)
{
    if (step < 0.0 || step > 1.0) {
        qWarning("QGraphicsItemAnimation::setTranslationAt: invalid step = %f", step);
        return;
    }

    d->xTranslation << QGraphicsItemAnimationPrivate::Pair(step, dx);
    d->yTranslation << QGraphicsItemAnimationPrivate::Pair(step, dy);
    qSort(d->xTranslation.begin(), d->xTranslation.end());
    qSort(d->yTranslation.begin(), d->yTranslation.end());
}

/*!
  Returns the vertical scale for the item at the specified \a step value.

  \sa setScaleAt()
*/
qreal QGraphicsItemAnimation::verticalScaleAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::verticalScaleAt: invalid step = %f", step);

    return d->linearValueForStep(step, &d->verticalScale, 1);
}

/*!
  Returns the horizontal scale for the item at the specified \a step value.

  \sa setScaleAt()
*/
qreal QGraphicsItemAnimation::horizontalScaleAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::horizontalScaleAt: invalid step = %f", step);

    return d->linearValueForStep(step, &d->horizontalScale, 1);
}

/*!
  Sets the scale of the item at the given \a step value using the horizontal and
  vertical scale factors specified by \a sx and \a sy.

  \sa scaleAt()
*/
void QGraphicsItemAnimation::setScaleAt(qreal step, qreal sx, qreal sy)
{
    if (step < 0.0 || step > 1.0) {
        qWarning("QGraphicsItemAnimation::setScaleAt: invalid step = %f", step);
        return;
    }

    d->horizontalScale << QGraphicsItemAnimationPrivate::Pair(step, sx);
    d->verticalScale<< QGraphicsItemAnimationPrivate::Pair(step, sy);
    qSort(d->horizontalScale.begin(), d->horizontalScale.end());
    qSort(d->verticalScale.begin(), d->verticalScale.end());
}

/*!
  Returns the vertical shear for the item at the specified \a step value.

  \sa setShearAt()
*/
qreal QGraphicsItemAnimation::verticalShearAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::verticalShearAt: invalid step = %f", step);

    return d->linearValueForStep(step, &d->verticalShear, 0);
}

/*!
  Returns the horizontal shear for the item at the specified \a step value.

  \sa setShearAt()
*/
qreal QGraphicsItemAnimation::horizontalShearAt(qreal step) const
{
    if (step < 0.0 || step > 1.0)
        qWarning("QGraphicsItemAnimation::horizontalShearAt: invalid step = %f", step);

    return d->linearValueForStep(step, &d->horizontalShear, 0);
}

/*!
  Sets the shear of the item at the given \a step value using the horizontal and
  vertical shear factors specified by \a sh and \a sv.

  \sa shearAt()
*/
void QGraphicsItemAnimation::setShearAt(qreal step, qreal sh, qreal sv)
{
    if (step < 0.0 || step > 1.0) {
        qWarning("QGraphicsItemAnimation::setShearAt: invalid step = %f", step);
        return;
    }

    d->horizontalShear << QGraphicsItemAnimationPrivate::Pair(step, sh);
    d->verticalShear << QGraphicsItemAnimationPrivate::Pair(step, sv);
    qSort(d->horizontalShear.begin(), d->horizontalShear.end());
    qSort(d->verticalShear.begin(), d->verticalShear.end());
}

/*!
  Clears the lists of points and transformations used for the animation, but
  retains the item and time line.
*/
void QGraphicsItemAnimation::clear()
{
    d->xPosition.clear();
    d->yPosition.clear();
    d->rotation.clear();
    d->verticalScale.clear();
    d->horizontalScale.clear();
    d->verticalShear.clear();
    d->horizontalShear.clear();
    d->xTranslation.clear();
    d->yTranslation.clear();
}

/*!
  \fn void QGraphicsItemAnimation::setStep(qreal step)

  Sets the current \a step value for the animation, causing the item to be
  transformed according to the specified parameter and the transformations
  previously set up for the animation.
*/
void QGraphicsItemAnimation::setStep(qreal x)
{
    if (x < 0.0 || x > 1.0) {
        qWarning("QGraphicsItemAnimation::setStep: invalid step = %f", x);
        return;
    }

    d->step = x;
    if (!d->item)
        return;

    if (!d->xPosition.isEmpty() || !d->yPosition.isEmpty())
	d->item->setPos(posAt(x));
    if (!d->rotation.isEmpty()
	|| !d->verticalScale.isEmpty()
	|| !d->horizontalScale.isEmpty()
	|| !d->verticalShear.isEmpty()
	|| !d->horizontalShear.isEmpty()
	|| !d->xTranslation.isEmpty()
	|| !d->yTranslation.isEmpty()) {
	d->item->setMatrix(d->startMatrix * matrixAt(x));
    }
}

/*!
Resets the item to its starting position and transformation.
*/
void QGraphicsItemAnimation::reset()
{
    if (!d->item)
        return;
    d->startPos = d->item->pos();
    d->startMatrix = d->item->matrix();
}

#endif // QT_NO_GRAPHICSVIEW
