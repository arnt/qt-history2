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

#include "qicon.h"
#include "qiconengine.h"
#include "qiconengineplugin.h"
#include "private/qfactoryloader_p.h"
#include "qapplication.h"
#include "qstyleoption.h"
#include "qpainter.h"
#include "qfileinfo.h"
#include "qstyle.h"
#include "qdebug.h"

/*!
  \enum QIcon::Mode

  This enum type describes the mode for which a pixmap is intended to be
  used.
  The currently defined modes are:

    \value Normal
         Display the pixmap when the user is
        not interacting with the icon, but the
        functionality represented by the icon is available.
    \value Disabled
         Display the pixmap when the
        functionality represented by the icon is not available.
    \value Active
         Display the pixmap when the
        functionality represented by the icon is available and
        the user is interacting with the icon, for example, moving the
        mouse over it or clicking it.
*/

/*!
  \enum QIcon::State

  This enum describes the state for which a pixmap is intended to be
  used. The \e state can be:

  \value Off  Display the pixmap when the widget is in an "off" state
  \value On  Display the pixmap when the widget is in an "on" state
*/


class QIconPrivate
{
public:
    QIconPrivate():ref(1),engine(0){}
    ~QIconPrivate() { delete engine; }
    QAtomic ref;
    QIconEngine *engine;
};


struct QPixmapIconEngineEntry
{
    QPixmapIconEngineEntry():mode(QIcon::Normal), state(QIcon::Off){}
    QPixmapIconEngineEntry(const QPixmap &pm, QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off)
        :pixmap(pm), mode(m), state(s){}
    QPixmap pixmap;
    QIcon::Mode mode;
    QIcon::State state;
};

class QPixmapIconEngine : public QIconEngine{
public:
    QPixmapIconEngine(const QPixmap &);
    ~QPixmapIconEngine();
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);
    QSize sizeUsed(const QSize &size, QIcon::Mode mode, QIcon::State state);
    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state);
private:
    QVector<QPixmapIconEngineEntry> pixmaps;
};

QPixmapIconEngine::QPixmapIconEngine(const QPixmap &pm)
{
    if (!pm.isNull())
        pixmaps += QPixmapIconEngineEntry(pm);
}

QPixmapIconEngine::~QPixmapIconEngine()
{
}

void QPixmapIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
}

static inline int area(const QSize &s) { return s.width() * s.height(); }

// returns the largest of the two that is still smaller than size.
static QPixmap bestSizeMatch( const QSize &size, const QPixmap &pa, const QPixmap &pb)
{
    int s = area(size);
    int a = area(pa.size());
    int b = area(pb.size());
    int res = a;
    if (qMax(a,b) <= s)
        res = qMax(a,b);
    else
        res = qMin(a,b);
    if (res == a)
        return pa;
    return pb;
}

QPixmap QPixmapIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (pixmaps.isEmpty())
        return QPixmap();

    bool hasCorrectMode = true;
    QPixmap pm;
    for (int i = 0; i < pixmaps.count(); ++i)
        if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
            if (!pm.isNull())
                pm = bestSizeMatch(size, pixmaps.at(i).pixmap, pm);
            else
                pm = pixmaps.at(i).pixmap;
        }
    if (pm.isNull()) {
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == mode) {
                if (!pm.isNull())
                    pm = bestSizeMatch(size, pixmaps.at(i).pixmap, pm);
                else
                    pm = pixmaps.at(i).pixmap;
            }
    }
    if (pm.isNull()) {
        hasCorrectMode = false;
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode != QIcon::Disabled) {
                if (!pm.isNull())
                    pm = bestSizeMatch(size, pixmaps.at(i).pixmap, pm);
                else
                    pm = pixmaps.at(i).pixmap;
            }
    }

    if (pm.isNull())
        return pm;

    if (!hasCorrectMode) {
        QStyleOption opt(0);
        opt.palette = QApplication::palette();
        pm = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
    }
    if (pm.width() > size.width() || pm.height() > size.height())
        pm = pm.scale(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //### here we might choose to cache the pixmap, but if we do, we
    //### need to through the cache away if the application style or
    //### palette changes

    return pm;
}

// returns the largest of the two that is still smaller than size.
static QSize bestSizeMatch( const QSize &size, const QSize &pa, const QSize &pb)
{
    int s = area(size);
    int a = area(pa);
    int b = area(pb);
    int res = a;
    if (qMax(a,b) <= s)
        res = qMax(a,b);
    else
        res = qMin(a,b);
    if (res == a)
        return pa;
    return pb;
}

QSize QPixmapIconEngine::sizeUsed(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (pixmaps.isEmpty())
        return QSize();

    QSize pm;
    for (int i = 0; i < pixmaps.count(); ++i)
        if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
            if (!pm.isNull())
                pm = bestSizeMatch(size, pixmaps.at(i).pixmap.size(), pm);
            else
                pm = pixmaps.at(i).pixmap.size();
        }
    if (pm.isNull()) {
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == mode) {
                if (!pm.isNull())
                    pm = bestSizeMatch(size, pixmaps.at(i).pixmap.size(), pm);
                else
                    pm = pixmaps.at(i).pixmap.size();
            }
    }
    if (pm.isNull()) {
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode != QIcon::Disabled) {
                if (!pm.isNull())
                    pm = bestSizeMatch(size, pixmaps.at(i).pixmap.size(), pm);
                else
                    pm = pixmaps.at(i).pixmap.size();
            }
    }

    if (pm.width() > size.width() || pm.height() > size.height())
        pm.scale(size, Qt::KeepAspectRatio);

    return pm;
}

void QPixmapIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
    if (!pixmap.isNull())
        pixmaps += QPixmapIconEngineEntry(pixmap, mode, state);
}


#ifndef QT_NO_COMPONENT
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QIconEngineFactoryInterface_iid, QCoreApplication::libraryPaths(), "/iconengines", Qt::CaseInsensitive))
#endif



/*!
  \class QIcon

  \brief The QIcon class provides scalable icons in different modes
  and states.

  \ingroup multimedia
  \ingroup shared
  \mainclass

  A QIcon can generate smaller, larger, active, and disabled pixmaps
  from the set of pixmaps it is given. Such pixmaps are used by Qt
  widgets to show an icon representing a particular action.

  The simplest use of QIcon is to create one from a QPixmap file or
  resource, and then use it, allowing Qt to work out all the required
  icon styles and sizes. For example:

  \code
    QToolButton *button = new QToolButton;
    button->setIcon(QIcon("open.xpm"));
  \endcode

  When you retrieve a pixmap using pixmap(QSize, Mode, State), and no
  pixmap for this given size, mode and state has been added with
  addPixmap(), then QIcon will generate one on the fly. This pixmap
  generation happens in a QIconEngine. The default engine scales
  pixmaps down if required, but never up, and it uses the current
  style to calculate a disabled appearance. By using custom icon
  engines, you can customize every aspect of generated icons. With
  QIconEnginePlugin it is possible to register different icon engines
  for different file suffixes, so you could provide a SVG icon engine
  or any other scalable format.

  \section1 Making Classes that Use QIcon

  If you write your own widgets that have an option to set a small
  pixmap, consider allowing a QIcon to be set for that pixmap.  The
  Qt class QToolButton is an example of such a widget.

  Provide a method to set a QIcon, and when you draw the icon, choose
  whichever pixmap is appropriate for the current state of your widget.
  For example:
  \code
    void MyWidget::drawIcon(QPainter *painter, QPoint pos)
    {
        QPixmap pixmap = icon.pixmap(Qt::SmallIconSize,
                                       isEnabled() ? QIcon::Normal
                                                   : QIcon::Disabled,
                                       isOn() ? QIcon::On
                                              : QIcon::Off);
        painter->drawPixmap(pos, pixmap);
    }
  \endcode

  You might also make use of the \c Active mode, perhaps making your
  widget \c Active when the mouse is over the widget (see \l
  QWidget::enterEvent()), while the mouse is pressed pending the
  release that will activate the function, or when it is the currently
  selected item. If the widget can be toggled, the "On" mode might be
  used to draw a different icon.

  \img icon.png QIcon

  \sa \link guibooks.html#fowler GUI Design Handbook: Iconic Label \endlink
*/


/*!
  Constructs a null icon.
*/
QIcon::QIcon()
    : d(0)
{
}

/*!
  Constructs an icon from a \a pixmap.
 */
QIcon::QIcon(const QPixmap &pixmap)
    :d(new QIconPrivate)
{
    d->engine = new QPixmapIconEngine(pixmap);
}

/*!
  Constructs a copy of \a other. This is very fast.
*/
QIcon::QIcon(const QIcon &other)
    :d(other.d)
{
    if (d)
        ++d->ref;
}

/*!
    Constructs an icon from the file with the given \a fileName. If
    the file does not exist or is of an unknown format, the icon
    becomes a null icon.

    If \a fileName contains a relative path (e.g. the filename only)
    the relevant file must be found relative to the runtime working
    directory.

    The file name can be either refer to an actual file on disk or to
    one of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how to
    embed images and other resource files in the application's
    executable.
*/
QIcon::QIcon(const QString &fileName)
    :d(new QIconPrivate)
{
    QFileInfo info(fileName);
    QString suffix = info.suffix();
    if (!suffix.isEmpty())
        if (QIconEngineFactoryInterface *factory = qt_cast<QIconEngineFactoryInterface*>(loader()->instance(suffix)))
            d->engine = factory->create(fileName);
    if (!d->engine)
        d->engine = new QPixmapIconEngine(QPixmap(fileName));
}


/*!  Creates an icon with a specific icon \a engine. The icon takes
 *   ownership of the engine.
 */
QIcon::QIcon(QIconEngine *engine)
    :d(new QIconPrivate)
{
    d->engine = engine;
}

/*!
    Destroys the icon.
*/
QIcon::~QIcon()
{
    if (d && !--d->ref)
        delete d;
}

/*!
    Assigns the \a other icon to this icon and returns a reference to
    this icon.
*/
QIcon &QIcon::operator=(const QIcon &other)
{
    QIconPrivate *x = other.d;
    if (x)
        ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (x && !--x->ref)
        delete x;
    return *this;
}


/*!  Returns a pixmap with the required \a size, \a mode, and \a
  state, generating one if necessary. The pixmap might be smaller than
  requested, but never larger.
*/
QPixmap QIcon::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) const
{
    if (!d)
        return QPixmap();
    return d->engine->pixmap(size, mode, state);
}

/*! \overload
  Returns a pixmap with the required \a size, \a mode, and
  \a state, generating one if necessary.
*/
QPixmap QIcon::pixmap(Qt::IconSize size, QIcon::Mode mode, QIcon::State state) const
{
    if (!d)
        return QPixmap();
    return d->engine->pixmap(sizeHint(size), mode, state);
}


/*! Uses the \a painter to paint the icon with specified \a alignment,
 *  required \a mode, and \a state into the rectangle \a rect/
*/
void QIcon::paint(QPainter *painter, const QRect &rect,  Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state) const
{
    if (!d)
        return;
    QRect alignedRect = QStyle::alignedRect(QApplication::layoutDirection(), alignment, d->engine->sizeUsed(rect.size(), mode, state), rect);
    d->engine->paint(painter, alignedRect, mode, state);
}


/*!
    Returns true if the icon is empty; otherwise returns false.
*/
bool QIcon::isNull() const
{
    return !d;
}

/*!\internal
 */
bool QIcon::isDetached() const
{
    return !d || d->ref == 1;
}


/*!  Adds \a pixmap to the icon, as a specialization for \a mode and
 *   \a state. Note: custom icon engines are free to ignore
 *   additionally added pixmaps.
 */
void QIcon::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
    if (!d || pixmap.isNull())
        return;
    d->engine->addPixmap(pixmap, mode, state);
}

/*!
  Returns the style's preferred icon size for a give logical \a size.
 */
QSize QIcon::sizeHint(Qt::IconSize size)
{
    int sz = QApplication::style()->pixelMetric(size == Qt::LargeIconSize ? QStyle::PM_LargeIconSize : QStyle::PM_SmallIconSize);
    return QSize(sz, sz);
}




#ifdef QT_COMPAT

static int widths[2] = { 22, 32 };
static int heights[2] = { 22, 32 };

/*! \compat */
static QSize pixmapSize(QIcon::Size which) {
    int i = 0;
    if (which == QIcon::Large)
        i = 1;
    return QSize(widths[i], heights[i]);
}

/*! \compat */
QPixmap QIcon::pixmap(Size size, QIcon::Mode mode, QIcon::State state) const
{ return pixmap(::pixmapSize(size), mode, state); }
/*! \compat */
QPixmap QIcon::pixmap(Size size, bool enabled, QIcon::State state) const
{ return pixmap(::pixmapSize(size), enabled ? Normal : Disabled, state); }
/*! \compat */
QPixmap QIcon::pixmap() const
{ return pixmap(::pixmapSize(Small), Normal, Off); }



/*!
  \compat

  Set the preferred size for all small or large pixmaps that are
  generated after this call. If \a which is Small, sets the preferred
  size of small generated pixmaps to \a size. Similarly, if \a which is
  Large, sets the preferred size of large generated pixmaps to \a size.

  Note that cached pixmaps will not be regenerated, so it is recommended
  that you set the preferred icon sizes before generating any icon sets.
  Also note that the preferred icon sizes will be ignored for icon sets
  that have been created using both small and large pixmaps.

  \sa pixmapSize()
*/
void QIcon::setPixmapSize(QIcon::Size which, const QSize &size)
{
    int i = 0;
    if (which == Large)
        i = 1;
    widths[i] = size.width();
    heights[i] = size.height();
}

/*!
    \compat

    If \a which is Small, returns the preferred size of a small
    generated icon; if \a which is Large, returns the preferred size
    of a large generated icon.

  \sa setPixmapSize()
*/
QSize QIcon::pixmapSize(QIcon::Size which)
{
    return ::pixmapSize(which);
}

#endif // QT_COMPAT


