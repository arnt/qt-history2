/****************************************************************************
**
** Implementation of QIconSet class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qiconset.h"

#ifndef QT_NO_ICONSET

#include "qapplication.h"
#include "qstyle.h"
#include "qbitmap.h"
#include "qcleanuphandler.h"
#include "qimage.h"
#include "qpainter.h"
#include "qpalette.h"

enum { NumSizes = 2, NumModes = 3, NumStates = 2 };

static short widths[2] = { 22, 32 };
static short heights[2] = { 22, 32 };

enum QIconSetIconOrigin {
    SuppliedFileName,   // 'fileName' contains the name of the file
    SuppliedPixmap,     // 'pixmap' is a pointer to the user-supplied pixmap
    CustomGenerated,    // 'pixmap' is a custom-generated pixmap (or 0)
    Generated           // 'pixmap' is a QIconSet-generated pixmap (or 0)
};

QIconSet::PixmapGeneratorFn QIconSet::defaultGeneratorFn = QIconSet::defaultGenerator;

struct QIconSetIcon
{
    QIconSetIconOrigin origin;
    union {
        QString *fileName;
        QPixmap *pixmap;
    };

    inline QIconSetIcon(): origin(Generated) { pixmap = 0; }
    inline QIconSetIcon(const QIconSetIcon &other)
        : origin(Generated)
    {
        pixmap = 0;
        operator=(other);
    }
    ~QIconSetIcon()
    {
        if (origin == SuppliedFileName)
            delete fileName;
        else
            delete pixmap;
    }
    QIconSetIcon &operator=(const QIconSetIcon &other);

    inline void clearCached()
    {
        if (pixmap && (origin == CustomGenerated || origin == Generated)) {
            origin = Generated;
            delete pixmap;
            pixmap = 0;
        }
    }
};

QIconSetIcon &QIconSetIcon::operator=(const QIconSetIcon &other)
{
    QPixmap *oldPixmap = 0;
    QString *oldFileName = 0;
    if (origin == SuppliedFileName)
        oldFileName = fileName;
    else
        oldPixmap = pixmap;

    origin = other.origin;
    if (other.origin == SuppliedFileName) {
        fileName = new QString(*other.fileName);
    } else {
        if (other.pixmap)
            pixmap = new QPixmap(*other.pixmap);
        else
            pixmap = 0;
    }
    delete oldPixmap;
    delete oldFileName;
    return *this;
}

class QIconSetData
{
public:
    QAtomic ref;
    QIconSetIcon icons[NumSizes][NumModes][NumStates];
    QPixmap defaultPix;
    QIconSet::PixmapGeneratorFn func;

    inline QIconSetData() : func(0) { ref = 1; }
    inline QIconSetData(const QIconSetData& other)
        : func(0)
    {
        ref = 1;
        for (int i = 0; i < NumSizes; ++i) {
            for (int j = 0; j < NumModes; ++j) {
                for (int k = 0; k < NumStates; ++k)
                    icons[i][j][k] = other.icons[i][j][k];
            }
        }
        defaultPix = other.defaultPix;
    }
    ~QIconSetData() {}

    QIconSetIcon *icon(const QIconSet *iconSet, QIconSet::Size size, QIconSet::Mode mode,
                       QIconSet::State state);
    Q_DUMMY_COMPARISON_OPERATOR(QIconSetData)
};

QIconSetIcon *QIconSetData::icon(const QIconSet *iconSet, QIconSet::Size size,
                                    QIconSet::Mode mode, QIconSet::State state)
{
    QIconSetIcon *ik = &icons[(int) size - 1][(int) mode][(int) state];

    if (iconSet) {
        if (ik->origin == SuppliedFileName) {
            QPixmap *newPixmap = new QPixmap(*ik->fileName);
            delete ik->fileName;
            if (newPixmap->isNull()) {
                delete newPixmap;
                ik->origin = Generated;
                ik->pixmap = 0;
            } else {
                ik->origin = SuppliedPixmap;
                ik->pixmap = newPixmap;
            }
        }

        if (!ik->pixmap && ik->origin == Generated) {
            /*
               We set 'origin' to CustomGenerated half a second too
               early to prevent recursive calls to this function.
               (This can happen if the pixmap generator function calls
               QIconSet::pixmap(), which in turn calls this
               function.)
             */
            QIconSet::PixmapGeneratorFn f = func;
            if (!f)
                f = QIconSet::defaultGeneratorFn;
            ik->origin = CustomGenerated;
            ik->pixmap = (*f)(*iconSet, size, mode, state);
            if (!ik->pixmap)
                ik->origin = Generated;
        }
    }
    return ik;
}

/*! \class QIconSet

  \brief The QIconSet class provides a set of icons with different
  styles and sizes.

  \ingroup multimedia
  \ingroup shared
  \mainclass

  A QIconSet can generate smaller, larger, active, and disabled pixmaps
  from the set of icons it is given. Such pixmaps are used by
  QToolButton, QHeader, QPopupMenu, etc. to show an icon representing a
  particular action.

  The simplest use of QIconSet is to create one from a QPixmap and then
  use it, allowing Qt to work out all the required icon styles and
  sizes. For example:

  \code
    QToolButton *but = new QToolButton(QIconSet(QPixmap("open.xpm")), ...);
  \endcode

  Using whichever pixmaps you specify as a base, QIconSet provides a
  set of six icons, each with a \l Size and a \l Mode: Small Normal,
  Small Disabled, Small Active, Large Normal, Large Disabled, and
  Large Active.

  An additional set of six icons can be provided for widgets that have
  an "On" or "Off" state, like checkable menu items or toggleable
  toolbuttons. If you provide pixmaps for the "On" state, but not for
  the "Off" state, the QIconSet will provide the "Off" pixmaps. You may
  specify icons for both states in you wish.

  You can set any of the icons using setPixmap().

  When you retrieve a pixmap using pixmap(Size, Mode, State),
  QIconSet will return the icon that has been set or previously
  generated for that size, mode and state combination. If none is
  available, QIconSet will ask the icon factory. If the icon factory
  cannot provide any (the default), QIconSet generates a pixmap based
  on the pixmaps it has been given and returns it.

  The \c Disabled appearance is computed using the current style. The
  \c Active appearance is identical to the \c Normal appearance unless
  you use setPixmap() to set it to something special.

  When scaling icons, QIconSet uses \link QImage::smoothScale()
  smooth scaling\endlink, which can partially blend the color component
  of pixmaps.  If the results look poor, the best solution
  is to supply pixmaps in both large and small sizes.

  You can use the static function setIconSize() to set the preferred
  size of the generated large/small icons. The default small size is
  22 x 22, while the default large size is 32 x 32. These sizes only
  affect generated icons.

  The isGenerated() function returns true if an icon was generated by
  QIconSet or by a factory; clearGenerated() clears all cached
  pixmaps.

  \section1 Making Classes that Use QIconSet

  If you write your own widgets that have an option to set a small
  pixmap, consider allowing a QIconSet to be set for that pixmap.  The
  Qt class QToolButton is an example of such a widget.

  Provide a method to set a QIconSet, and when you draw the icon, choose
  whichever icon is appropriate for the current state of your widget.
  For example:
  \code
    void MyWidget::drawIcon(QPainter* p, QPoint pos)
    {
        p->drawPixmap(pos, icons->pixmap(
                                QIconSet::Small,
                                isEnabled() ? QIconSet::Normal :
                                              QIconSet::Disabled,
                                isEnabled() ? QIconSet::On :
                                              QIconSet::Off));
    }
  \endcode

  You might also make use of the \c Active mode, perhaps making your
  widget \c Active when the mouse is over the widget (see \l
  QWidget::enterEvent()), while the mouse is pressed pending the
  release that will activate the function, or when it is the currently
  selected item. If the widget can be toggled, the "On" mode might be
  used to draw a different icon.

  \img iconset.png QIconSet

  \sa QPixmap QMainWindow::setUsesBigPixmaps()
      \link guibooks.html#fowler GUI Design Handbook: Iconic Label \endlink
*/

/*!
    \enum QIconSet::PixmapGeneratorFn
    \internal
*/

/*!
  \enum QIconSet::Size

  This enum type describes the size at which a pixmap is intended to be
  used.
  The currently defined sizes are:

    \value Automatic  The size of the pixmap is determined from its
                      pixel size. This is a useful default.
    \value Small  The pixmap is the smaller of two.
    \value Large  The pixmap is the larger of two.

  If a Small pixmap is not set by QIconSet::setPixmap(), the Large
  pixmap will be automatically scaled down to the size of a small pixmap
  to generate the Small pixmap when required.  Similarly, a Small pixmap
  will be automatically scaled up to generate a Large pixmap. The
  preferred sizes for large/small generated icons can be set using
  setIconSize().

  \sa setIconSize() iconSize() setPixmap() pixmap() QMainWindow::setUsesBigPixmaps()
*/

/*!
  \enum QIconSet::Mode

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
  \enum QIconSet::State

  This enum describes the state for which a pixmap is intended to be
  used. The \e state can be:

  \value Off  Display the pixmap when the widget is in an "off" state
  \value On  Display the pixmap when the widget is in an "on" state

  \sa setPixmap() pixmap()
*/

/*! \fn QIconSet::QIconSet()

  Constructs a null icon set.

  \sa setPixmap(), reset()
*/

/*!
  Constructs an icon set for which the Normal pixmap is \a pixmap,
  which is assumed to be of size \a size.

  The default for \a size is \c Automatic, which means that QIconSet
  will determine whether the pixmap is Small or Large from its pixel
  size. Pixmaps less than the width of a small generated icon are
  considered to be Small. You can use setIconSize() to set the
  preferred size of a generated icon.

  \sa setIconSize() reset()
*/
QIconSet::QIconSet(const QPixmap &pixmap, Size size)
    : d(0)
{
    reset(pixmap, size);
}

/*!  Creates an iconset which uses the pixmap \a smallPix for for
  displaying a small icon, and the pixmap \a largePix for displaying a
  large icon.
*/
QIconSet::QIconSet(const QPixmap &smallPix, const QPixmap &largePix)
    : d(0)
{
    reset(smallPix, Small);
    reset(largePix, Large);
}

/*!
  Constructs a copy of \a other. This is very fast.
*/
QIconSet::QIconSet(const QIconSet &other)
    : d(other.d)
{
    if (d)
        ++d->ref;
}

/*!
  Destroys the icon set and frees any allocated resources.
*/
QIconSet::~QIconSet()
{
    if (d && !--d->ref)
        delete d;
}

/*!
  Sets this icon set to use pixmap \a pixmap for the Normal pixmap,
  assuming it to be of size \a size.

  This is equivalent to assigning QIconSet(\a pixmap, \a size) to this
  icon set.

  This function does nothing if \a pixmap is a null pixmap.
*/
void QIconSet::reset(const QPixmap &pixmap, Size size)
{
    if (pixmap.isNull())
        return;

    detach();
    normalize(size, pixmap.size());
    setPixmap(pixmap, size, Normal);
    d->defaultPix = pixmap;
}

/*!
  Sets this icon set to provide pixmap \a pixmap for size \a size, mode \a
  mode and state \a state. The icon set may also use \a pixmap for
  generating other pixmaps if they are not explicitly set.

  The \a size can be one of Automatic, Large or Small.  If Automatic is
  used, QIconSet will determine if the pixmap is Small or Large from its
  pixel size.

  Pixmaps less than the width of a small generated icon are
  considered to be Small. You can use setIconSize() to set the preferred
  size of a generated icon.

  This function does nothing if \a pixmap is a null pixmap.

  \sa reset()
*/
void QIconSet::setPixmap(const QPixmap &pixmap, Size size, Mode mode, State state)
{
    if (pixmap.isNull())
        return;

    normalize(size, pixmap.size());

    detach();
    clearGenerated();

    QIconSetIcon *icon = d->icon(0, size, mode, state);
    if (icon->origin == SuppliedFileName) {
        delete icon->fileName;
        icon->pixmap = 0;
    }
    icon->origin = SuppliedPixmap;
    if (!icon->pixmap)
        icon->pixmap = new QPixmap(pixmap);
    else
        *icon->pixmap = pixmap;
}

/*!
  \overload

  The pixmap is loaded from \a fileName when it becomes necessary.
*/
void QIconSet::setPixmap(const QString &fileName, Size size, Mode mode, State state)
{
    if (size == Automatic) {
        setPixmap(QPixmap(fileName), size, mode, state);
    } else {
        detach();
        clearGenerated();
        QIconSetIcon *icon = d->icon(0, size, mode, state);
        if (icon->origin == SuppliedFileName) {
            *icon->fileName = fileName;
        } else {
            delete icon->pixmap;
            icon->fileName = new QString(fileName);
            icon->origin = SuppliedFileName;
        }
    }
}

/*!
  Returns a pixmap with size \a size, mode \a mode and state \a
  state, generating one if necessary. Generated pixmaps are cached.
*/
QPixmap QIconSet::pixmap(Size size, Mode mode, State state) const
{
    if (!d) {
        if (defaultGeneratorFn != defaultGenerator) {
            QIconSet *that = const_cast<QIconSet *>(this);
            that->detach();
        } else {
            return QPixmap();
        }
    }

    if (size == Automatic)
        size = Small;

    QIconSetIcon *icon = d->icon(this, size, mode, state);
    if (icon->pixmap)
        return *icon->pixmap;
    if (icon->origin == CustomGenerated) {
        /*
          This can only occur during the half a second's time when
          the icon is being manufactured. If the pixmap generator somehow
          tries to access the pixmap it's supposed to be creating, it
          will get a null pixmap.
        */
        return QPixmap();
    }

    Size otherSize = (size == Large) ? Small : Large;
    QIconSetIcon *otherSizeIcon = d->icon(this, otherSize, mode, state);

    if (state == Off) {
        bool couldCreate = mode == Disabled || mode == Active;
        if (couldCreate && d->icon(this, size, Normal, Off)->origin != Generated) {
            icon->pixmap = createIcon(size, mode, Off);
        } else if (otherSizeIcon->origin != Generated) {
            icon->pixmap = createScaled(size, otherSizeIcon->pixmap);
        } else if (couldCreate) {
            icon->pixmap = createIcon(size, mode, Off);
        } else if (!d->defaultPix.isNull()) {
            icon->pixmap = new QPixmap(d->defaultPix);
        } else {
            /*
              No icons are available for { true, Normal, Off } and
              { false, Normal, Off }. Try the other 10 combinaisons,
              best ones first.
            */
            const int N = 10;
            static const struct {
                bool sameSize;
                Mode mode;
                State state;
            } tryList[N] = {
                {true, Active, Off},
                {true, Normal, On},
                {true, Active, On},
                {false, Active, Off},
                {false, Normal, On},
                {false, Active, On},
                {true, Disabled, Off},
                {true, Disabled, On},
                {false, Disabled, Off},
                {false, Disabled, On}
            };

            for (int i = 0; i < N; ++i) {
                bool sameSize = tryList[i].sameSize;
                QIconSetIcon *tryIcon = d->icon(this, sameSize ? size : otherSize,
                                                tryList[i].mode, tryList[i].state);
                if (tryIcon->origin != Generated) {
                    if (sameSize) {
                        if (tryIcon->pixmap)
                            icon->pixmap = new QPixmap(*tryIcon->pixmap);
                    } else {
                        icon->pixmap = createScaled(size, tryIcon->pixmap);
                    }
                    break;
                }
            }
        }
    } else {
        if (mode == Normal) {
            if (otherSizeIcon->origin != Generated)
                icon->pixmap = createScaled(size, otherSizeIcon->pixmap);
            else
                icon->pixmap = new QPixmap(pixmap(size, mode, Off));
        } else {
            QIconSetIcon *offIcon = d->icon(this, size, mode, Off);
            QIconSetIcon *otherSizeOffIcon = d->icon(this, otherSize, mode, Off);
            if (offIcon->origin != Generated) {
                if (offIcon->pixmap)
                    icon->pixmap = new QPixmap(*offIcon->pixmap);
            } else if (d->icon(this, size, Normal, On)->origin != Generated) {
                icon->pixmap = createIcon(size, mode, On);
            } else if (otherSizeIcon->origin != Generated) {
                icon->pixmap = createScaled(size, otherSizeIcon->pixmap);
            } else if (otherSizeOffIcon->origin != Generated) {
                icon->pixmap = createScaled(size, otherSizeOffIcon->pixmap);
            } else {
                icon->pixmap = createIcon(size, mode, On);
            }
        }
    }
    if (icon->pixmap)
        return *icon->pixmap;
    return QPixmap();
}

/*! \overload
    \obsolete

  This is the same as pixmap(\a size, \a enabled, \a state).
*/
QPixmap QIconSet::pixmap(Size size, bool enabled, State state) const
{
    return pixmap(size, enabled ? Normal : Disabled, state);
}

/*!
  \overload

  Returns the pixmap originally provided to the constructor or to
  reset(). This is the Normal pixmap of unspecified Size.

  \sa reset()
*/
QPixmap QIconSet::pixmap() const
{
    if (!d)
        return QPixmap();
    return d->defaultPix;
}

/*!
  Returns true if the pixmap with size \a size, mode \a mode and
  state \a state is generated from other pixmaps; otherwise returns
  false.

  A pixmap obtained from a QIconFactory is considered non-generated.
*/
bool QIconSet::isGenerated(Size size, Mode mode, State state) const
{
    if (!d)
        return true;
    return d->icon(this, size, mode, state)->origin == Generated;
};

/*!
    Clears all cached pixmaps, including those obtained from an
    eventual QIconFactory.
*/
void QIconSet::clearGenerated()
{
    if (!d)
        return;

    for (int i = 0; i < NumSizes; ++i) {
        for (int j = 0; j < NumModes; ++j) {
            for (int k = 0; k < NumStates; ++k) {
                d->icons[i][j][k].clearCached();
            }
        }
    }
}

/*!
    Installs \a func as the icon factory for this iconset. The
    icon factory is used to generates pixmaps not set by the user.

    If no icon factory is installed, QIconFactory::defaultFactory()
    is used.

    \sa defaultPixmapGeneratorFn()
*/
void QIconSet::setPixmapGeneratorFn(PixmapGeneratorFn func)
{
    detach();
    d->func = func;
}

/*!
    \fn PixmapGeneratorFn QIconSet::defaultPixmapGeneratorFn()

    Returns the default icon factory.

    \sa setPixmapGeneratorFn()
*/

/*!
    \fn bool QIconSet::isNull() const
    Returns true if the icon set is empty; otherwise returns false.
*/

/*!
  Detaches this icon set from others with which it may share data.

  You will never need to call this function; other QIconSet functions
  call it as necessary.
*/
void QIconSet::detach()
{
    if (!d) {
        d = new QIconSetData;
        return;
    }
    qAtomicDetach(d);
}

/*!
  Assigns \a other to this icon set and returns a reference to this
  icon set.

  \sa detach()
*/
QIconSet& QIconSet::operator=(const QIconSet &other)
{
    QIconSetData *x = other.d;
    if (x)
        ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (x && !--x->ref)
        delete x;
    return *this;
}

/*!
  Set the preferred size for all small or large icons that are
  generated after this call. If \a which is Small, sets the preferred
  size of small generated icons to \a size. Similarly, if \a which is
  Large, sets the preferred size of large generated icons to \a size.

  Note that cached icons will not be regenerated, so it is recommended
  that you set the preferred icon sizes before generating any icon sets.
  Also note that the preferred icon sizes will be ignored for icon sets
  that have been created using both small and large pixmaps.

  \sa iconSize()
*/
void QIconSet::setIconSize(Size which, const QSize &size)
{
    widths[(int)which - 1] = size.width();
    heights[(int)which - 1] = size.height();
}

/*!
    If \a which is Small, returns the preferred size of a small
    generated icon; if \a which is Large, returns the preferred size
    of a large generated icon.

  \sa setIconSize()
*/
QSize QIconSet::iconSize(Size which)
{
    return QSize(widths[(int)which - 1], heights[(int)which - 1]);
}

void QIconSet::normalize(Size &which, const QSize &pixSize)
{
    if (which == Automatic)
        which = pixSize.width() > iconSize(Small).width() ? Large : Small;
}

/*!
    Installs \a func as the application's default icon factory. The
    icon factory is used to generates pixmaps not set by the user.

    If no icon factory is installed, QIconFactory::defaultFactory()
    is used. Individual QIconSet instances can have their own icon
    factories; see setPixmapGeneratorFn().
*/
void QIconSet::setDefaultPixmapGeneratorFn(PixmapGeneratorFn func)
{
    defaultGeneratorFn = func;
}

QPixmap *QIconSet::defaultGenerator(const QIconSet &, Size, Mode, State)
{
    return 0;
}

/*!
    Returns a new pixmap that is a copy of \a suppliedPix, scaled to
    the icon size \a size.
*/
QPixmap *QIconSet::createScaled(Size size, const QPixmap *suppliedPix) const
{
    if (!suppliedPix || suppliedPix->isNull())
        return 0;

    QImage img = suppliedPix->convertToImage();
    QSize imgSize = iconSize(size);
    if (size == Small) {
        imgSize = imgSize.boundedTo(img.size());
    } else {
        imgSize = imgSize.expandedTo(img.size());
    }
    img = img.smoothScale(imgSize);

    QPixmap *pixmap = new QPixmap(img);
    if (!pixmap->mask()) {
        QBitmap mask;
        mask.convertFromImage(img.createHeuristicMask(), Qt::MonoOnly | Qt::ThresholdDither);
        pixmap->setMask(mask);
    }
    return pixmap;
}

/*!
    Returns a new pixmap that has a mode \a mode look, taking as its
    base the iconset's icon with size \a size and state \a state.
*/
QPixmap *QIconSet::createIcon(Size size, Mode mode, State state) const
{
    Q_ASSERT_X(mode != Normal, "QIconSet::createIcon", "Mode cannot be \"Normal\"");
    QPixmap normalPix = pixmap(size, Normal, state);
    if (normalPix.isNull())
        return 0;

    QPixmap pix = QApplication::style().stylePixmap(mode == Disabled
                                                    ? QStyle::PT_Disabled 
                                                    : QStyle::PT_Active, normalPix,
                                                    QApplication::palette());
    return new QPixmap(pix);
}

#endif // QT_NO_ICONSET
