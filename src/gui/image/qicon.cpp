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

    This enum type describes the mode for which a pixmap is intended
    to be used. The currently defined modes are:

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
   \value Selected
        Display the pixmap when the item represented by the icon is
        selected.
*/

/*!
  \enum QIcon::State

  This enum describes the state for which a pixmap is intended to be
  used. The \e state can be:

  \value Off  Display the pixmap when the widget is in an "off" state
  \value On  Display the pixmap when the widget is in an "on" state
*/

extern Q_GUI_EXPORT qint64 qt_pixmap_id(const QPixmap &pixmap);

QBasicAtomic serialNumCounter = Q_ATOMIC_INIT(0);

class QIconPrivate
{
public:
    QIconPrivate():ref(1), engine(0), serialNum(serialNumCounter.fetchAndAdd(1)), detach_no(0), engine_version(2) {}

    ~QIconPrivate() { delete engine; }
    QAtomic ref;
    QIconEngine *engine;
    int serialNum;
    int detach_no;
    int engine_version;
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

class QPixmapIconEngine : public QIconEngineV2 {
public:
    QPixmapIconEngine();
    QPixmapIconEngine(const QPixmapIconEngine &);
    ~QPixmapIconEngine();
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);
    QPixmapIconEngineEntry *bestMatch(const QSize &size, QIcon::Mode mode, QIcon::State state, bool sizeOnly);
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state);
    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state);

    // v2 functions
    QString key() const;
    QIconEngineV2 *clone() const;
    bool read(QDataStream &in);
    bool write(QDataStream &out) const;

private:
    QPixmapIconEngineEntry *tryMatch(const QSize &size, QIcon::Mode mode, QIcon::State state);
    QVector<QPixmapIconEngineEntry> pixmaps;

    friend QDataStream &operator<<(QDataStream &s, const QIcon &icon);
};

QPixmapIconEngine::QPixmapIconEngine()
{
}

QPixmapIconEngine::QPixmapIconEngine(const QPixmapIconEngine &other)
    : QIconEngineV2(other), pixmaps(other.pixmaps)
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

// returns the smallest of the two that is still larger than or equal to size.
static QPixmapIconEngineEntry *bestSizeMatch( const QSize &size, QPixmapIconEngineEntry *pa, QPixmapIconEngineEntry *pb)
{
    int s = area(size);
    if (pa->size == QSize() && pa->pixmap.isNull()) {
        pa->pixmap = QPixmap(pa->fileName);
        pa->size = pa->pixmap.size();
    }
    int a = area(pa->size);
    if (pb->size == QSize() && pb->pixmap.isNull()) {
        pb->pixmap = QPixmap(pb->fileName);
        pb->size = pb->pixmap.size();
    }
    int b = area(pb->size);
    int res = a;
    if (qMin(a,b) >= s)
        res = qMin(a,b);
    else
        res = qMax(a,b);
    if (res == a)
        return pa;
    return pb;
}

QPixmapIconEngineEntry *QPixmapIconEngine::tryMatch(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmapIconEngineEntry *pe = 0;
    for (int i = 0; i < pixmaps.count(); ++i)
        if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
            if (pe)
                pe = bestSizeMatch(size, &pixmaps[i], pe);
            else
                pe = &pixmaps[i];
        }
    return pe;
}


QPixmapIconEngineEntry *QPixmapIconEngine::bestMatch(const QSize &size, QIcon::Mode mode, QIcon::State state, bool sizeOnly)
{
    QPixmapIconEngineEntry *pe = tryMatch(size, mode, state);
    while (!pe){
        QIcon::State oppositeState = (state == QIcon::On) ? QIcon::Off : QIcon::On;
        if (mode == QIcon::Disabled || mode == QIcon::Selected) {
            QIcon::Mode oppositeMode = (mode == QIcon::Disabled) ? QIcon::Selected : QIcon::Disabled;
            if ((pe = tryMatch(size, QIcon::Normal, state)))
                break;
            if ((pe = tryMatch(size, QIcon::Active, state)))
                break;
            if ((pe = tryMatch(size, mode, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Normal, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Active, oppositeState)))
                break;
            if ((pe = tryMatch(size, oppositeMode, state)))
                break;
            if ((pe = tryMatch(size, oppositeMode, oppositeState)))
                break;
        } else {
            QIcon::Mode oppositeMode = (mode == QIcon::Normal) ? QIcon::Active : QIcon::Normal;
            if ((pe = tryMatch(size, oppositeMode, state)))
                break;
            if ((pe = tryMatch(size, mode, oppositeState)))
                break;
            if ((pe = tryMatch(size, oppositeMode, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Disabled, state)))
                break;
            if ((pe = tryMatch(size, QIcon::Selected, state)))
                break;
            if ((pe = tryMatch(size, QIcon::Disabled, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Selected, oppositeState)))
                break;
        }

        if (!pe)
            return pe;
    }

    if (sizeOnly ? (pe->size.isNull() || !pe->size.isValid()) : pe->pixmap.isNull()) {
        pe->pixmap = QPixmap(pe->fileName);
        if (!pe->pixmap.isNull())
            pe->size = pe->pixmap.size();
    }

    return pe;
}

QPixmap QPixmapIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pm;
    QPixmapIconEngineEntry *pe = bestMatch(size, mode, state, false);
    if (pe)
        pm = pe->pixmap;

    if (pm.isNull())
        return pm;

    QSize actualSize = pm.size();
    if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height()))
        actualSize.scale(size, Qt::KeepAspectRatio);

    QString key = QLatin1String("$qt_icon_")
                  + QString::number(qt_pixmap_id(pm))
                  + QString::number(actualSize.width())
                  + QLatin1Char('_')
                  + QString::number(actualSize.height())
                  + QLatin1Char('_');


    if (mode == QIcon::Active) {
        if (QPixmapCache::find(key + QString::number(mode), pm))
            return pm; // horray
        if (QPixmapCache::find(key + QString::number(QIcon::Normal), pm)) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            QPixmap active = QApplication::style()->generatedIconPixmap(QIcon::Active, pm, &opt);
            if (qt_pixmap_id(pm) == qt_pixmap_id(active))
                return pm;
        }
    }

    if (!QPixmapCache::find(key + QString::number(mode), pm)) {
        if (pe->mode != mode && mode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
            if (!generated.isNull())
                pm = generated;
        }
        if (pm.size() != actualSize)
            pm = pm.scaled(actualSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(key + QString::number(mode), pm);
    }
    return pm;
}

QSize QPixmapIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QSize actualSize;
    if (QPixmapIconEngineEntry *pe = bestMatch(size, mode, state, true))
        actualSize = pe->size;

    if (actualSize.isNull())
        return actualSize;

    if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height()))
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

QString QPixmapIconEngine::key() const
{
    return QLatin1String("QPixmapIconEngine");
}

QIconEngineV2 *QPixmapIconEngine::clone() const
{
    return new QPixmapIconEngine(*this);
}

bool QPixmapIconEngine::read(QDataStream &in)
{
    int num_entries;
    QPixmap pm;
    QString fileName;
    QSize sz;
    uint mode;
    uint state;

    in >> num_entries;
    for (int i=0; i < num_entries; ++i) {
        if (in.atEnd()) {
            pixmaps.clear();
            return false;
        }
        in >> pm;
        in >> fileName;
        in >> sz;
        in >> mode;
        in >> state;
        if (pm.isNull())
            addFile(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
        else
            addPixmap(pm, QIcon::Mode(mode), QIcon::State(state));
    }
    return true;
}

bool QPixmapIconEngine::write(QDataStream &out) const
{
    int num_entries = pixmaps.size();
    out << num_entries;
    for (int i=0; i < num_entries; ++i) {
        if (pixmaps.at(i).pixmap.isNull())
            out << QPixmap(pixmaps.at(i).fileName);
        else
            out << pixmaps.at(i).pixmap;
        out << pixmaps.at(i).fileName;
        out << pixmaps.at(i).size;
        out << (uint) pixmaps.at(i).mode;
        out << (uint) pixmaps.at(i).state;
    }
    return true;
}

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QIconEngineFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/iconengines"), Qt::CaseInsensitive))
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loaderV2,
    (QIconEngineFactoryInterfaceV2_iid, QCoreApplication::libraryPaths(), QLatin1String("/iconengines"), Qt::CaseInsensitive))
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

  Use the QImageReader::supportedImageFormats() and
  QImageWriter::supportedImageFormats() functions to retrieve a
  complete list of the supported file formats.

  When you retrieve a pixmap using pixmap(QSize, Mode, State), and no
  pixmap for this given size, mode and state has been added with
  addFile() or addPixmap(), then QIcon will generate one on the
  fly. This pixmap generation happens in a QIconEngine. The default
  engine scales pixmaps down if required, but never up, and it uses
  the current style to calculate a disabled appearance. By using
  custom icon engines, you can customize every aspect of generated
  icons. With QIconEnginePlugin it is possible to register different
  icon engines for different file suffixes, so you could provide a SVG
  icon engine or any other scalable format.

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
        QPixmap pixmap = icon.pixmap(QSize(22, 22),
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

  \sa {fowler}{GUI Design Handbook: Iconic Label}, {Icons Example}
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
        d->ref.ref();
}

/*!
    Constructs an icon from the file with the given \a fileName. The
    file will be loaded on demand.

    If \a fileName contains a relative path (e.g. the filename only)
    the relevant file must be found relative to the runtime working
    directory.

    The file name can be either refer to an actual file on disk or to
    one of the application's embedded resources.  See the
    \l{resources.html}{Resource System} overview for details on how to
    embed images and other resource files in the application's
    executable.

    Use the QImageReader::supportedImageFormats() and
    QImageWriter::supportedImageFormats() functions to retrieve a
    complete list of the supported file formats.
*/
QIcon::QIcon(const QString &fileName)
    : d(0)
{
    QFileInfo info(fileName);
    QString suffix = info.suffix();
#ifndef QT_NO_LIBRARY
    if (!suffix.isEmpty()) {
        // first try version 2 engines..
        if (QIconEngineFactoryInterfaceV2 *factory = qobject_cast<QIconEngineFactoryInterfaceV2*>(loaderV2()->instance(suffix))) {
            if (QIconEngine *engine = factory->create(fileName)) {
                d = new QIconPrivate;
                d->engine = engine;
                return;
            }
        }
        // ..then fall back and try to load version 1 engines
        if (QIconEngineFactoryInterface *factory = qobject_cast<QIconEngineFactoryInterface*>(loader()->instance(suffix))) {
            if (QIconEngine *engine = factory->create(fileName)) {
                d = new QIconPrivate;
                d->engine = engine;
                d->engine_version = 1;
                return;
            }
        }
    }
#endif
    addFile(fileName);
}


/*!
    Creates an icon with a specific icon \a engine. The icon takes
    ownership of the engine.
*/
QIcon::QIcon(QIconEngine *engine)
    :d(new QIconPrivate)
{
    d->engine_version = 1;
    d->engine = engine;
}

/*!
    Creates an icon with a specific icon \a engine. The icon takes
    ownership of the engine.
*/
QIcon::QIcon(QIconEngineV2 *engine)
    :d(new QIconPrivate)
{
    d->engine_version = 2;
    d->engine = engine;
}


/*!
    Destroys the icon.
*/
QIcon::~QIcon()
{
    if (d && !d->ref.deref())
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
        x->ref.ref();
    x = qAtomicSetPtr(&d, x);
    if (x && !x->ref.deref())
        delete x;
    return *this;
}

/*!
   Returns the icon as a QVariant.
*/
QIcon::operator QVariant() const
{
    return QVariant(QVariant::Icon, this);
}

/*! \obsolete

    Returns a number that identifies the contents of this
    QIcon object. Distinct QIcon objects can have
    the same serial number if they refer to the same contents
    (but they don't have to). Also, the serial number of
    a QIcon object may change during its lifetime.

    Use cacheKey() instead.

    A null icon always has a serial number of 0.

    Serial numbers are mostly useful in conjunction with caching.

    \sa QPixmap::serialNumber()
*/

int QIcon::serialNumber() const
{
    return d ? d->serialNum : 0;
}

/*!
    Returns a number that identifies the contents of this QIcon
    object. Distinct QIcon objects can have the same key if
    they refer to the same contents.
    \since 4.3

    The cacheKey() will change when the icon is altered via
    addPixmap() or addFile().
*/
qint64 QIcon::cacheKey() const
{
    if (!d)
        return 0;
    return (((qint64) d->serialNum) << 32) | ((qint64) (d->detach_no));
}

/*!
  Returns a pixmap with the requested \a size, \a mode, and \a
  state, generating one if necessary. The pixmap might be smaller than
  requested, but never larger.

  \sa actualSize(), paint()
*/
QPixmap QIcon::pixmap(const QSize &size, Mode mode, State state) const
{
    if (!d)
        return QPixmap();
    return d->engine->pixmap(size, mode, state);
}

/*!
    \fn QPixmap QIcon::pixmap(int w, int h, Mode mode = Normal, State state = Off) const

    \overload

    Returns a pixmap of size QSize(\a w, \a h). The pixmap might be smaller than
    requested, but never larger.
*/

/*!
    \fn QPixmap QIcon::pixmap(int extent, Mode mode = Normal, State state = Off) const

    \overload

    Returns a pixmap of size QSize(\a extent, \a extent). The pixmap might be smaller
    than requested, but never larger.
*/

/*!  Returns the actual size of the icon for the requested \a size, \a
  mode, and \a state. The result might be smaller than requested, but
  never larger.

  \sa pixmap(), paint()
*/
QSize QIcon::actualSize(const QSize &size, Mode mode, State state) const
{
    if (!d)
        return QSize();
    return d->engine->actualSize(size, mode, state);
}


/*!
    Uses the \a painter to paint the icon with specified \a alignment,
    required \a mode, and \a state into the rectangle \a rect.

    \sa actualSize(), pixmap()
*/
void QIcon::paint(QPainter *painter, const QRect &rect, Qt::Alignment alignment, Mode mode, State state) const
{
    if (!d || !painter)
        return;
    QRect alignedRect = QStyle::alignedRect(painter->layoutDirection(), alignment, d->engine->actualSize(rect.size(), mode, state), rect);
    d->engine->paint(painter, alignedRect, mode, state);
}

/*!
    \fn void QIcon::paint(QPainter *painter, int x, int y, int w, int h, Qt::Alignment alignment,
                          Mode mode, State state) const

    \overload

    Paints the icon into the rectangle QRect(\a x, \a y, \a w, \a h).
*/

/*!
    Returns true if the icon is empty; otherwise returns false.

    An icon is empty if it has neither a pixmap nor a filename.

    Note: Even a non-null icon might not be able to create valid
    pixmaps, eg. if the file does not exist or cannot be read.
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

/*! \internal
 */
void QIcon::detach()
{
    if (d) {
        if (d->ref != 1) {
            QIconPrivate *x = new QIconPrivate;
            if (d->engine_version > 1) {
                QIconEngineV2 *engine = static_cast<QIconEngineV2 *>(d->engine);
                x->engine = engine->clone();
            } else {
                x->engine = d->engine;
            }
            x->engine_version = d->engine_version;
            x = qAtomicSetPtr(&d, x);
            if (!x->ref.deref())
                delete x;
        }
        ++d->detach_no;
    }
}

/*!
    Adds \a pixmap to the icon, as a specialization for \a mode and
    \a state.

    Custom icon engines are free to ignore additionally added
    pixmaps.

    \sa addFile()
*/
void QIcon::addPixmap(const QPixmap &pixmap, Mode mode, State state)
{
    if (pixmap.isNull())
        return;
    if (!d) {
        d = new QIconPrivate;
        d->engine = new QPixmapIconEngine;
    } else {
        detach();
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

    Use the QImageReader::supportedImageFormats() and
    QImageWriter::supportedImageFormats() functions to retrieve a
    complete list of the supported file formats.

    \sa addPixmap()
 */
void QIcon::addFile(const QString &fileName, const QSize &size, Mode mode, State state)
{
    if (fileName.isEmpty())
        return;
    if (!d) {
        d = new QIconPrivate;
        d->engine = new QPixmapIconEngine;
    } else {
        detach();
    }
    d->engine->addFile(fileName, size, mode, state);
}

/*****************************************************************************
  QIcon stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM)
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QIcon &icon)
    \relates QIcon
    \since 4.2

    Writes the given \a icon to the the given \a stream as a PNG
    image. If the icon contains more than one image, all images will
    be written to the stream. Note that writing the stream to a file
    will not produce a valid image file.
*/

QDataStream &operator<<(QDataStream &s, const QIcon &icon)
{
    if (s.version() >= QDataStream::Qt_4_3) {
        if (icon.isNull()) {
            s << QString();
        } else {
            if (icon.d->engine_version > 1) {
                QIconEngineV2 *engine = static_cast<QIconEngineV2 *>(icon.d->engine);
                s << engine->key();
                engine->write(s);
            } else {
                // not really supported
                qWarning("QIcon: Cannot stream QIconEngine. Use QIconEngineV2 instead.");
            }
        }
    } else if (s.version() == QDataStream::Qt_4_2) {
        if (icon.isNull()) {
            s << 0;
        } else {
            QPixmapIconEngine *engine = static_cast<QPixmapIconEngine *>(icon.d->engine);
            int num_entries = engine->pixmaps.size();
            s << num_entries;
            for (int i=0; i < num_entries; ++i) {
                s << engine->pixmaps.at(i).pixmap;
                s << engine->pixmaps.at(i).fileName;
                s << engine->pixmaps.at(i).size;
                s << (uint) engine->pixmaps.at(i).mode;
                s << (uint) engine->pixmaps.at(i).state;
            }
        }
    } else {
        s << QPixmap(icon.pixmap(22,22));
    }
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QIcon &icon)
    \relates QIcon
    \since 4.2

    Reads an image, or a set of images, from the given \a stream into
    the given \a icon.
*/

QDataStream &operator>>(QDataStream &s, QIcon &icon)
{
    if (s.version() >= QDataStream::Qt_4_3) {
        icon = QIcon();
        QString key;
        s >> key;
        if (key == QLatin1String("QPixmapIconEngine")) {
            icon.d = new QIconPrivate;
            QIconEngineV2 *engine = new QPixmapIconEngine;
            icon.d->engine = engine;
            engine->read(s);
        } else if (QIconEngineFactoryInterfaceV2 *factory = qobject_cast<QIconEngineFactoryInterfaceV2*>(loaderV2()->instance(key))) {
            if (QIconEngineV2 *engine= factory->create()) {
                icon.d = new QIconPrivate;
                icon.d->engine = engine;
                engine->read(s);
            }
        }
    } else if (s.version() == QDataStream::Qt_4_2) {
        icon = QIcon();
        int num_entries;
        QPixmap pm;
        QString fileName;
        QSize sz;
        uint mode;
        uint state;

        s >> num_entries;
        for (int i=0; i < num_entries; ++i) {
            s >> pm;
            s >> fileName;
            s >> sz;
            s >> mode;
            s >> state;
            if (pm.isNull())
                icon.addFile(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
            else
                icon.addPixmap(pm, QIcon::Mode(mode), QIcon::State(state));
        }
    } else {
        QPixmap pm;
        s >> pm;
        icon.addPixmap(pm);
    }
    return s;
}

#endif //QT_NO_DATASTREAM


#ifdef QT3_SUPPORT

static int widths[2] = { 22, 32 };
static int heights[2] = { 22, 32 };

static QSize pixmapSize(QIcon::Size which) {
    int i = 0;
    if (which == QIcon::Large)
        i = 1;
    return QSize(widths[i], heights[i]);
}

/*!
    \enum QIcon::Size
    \compat

    \value Small  Use QStyle::pixelMetric(QStyle::PM_SmallIconSize) instead.
    \value Large  Use QStyle::pixelMetric(QStyle::PM_LargeIconSize) instead.
    \value Automatic  N/A.
*/

/*!
    Use pixmap(QSize(...), \a mode, \a state), where the first
    argument is an appropriate QSize instead of a \l Size value.

    \sa pixmapSize()
*/
QPixmap QIcon::pixmap(Size size, Mode mode, State state) const
{ return pixmap(::pixmapSize(size), mode, state); }

/*!
    Use pixmap(QSize(...), mode, \a state), where the first argument
    is an appropriate QSize instead of a \l Size value, and the
    second argument is QIcon::Normal or QIcon::Disabled, depending on
    the value of \a enabled.

    \sa pixmapSize()
*/
QPixmap QIcon::pixmap(Size size, bool enabled, State state) const
{ return pixmap(::pixmapSize(size), enabled ? Normal : Disabled, state); }

/*!
    Use one of the other pixmap() overloads.
*/
QPixmap QIcon::pixmap() const
{ return pixmap(::pixmapSize(Small), Normal, Off); }

/*!
    The pixmap() function now takes a QSize instead of a QIcon::Size,
    so there is no need for this function in new code.
*/
void QIcon::setPixmapSize(Size which, const QSize &size)
{
    int i = 0;
    if (which == Large)
        i = 1;
    widths[i] = size.width();
    heights[i] = size.height();
}

/*!
    Use QStyle::pixelMetric() with QStyle::PM_SmallIconSize or
    QStyle::PM_LargeIconSize as the first argument, depending on \a
    which.
*/
QSize QIcon::pixmapSize(Size which)
{
    return ::pixmapSize(which);
}

/*!
    \fn void QIcon::reset(const QPixmap &pixmap, Size size)

    Use the constructor that takes a QPixmap and operator=().
*/

/*!
    \fn void QIcon::setPixmap(const QPixmap &pixmap, Size size, Mode mode, State state)

    Use addPixmap(\a pixmap, \a mode, \a state) instead. The \a size
    parameter is ignored.
*/

/*!
    \fn void QIcon::setPixmap(const QString &fileName, Size size, Mode mode, State state)

    Use addFile(\a fileName, \a mode, \a state) instead. The \a size
    parameter is ignored.
*/

#endif // QT3_SUPPORT
