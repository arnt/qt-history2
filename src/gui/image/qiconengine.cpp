/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qiconengine.h"
#include "qpainter.h"

/*!
  \class QIconEngine

  \brief The QIconEngine class provides an abstract base class to be used as backend for QIcon.

  \ingroup multimedia

  An icon engine is the backend of a QIcon, every icon owns one icon
  engine. The icon engine is responsible for drawing an icon in a
  requested size, mode and state. This happens in the virtual function
  paint(). Additionally there is a virtual function pixmap() that
  returns the icon as pixmap (the default implementation simply uses
  paint()), and another virtual addPixmap(), which is called by QIcon
  when the users adds specicializations.

  \sa QIconEnginePlugin

*/

/*!\fn  virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) = 0;

  Uses the \a painter to paint the icon with the required \a mode, and
  \a state into the rectangle \a rect.
*/

/*!  Returns the size the engine is able to scale the icon to for a
  requested \a size, \a mode and \a state. The default returns \a
  size.
 */
QSize QIconEngine::sizeUsed(const QSize &size, QIcon::Mode /*mode*/, QIcon::State /*state*/)
{
    return size;
}


/*!
  Destroys the icon engine.
 */
QIconEngine::~QIconEngine()
{
}


/*!  Returns the icon as pixmap with the required \a size, \a mode,
  and \a state. The default implementation creates a new pixmap and
  calls paint() to fill it.
*/
QPixmap QIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pm(size);
    {
        QPainter p(&pm);
        paint(&p, QRect(QPoint(0,0),size), mode, state);
    }
    return pm;
}

/*!  Called by QIcon::addPixmap(). Adds a specialization \a pixmap for
  \a mode and \a state. The default pixmap-based engine stores those
  pixmaps and uses them later. Custom icon engines that implement
  scalable vector formats are free to ignores those extra pixmaps.
 */
void QIconEngine::addPixmap(const QPixmap &/*pixmap*/, QIcon::Mode /*mode*/, QIcon::State /*state*/)
{
}
