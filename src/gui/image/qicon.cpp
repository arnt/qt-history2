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
#include "qpixmapcache.h"
#include "qvariant.h"
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

static int serialNumCounter = 0;

class QIconPrivate
{
public:
    QIconPrivate():ref(1),engine(0),serialNum(++serialNumCounter){}
    ~QIconPrivate() { delete engine; }
    QAtomic ref;
    QIconEngine *engine;
    int serialNum;
};


struct QPixmapIconEngineEntry
{
    QPixmapIconEngineEntry():mode(QIcon::Normal), state(QIcon::Off){}
    QPixmapIconEngineEntry(const QPixmap &pm, QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off)
        :pixmap(pm), size(pm.size()), mode(m), state(s){}
    QPixmapIconEngineEntry(const QString &file, const QSize &sz = QSize(), QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off)
        :fileName(file), size(sz), mode(m), state(s){}
    QPixmap pixmap;
    QString fileName;
    QSize size;
    QIcon::Mode mode;
    QIcon::State state;
    bool isNull() const {return (fileName.isEmpty() && pixmap.isNull()); }
};

class QPixmapIconEngine : public QIconEngine{
public:
    QPixmapIconEngine();
    ~QPixmapIconEngine();
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state);
    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state);
private:
    QVector<QPixmapIconEngineEntry> pixmaps;
};

QPixmapIconEngine::QPixmapIconEngine()
{
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
static QPixmapIconEngineEntry bestSizeMatch( const QSize &size, const QPixmapIconEngineEntry &pa, const QPixmapIconEngineEntry &pb)
{
    int s = area(size);
    int a = area(pa.size);
    int b = area(pb.size);
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
    QPixmapIconEngineEntry pe;

    // look for right mode and right state
    for (int i = 0; i < pixmaps.count(); ++i)
        if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
            if (!pe.isNull())
                pe = bestSizeMatch(size, pixmaps.at(i), pe);
            else
                pe = pixmaps.at(i);
        }
    if (pe.isNull()) {
        // look for right mode, ignore state
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == mode) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
                }
    }
    if (pe.isNull()) {
        // merge active and normal mode, and look for right state
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == (mode == QIcon::Disabled ? QIcon::Disabled : QIcon::Normal) && pixmaps.at(i).state == state) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
            }
    }
    if (pe.isNull()) {
        // merge active and normal mode,  ignore state
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == (mode == QIcon::Disabled ? QIcon::Disabled : QIcon::Normal)) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
            }
    }

    if (pe.isNull()) {
        // fallback: look for a normal one
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == QIcon::Normal) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
            }
    }

    if (pe.isNull())
        return pe.pixmap;

    if (pe.pixmap.isNull()) {
        pe.pixmap = QPixmap(pe.fileName);
        if (!pe.pixmap.isNull())
            pe.size = pe.pixmap.size();
    }

    QPixmap pm = pe.pixmap;
    if (pm.isNull())
        return pm;

    QSize actualSize = pm.size();
    actualSize.scale(size, Qt::KeepAspectRatio);

    QString key = QLatin1String("$qt_icon_")
                  + QString::number(pm.serialNumber())
                  + QString::number(actualSize.width())
                  + QLatin1Char('_')
                  + QString::number(actualSize.height())
                  + QLatin1Char('_')
                  + QString::number(pe.state)
                  + QLatin1Char('_');


    if (mode == QIcon::Active) {
        if (QPixmapCache::find(key + QString::number(mode), pm))
            return pm; // horray
        if (QPixmapCache::find(key + QString::number(QIcon::Normal), pm)) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            QPixmap active = QApplication::style()->generatedIconPixmap(QIcon::Active, pm, &opt);
            if (pm.serialNumber() == active.serialNumber())
                return pm;
        }
    }

    if (!QPixmapCache::find(key + QString::number(mode), pm)) {
        if (pe.mode == QIcon::Normal && pe.mode != mode) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
            if (generated.isNull())
                return pm;
            pm = generated;
        }
        if (pm.width() > size.width() || pm.height() > size.height())
            pm = pm.scale(actualSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (pm.isDetached())
            QPixmapCache::insert(key + QString::number(mode), pm);
    }
    return pm;
}

QSize QPixmapIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmapIconEngineEntry pe;

    // look for right mode and right state
    for (int i = 0; i < pixmaps.count(); ++i)
        if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
            if (!pe.isNull())
                pe = bestSizeMatch(size, pixmaps.at(i), pe);
            else
                pe = pixmaps.at(i);
        }
    if (pe.isNull()) {
        // look for right mode, ignore state
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == mode) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
                }
    }
    if (pe.isNull()) {
        // merge active and normal mode, and look for right state
        for (int i = 0; i < pixmaps.count(); ++i)
            if ((pixmaps.at(i).mode == QIcon::Disabled ? QIcon::Disabled : QIcon::Normal) == mode && pixmaps.at(i).state == state) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
            }
    }
    if (pe.isNull()) {
        // merge active and normal mode,  ignore state
        for (int i = 0; i < pixmaps.count(); ++i)
            if ((pixmaps.at(i).mode == QIcon::Disabled ? QIcon::Disabled : QIcon::Normal) == mode) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
            }
    }

    if (pe.isNull()) {
        // fallback: look for a normal one
        for (int i = 0; i < pixmaps.count(); ++i)
            if (pixmaps.at(i).mode == QIcon::Normal) {
                if (!pe.isNull())
                    pe = bestSizeMatch(size, pixmaps.at(i), pe);
                else
                    pe = pixmaps.at(i);
            }
    }

    if (pe.isNull())
        return QSize();

    if (pe.size.isNull()) {
        pe.pixmap = QPixmap(pe.fileName);
        if (!pe.pixmap.isNull())
            pe.size = pe.pixmap.size();
    }


    QSize actualSize = pe.size;
    if (!actualSize.isNull())
        actualSize.scale(size, Qt::KeepAspectRatio);
    return actualSize;
}

void QPixmapIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
    if (!pixmap.isNull())
        pixmaps += QPixmapIconEngineEntry(pixmap, mode, state);
}

void QPixmapIconEngine::addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (!fileName.isEmpty()) {
        QString abs = fileName;
        if (fileName.at(0) != QLatin1Char(':'))
            abs = QFileInfo(fileName).absoluteFilePath();
        pixmaps += QPixmapIconEngineEntry(abs, size, mode, state);
    }
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
    :d(0)
{
    addPixmap(pixmap);
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
    Constructs an icon from the file with the given \a fileName. The
    file will be loaded on demand. If the file does not exist or is of
    an unknown format, the icon becomes a null icon.

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
    :d(0)
{
    QFileInfo info(fileName);
    QString suffix = info.suffix();
    if (!suffix.isEmpty())
        if (QIconEngineFactoryInterface *factory = qobject_cast<QIconEngineFactoryInterface*>(loader()->instance(suffix)))
            if (QIconEngine *engine = factory->create(fileName)) {
                d = new QIconPrivate;
                d->engine = engine;
                return;
            }
    addFile(fileName);
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

/*!
   Returns the icon as a QVariant
*/
QIcon::operator QVariant() const
{
    extern bool qRegisterGuiVariant();
    static const bool b = qRegisterGuiVariant();
    Q_UNUSED(b)
    return QVariant(QVariant::Icon, this);
}

/*!
    Returns a number that uniquely identifies the contents of this
    QIcon object. This means that multiple QIcon objects can have
    the same serial number as long as they refer to the same contents.

    A null icon always have a serial number of 0.

    \sa QPixmap::serialNumber()
*/

int QIcon::serialNumber() const
{
    return d ? d->serialNum : 0;
}

/*!  Returns a pixmap with the requested \a size, \a mode, and \a
  state, generating one if necessary. The pixmap might be smaller than
  requested, but never larger.
*/
QPixmap QIcon::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) const
{
    if (!d)
        return QPixmap();
    return d->engine->pixmap(size, mode, state);
}


/*!  Returns the actual size of the icon for the requested \a size, \a
  mode, and \a state. The result might be smaller than requested, but
  never larger.
*/
QSize QIcon::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) const
{
    if (!d)
        return QSize();
    return d->engine->actualSize(size, mode, state);
}


/*! Uses the \a painter to paint the icon with specified \a alignment,
 *  required \a mode, and \a state into the rectangle \a rect/
*/
void QIcon::paint(QPainter *painter, const QRect &rect,  Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state) const
{
    if (!d || !painter)
        return;
    QRect alignedRect = QStyle::alignedRect(painter->layoutDirection(), alignment, d->engine->actualSize(rect.size(), mode, state), rect);
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
    if (pixmap.isNull())
        return;
    if (!d) {
        d = new QIconPrivate;
        d->engine = new QPixmapIconEngine;
    }
    d->engine->addPixmap(pixmap, mode, state);
}


/*!  Adds a pixmap from the file with the given \a fileName to the
     icon, as a specialization for \a size, \a mode and \a state. The
     file will be loaded on demand. Note: custom icon engines are free
     to ignore additionally added pixmaps.

     If \a fileName contains a relative path (e.g. the filename only)
     the relevant file must be found relative to the runtime working
     directory.

    The file name can be either refer to an actual file on disk or to
    one of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how to
    embed images and other resource files in the application's
    executable.
 */
void QIcon::addFile(const QString &fileName, const QSize &size, Mode mode, State state)
{
    if (fileName.isEmpty())
        return;
    if (!d) {
        d = new QIconPrivate;
        d->engine = new QPixmapIconEngine;
    }
    d->engine->addFile(fileName, size, mode, state);
}



#ifdef QT3_SUPPORT

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

#endif // QT3_SUPPORT


