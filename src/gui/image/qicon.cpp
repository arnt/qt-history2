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

#ifndef QT_NO_ICON

#include "qapplication.h"
#include "qbitmap.h"
#include "qcleanuphandler.h"
#include "qimage.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qstyle.h"
#include "qstyleoption.h"

enum { NumSizes = 2, NumModes = 3, NumStates = 2 };

static short widths[2] = { 22, 32 };
static short heights[2] = { 22, 32 };

enum QIconVariantOrigin {
    SuppliedFileName,   // 'fileName' contains the name of the file
    SuppliedPixmap,     // 'pixmap' is a pointer to the user-supplied pixmap
    CustomGenerated,    // 'pixmap' is a custom-generated pixmap (or 0)
    Generated           // 'pixmap' is a QIcon-generated pixmap (or 0)
};

struct QIconVariant
{
    QIconVariantOrigin origin;
    union {
        QString *fileName;
        QPixmap *pixmap;
    };

    inline QIconVariant(): origin(Generated) { pixmap = 0; }
    inline QIconVariant(const QIconVariant &other)
        : origin(Generated)
    {
        pixmap = 0;
        operator=(other);
    }
    inline ~QIconVariant()
    {
        if (origin == SuppliedFileName)
            delete fileName;
        else
            delete pixmap;
    }
    QIconVariant &operator=(const QIconVariant &other);

    inline void clearCached()
    {
        if (pixmap && (origin == CustomGenerated || origin == Generated)) {
            origin = Generated;
            delete pixmap;
            pixmap = 0;
        }
    }
};

QIconVariant &QIconVariant::operator=(const QIconVariant &other)
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

class QIconPrivate
{
public:
    QAtomic ref;
    QIconVariant iconVariants[NumSizes][NumModes][NumStates];
    QPixmap defaultPix;
    QIcon::PixmapGeneratorFn func;

    inline QIconPrivate() : func(0) { ref = 1; }
    inline QIconPrivate(const QIconPrivate &other)
        : func(0)
    {
        ref = 1;
        for (int i = 0; i < NumSizes; ++i) {
            for (int j = 0; j < NumModes; ++j) {
                for (int k = 0; k < NumStates; ++k)
                    iconVariants[i][j][k] = other.iconVariants[i][j][k];
            }
        }
        defaultPix = other.defaultPix;
    }
    ~QIconPrivate() {}

    QIconVariant *iconVariant(const QIcon *icon, Qt::IconSize size, QIcon::Mode mode,
                              QIcon::State state);
    QPixmap generatePixmap(const QIcon *icon, Qt::IconSize size, QIcon::Mode mode,
                           QIcon::State state);

    Q_DUMMY_COMPARISON_OPERATOR(QIconPrivate)

    void normalize(Qt::IconSize &which, const QSize &pixSize);
    QPixmap *createScaled(Qt::IconSize size, const QPixmap *suppliedPix);
    QPixmap *createIcon(const QIcon *icon, Qt::IconSize size, QIcon::Mode mode,
                        QIcon::State state);

    static QPixmap *defaultGenerator(const QIcon &icon, Qt::IconSize size, QIcon::Mode mode,
                                     QIcon::State state);
};

QIconVariant *QIconPrivate::iconVariant(const QIcon *icon, Qt::IconSize size, QIcon::Mode mode,
                                        QIcon::State state)
{
    if (size == Qt::AutomaticIconSize)
        size = Qt::SmallIconSize;
    
    QIconVariant *variant = &iconVariants[(int)size - 1][(int)mode][(int)state];

    if (icon) {
        if (variant->origin == SuppliedFileName) {
            QPixmap *newPixmap = new QPixmap(*variant->fileName);
            delete variant->fileName;
            if (newPixmap->isNull()) {
                delete newPixmap;
                variant->origin = Generated;
                variant->pixmap = 0;
            } else {
                variant->origin = SuppliedPixmap;
                variant->pixmap = newPixmap;
            }
        }

        if (!variant->pixmap && variant->origin == Generated) {
            /*
               We set 'origin' to CustomGenerated half a second too
               early to prevent recursive calls to this function.
               (This can happen if the pixmap generator function calls
               QIcon::pixmap(), which in turn calls this
               function.)
             */
            QIcon::PixmapGeneratorFn f = func;
            if (!f)
                f = QIcon::defaultGeneratorFn;
            variant->origin = CustomGenerated;
            variant->pixmap = (*f)(*icon, size, mode, state);
            if (!variant->pixmap)
                variant->origin = Generated;
        }
    }
    return variant;
}

QPixmap QIconPrivate::generatePixmap(const QIcon *icon, Qt::IconSize size, QIcon::Mode mode,
                                     QIcon::State state)
{
    if (size == Qt::AutomaticIconSize)
        size = Qt::SmallIconSize;

    QIconVariant *variant = iconVariant(icon, size, mode, state);
    if (variant->pixmap)
        return *variant->pixmap;
    if (variant->origin == CustomGenerated) {
        /*
          This can only occur during the half a second's time when
          the icon is being manufactured. If the pixmap generator somehow
          tries to access the pixmap it's supposed to be creating, it
          will get a null pixmap.
        */
        return QPixmap();
    }

    Qt::IconSize otherSize = (size == Qt::LargeIconSize) ? Qt::SmallIconSize : Qt::LargeIconSize;
    QIconVariant *otherSizeIcon = iconVariant(icon, otherSize, mode, state);

    if (state == QIcon::Off) {
        bool couldCreate = mode == QIcon::Disabled || mode == QIcon::Active;
        if (couldCreate
                && iconVariant(icon, size, QIcon::Normal, QIcon::Off)->origin != Generated) {
            variant->pixmap = createIcon(icon, size, mode, QIcon::Off);
        } else if (otherSizeIcon->origin != Generated) {
            variant->pixmap = createScaled(size, otherSizeIcon->pixmap);
        } else if (couldCreate) {
            variant->pixmap = createIcon(icon, size, mode, QIcon::Off);
        } else if (!defaultPix.isNull()) {
            variant->pixmap = new QPixmap(defaultPix);
        } else {
            /*
              No icon variants are available for
              { true, Normal, Off } and { false, Normal, Off }.
              Try the other 10 combinaisons, best ones first.
            */
            const int N = 10;
            static const struct {
                bool sameSize;
                QIcon::Mode mode;
                QIcon::State state;
            } tryList[N] = {
                { true, QIcon::Active, QIcon::Off },
                { true, QIcon::Normal, QIcon::On },
                { true, QIcon::Active, QIcon::On },
                { false, QIcon::Active, QIcon::Off },
                { false, QIcon::Normal, QIcon::On },
                { false, QIcon::Active, QIcon::On },
                { true, QIcon::Disabled, QIcon::Off },
                { true, QIcon::Disabled, QIcon::On },
                { false, QIcon::Disabled, QIcon::Off },
                { false, QIcon::Disabled, QIcon::On }
            };

            for (int i = 0; i < N; ++i) {
                bool sameSize = tryList[i].sameSize;
                QIconVariant *tryVariant = iconVariant(icon, sameSize ? size : otherSize,
                                                       tryList[i].mode, tryList[i].state);
                if (tryVariant->origin != Generated) {
                    if (sameSize) {
                        if (tryVariant->pixmap)
                            variant->pixmap = new QPixmap(*tryVariant->pixmap);
                    } else {
                        variant->pixmap = createScaled(size, tryVariant->pixmap);
                    }
                    break;
                }
            }
        }
    } else {
        if (mode == QIcon::Normal) {
            if (otherSizeIcon->origin != Generated)
                variant->pixmap = createScaled(size, otherSizeIcon->pixmap);
            else
                variant->pixmap = new QPixmap(generatePixmap(icon, size, mode, QIcon::Off));
        } else {
            QIconVariant *offIcon = iconVariant(icon, size, mode, QIcon::Off);
            QIconVariant *otherSizeOffIcon = iconVariant(icon, otherSize, mode, QIcon::Off);
            if (offIcon->origin != Generated) {
                if (offIcon->pixmap)
                    variant->pixmap = new QPixmap(*offIcon->pixmap);
            } else if (iconVariant(icon, size, QIcon::Normal, QIcon::On)->origin != Generated) {
                variant->pixmap = createIcon(icon, size, mode, QIcon::On);
            } else if (otherSizeIcon->origin != Generated) {
                variant->pixmap = createScaled(size, otherSizeIcon->pixmap);
            } else if (otherSizeOffIcon->origin != Generated) {
                variant->pixmap = createScaled(size, otherSizeOffIcon->pixmap);
            } else {
                variant->pixmap = createIcon(icon, size, mode, QIcon::On);
            }
        }
    }
    if (variant->pixmap)
        return *variant->pixmap;
    return QPixmap();
}

void QIconPrivate::normalize(Qt::IconSize &which, const QSize &pixSize)
{
    if (which == Qt::AutomaticIconSize)
        which = pixSize.width() > widths[Qt::SmallIconSize - 1] ? Qt::LargeIconSize : Qt::SmallIconSize;
}

/*
    Returns a new pixmap that is a copy of \a suppliedPix, scaled to
    the icon size \a size.
*/
QPixmap *QIconPrivate::createScaled(Qt::IconSize size, const QPixmap *suppliedPix)
{
    if (!suppliedPix || suppliedPix->isNull())
        return 0;

    QImage img = suppliedPix->toImage();
    QSize imgSize(widths[size - 1], heights[size - 1]);
    if (size == Qt::SmallIconSize) {
        imgSize = imgSize.boundedTo(img.size());
    } else {
        imgSize = imgSize.expandedTo(img.size());
    }
    img = img.scale(imgSize);

    QPixmap *pixmap = new QPixmap(img);
    if (!pixmap->mask()) {
        QBitmap mask;
        mask.fromImage(img.createHeuristicMask(), Qt::MonoOnly | Qt::ThresholdDither);
        pixmap->setMask(mask);
    }
    return pixmap;
}

/*
    Returns a new pixmap that has a mode \a mode look, taking as its
    base the pixmap with size \a size and state \a state.
*/
QPixmap *QIconPrivate::createIcon(const QIcon *icon, Qt::IconSize size, QIcon::Mode mode,
                                  QIcon::State state)
{
    Q_ASSERT_X(mode != QIcon::Normal, "QIconPrivate::createIcon", "Mode cannot be \"Normal\"");
    QPixmap normalPix = generatePixmap(icon, size, QIcon::Normal, state);
    if (normalPix.isNull())
        return 0;

    QStyleOption opt(0);
    opt.palette = QApplication::palette();
    QPixmap pix = QApplication::style()->generatedIconPixmap(mode == QIcon::Disabled
                                                            ? QStyle::IM_Disabled
                                                            : QStyle::IM_Active, normalPix, &opt);
    return new QPixmap(pix);
}

QPixmap *QIconPrivate::defaultGenerator(const QIcon &, Qt::IconSize, QIcon::Mode, QIcon::State)
{
    return 0;
}

/*!
  \class QIcon

  \brief The QIcon class provides different versions of an icon.

  \ingroup multimedia
  \ingroup shared
  \mainclass

  A QIcon can generate smaller, larger, active, and disabled pixmaps
  from the set of pixmaps it is given. Such pixmaps are used by Qt
  widgets to show an icon representing a particular action.

  The simplest use of QIcon is to create one from a QPixmap and then
  use it, allowing Qt to work out all the required icon styles and
  sizes. For example:

  \code
    QToolButton *but = new QToolButton(QIcon(QPixmap("open.xpm")), ...);
  \endcode

  Using whichever pixmaps you specify as a base, QIcon provides a set
  of six pixmaps, each with a \l Size and a \l Mode: Small Normal,
  Small Disabled, Small Active, Large Normal, Large Disabled, and
  Large Active.

  An additional set of six pixmaps can be provided for widgets that
  have an "On" or "Off" state, like checkable menu items or checkable
  toolbuttons. If you provide pixmaps for the "On" state, but not for
  the "Off" state, the QIcon will provide the "Off" pixmaps. You may
  specify pixmaps for both states in you wish.

  You can set any of the pixmaps using setPixmap().

  When you retrieve a pixmap using pixmap(Size, Mode, State), QIcon
  will return the pixmap that has been set or previously generated
  for that size, mode and state combination. If none is available,
  QIcon will ask the installed pixmap generator function. If the
  pixmap generator cannot provide any (the default), QIcon generates
  a pixmap based on the pixmaps it has been given and returns it.

  The \c Disabled appearance is computed using the current style. The
  \c Active appearance is identical to the \c Normal appearance unless
  you use setPixmap() to set it to something special.

  When scaling pixmaps, QIcon uses \link QImage::smoothScale() smooth
  scaling\endlink, which can partially blend the color component of
  pixmaps.  If the results look poor, the best solution is to supply
  pixmaps in both large and small sizes.

  You can use the static function setPixmapSize() to set the preferred
  size of the generated large/small pixmaps. The default small size
  is 22 x 22, while the default large size is 32 x 32. These sizes
  only affect generated pixmaps.

  The isGenerated() function returns true if an icon was generated by
  QIcon or by a pixmap generator function; clearGenerated() clears
  all cached pixmaps.

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
        QPixmap pixmap = icons->pixmap(Qt::SmallIconSize,
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

  \sa QPixmap QMainWindow::setUsesBigPixmaps()
      \link guibooks.html#fowler GUI Design Handbook: Iconic Label \endlink
*/

/*!
    \enum QIcon::PixmapGeneratorFn
    \internal
*/

/*!
  \enum Qt::IconSize

  This enum type describes the size at which a pixmap is intended to be
  used.
  The currently defined sizes are:

    \value AutomaticIconSize  The size of the pixmap is determined from its
                      pixel size. This is a useful default.
    \value SmallIconSize  The pixmap is the smaller of two.
    \value LargeIconSize  The pixmap is the larger of two.

  If a Small pixmap is not set by setPixmap(), the Large
  pixmap will be automatically scaled down to the size of a small pixmap
  to generate the Small pixmap when required.  Similarly, a Small pixmap
  will be automatically scaled up to generate a Large pixmap. The
  preferred sizes for large/small generated pixmaps can be set using
  setPixmapSize().

  \sa setPixmapSize(), pixmapSize(), setPixmap(), pixmap(), QMainWindow::setUsesBigPixmaps()
*/

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

  \sa setPixmap() pixmap()
*/

/*! \fn QIcon::QIcon()

  Constructs a null icon.

  \sa setPixmap(), reset()
*/

QIcon::PixmapGeneratorFn QIcon::defaultGeneratorFn = QIconPrivate::defaultGenerator;

QIcon::QIcon()
    : d(0)
{
}

/*!
  Constructs an icon for which the Normal pixmap is \a pixmap,
  which is assumed to be of size \a size.

  The default for \a size is \c Automatic, which means that QIcon
  will determine whether the pixmap is Small or Large from its pixel
  size. Pixmaps less than the width of a small generated icon are
  considered to be Small. You can use setPixmapSize() to set the
  preferred size of a generated icon.

  \sa setPixmapSize() reset()
*/
QIcon::QIcon(const QPixmap &pixmap, Qt::IconSize size)
    : d(0)
{
    reset(pixmap, size);
}

/*!  Creates an icon which uses the pixmap \a smallPix for for
  displaying a small icon, and the pixmap \a largePix for displaying a
  large icon.
*/
QIcon::QIcon(const QPixmap &smallPix, const QPixmap &largePix)
    : d(0)
{
    reset(smallPix, Qt::SmallIconSize);
    reset(largePix, Qt::LargeIconSize);
}

/*!
  Constructs a copy of \a other. This is very fast.
*/
QIcon::QIcon(const QIcon &other)
    : d(other.d)
{
    if (d)
        ++d->ref;
}

/*!
  Destroys the icon and frees any allocated resources.
*/
QIcon::~QIcon()
{
    if (d && !--d->ref)
        delete d;
}

/*!
  Assigns \a other to this icon and returns a reference to this
  icon.

  \sa detach()
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
  Sets this icon to use the given \a pixmap for the \c Normal pixmap,
  assuming it to be of the \a size specified.

  This is equivalent to assigning QIcon(\a pixmap, \a size) to this
  icon.

  This function does nothing if \a pixmap is a null pixmap.
*/
void QIcon::reset(const QPixmap &pixmap, Qt::IconSize size)
{
    if (pixmap.isNull())
        return;

    detach();
    d->normalize(size, pixmap.size());
    setPixmap(pixmap, size, Normal);
    d->defaultPix = pixmap;
}

/*!
    Sets the pixmap for this icon to be the given \a pixmap for requests
    of the same \a size, \a mode, and \a state. The icon may also use
    \a pixmap for generating other pixmaps if they are not explicitly set.

    The \a size can be one of \c Automatic, \c Large or \c Small.
    If \c Automatic is used, QIcon will determine if the pixmap is \c Small
    or \c Large from its pixel size.

    Pixmaps less than the width of a small generated icon are
    considered to be Small. You can use setPixmapSize() to set the preferred
    size of a generated icon.

    This function does nothing if \a pixmap is a null pixmap.

    \sa reset()
*/
void QIcon::setPixmap(const QPixmap &pixmap, Qt::IconSize size, Mode mode, State state)
{
    if (pixmap.isNull())
        return;

    d->normalize(size, pixmap.size());

    detach();
    clearGenerated();

    QIconVariant *variant = d->iconVariant(0, size, mode, state);
    if (variant->origin == SuppliedFileName) {
        delete variant->fileName;
        variant->pixmap = 0;
    }
    variant->origin = SuppliedPixmap;
    if (!variant->pixmap)
        variant->pixmap = new QPixmap(pixmap);
    else
        *variant->pixmap = pixmap;
}

/*!
  \overload

  The pixmap is loaded from \a fileName when it becomes necessary.
*/
void QIcon::setPixmap(const QString &fileName, Qt::IconSize size, Mode mode, State state)
{
    if (size == Qt::AutomaticIconSize) {
        setPixmap(QPixmap(fileName), size, mode, state);
    } else {
        detach();
        clearGenerated();
        QIconVariant *variant = d->iconVariant(0, size, mode, state);
        if (variant->origin == SuppliedFileName) {
            *variant->fileName = fileName;
        } else {
            delete variant->pixmap;
            variant->fileName = new QString(fileName);
            variant->origin = SuppliedFileName;
        }
    }
}

/*!
  Returns a pixmap with the required \a size, \a mode, and \a state,
  generating one if necessary. Generated pixmaps are cached.
*/
QPixmap QIcon::pixmap(Qt::IconSize size, Mode mode, State state) const
{
    if (!d) {
        if (defaultGeneratorFn != QIconPrivate::defaultGenerator) {
            // ### this is very evil
            QIcon *that = const_cast<QIcon *>(this);
            that->detach();
        } else {
            return QPixmap();
        }
    }
    return d->generatePixmap(this, size, mode, state);
}

/*!
    \overload
    \obsolete

    This is the same as pixmap(\a size, \a enabled, \a state).
*/
QPixmap QIcon::pixmap(Qt::IconSize size, bool enabled, State state) const
{
    return pixmap(size, enabled ? Normal : Disabled, state);
}

/*!
  \overload

  Returns the pixmap originally provided to the constructor or to
  reset(). This is the Normal pixmap of unspecified Size.

  \sa reset()
*/
QPixmap QIcon::pixmap() const
{
    if (!d)
        return QPixmap();
    return d->defaultPix;
}

/*!
    Returns true if the pixmap used for the icon with the given combination of
    \a size, \a mode, and \a state is generated from another pixmap; otherwise
    returns false.

    \sa pixmap() setPixmap()
*/
bool QIcon::isGenerated(Qt::IconSize size, Mode mode, State state) const
{
    if (!d)
        return true;
    QIconVariantOrigin origin = d->iconVariant(this, size, mode, state)->origin;
    return origin == CustomGenerated || origin == Generated;
}

/*!
    Clears all cached pixmaps, including those obtained from an
    eventual QIconFactory.
*/
void QIcon::clearGenerated()
{
    if (!d)
        return;

    for (int i = 0; i < NumSizes; ++i) {
        for (int j = 0; j < NumModes; ++j) {
            for (int k = 0; k < NumStates; ++k) {
                d->iconVariants[i][j][k].clearCached();
            }
        }
    }
}

/*!
    Installs \a func as the pixmap factory for this icon. The
    pixmap factory is used to generates pixmaps not set by the user.

    If no pixmap factory is installed, defaultPixmapGeneratorFn() is
    used.

    \sa defaultPixmapGeneratorFn()
*/
void QIcon::setPixmapGeneratorFn(PixmapGeneratorFn func)
{
    detach();
    d->func = func;
}

/*!
    \fn PixmapGeneratorFn QIcon::defaultPixmapGeneratorFn()

    Returns the default icon factory.

    \sa setPixmapGeneratorFn()
*/

/*!
    Returns true if the icon is empty; otherwise returns false.
*/
bool QIcon::isNull() const
{
    return !d;
}

/*!
    \internal

    Detaches this icon from others with which it may share data.

    You will never need to call this function; other QIcon functions
    call it as necessary.
*/
void QIcon::detach()
{
    if (!d) {
        d = new QIconPrivate;
        return;
    }
    qAtomicDetach(d);
}

/*!
  \internal
*/
bool QIcon::isDetached() const
{
    return !d || d->ref == 1;
}

/*!
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
void QIcon::setPixmapSize(Qt::IconSize which, const QSize &size)
{
    widths[(int)which - 1] = size.width();
    heights[(int)which - 1] = size.height();
}

/*!
    If \a which is Small, returns the preferred size of a small
    generated icon; if \a which is Large, returns the preferred size
    of a large generated icon.

  \sa setPixmapSize()
*/
QSize QIcon::pixmapSize(Qt::IconSize which)
{
    return QSize(widths[(int)which - 1], heights[(int)which - 1]);
}

/*!
    Installs \a func as the application's default icon factory. The
    icon factory is used to generates pixmaps not set by the user.

    If no icon factory is installed, QIconFactory::defaultFactory()
    is used. Individual QIcon instances can have their own icon
    factories; see setPixmapGeneratorFn().
*/
void QIcon::setDefaultPixmapGeneratorFn(PixmapGeneratorFn func)
{
    defaultGeneratorFn = func;
}

#endif // QT_NO_ICON
