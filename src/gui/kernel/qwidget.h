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

#ifndef QWIDGET_H
#define QWIDGET_H

#include "qwindowdefs.h"
#include "qobject.h"
#include "qpaintdevice.h"
#include "qpalette.h"
#include "qfont.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qsizepolicy.h"
#include "qregion.h"
#include "qbrush.h"
#include "qcursor.h"
#include "qkeysequence.h"

#ifdef QT_INCLUDE_COMPAT
#include "qevent.h"
#endif

class QLayout;
class QWSRegionManager;
class QStyle;
class QAction;
class QVariant;

class QActionEvent;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QFocusEvent;
class QPaintEvent;
class QMoveEvent;
class QResizeEvent;
class QCloseEvent;
class QContextMenuEvent;
class QInputMethodEvent;
class QTabletEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QShowEvent;
class QHideEvent;
class QInputContext;
#if defined(Q_WS_X11)
class QX11Info;
#endif

class QWidgetData
{
public:
    WId winid;
    uint widget_state; // will go away, eventually
    uint widget_attributes;
    uint widget_flags;
    uint focus_policy : 4;
    uint sizehint_forced :1;
    uint is_closing :1;
    uint in_show : 1;
    uint in_set_window_state : 1;
    uint fstrut_dirty : 1;
    uint im_enabled : 1;
    QRect crect;
#ifndef QT_NO_PALETTE
    mutable QPalette pal;
#endif
    QFont fnt;
#if defined(Q_WS_QWS)
    QRegion req_region;                 // Requested region
    mutable QRegion paintable_region;   // Paintable region
    mutable bool paintable_region_dirty;// needs to be recalculated
    mutable QRegion alloc_region;       // Allocated region
    mutable bool alloc_region_dirty;    // needs to be recalculated
    mutable int overlapping_children;   // Handle overlapping children

    int alloc_region_index;
    int alloc_region_revision;
#endif
    QRect wrect;
};

class QWidgetPrivate;

class Q_GUI_EXPORT QWidget : public QObject, public QPaintDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWidget)

    Q_PROPERTY(bool modal READ isModal)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)
    Q_PROPERTY(QRect frameGeometry READ frameGeometry)
    Q_PROPERTY(QRect normalGeometry READ normalGeometry)
    Q_PROPERTY(int x READ x)
    Q_PROPERTY(int y READ y)
    Q_PROPERTY(QPoint pos READ pos WRITE move DESIGNABLE false STORED false)
    Q_PROPERTY(QSize frameSize READ frameSize)
    Q_PROPERTY(QSize size READ size WRITE resize DESIGNABLE false STORED false)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(QRect rect READ rect)
    Q_PROPERTY(QRect childrenRect READ childrenRect)
    Q_PROPERTY(QRegion childrenRegion READ childrenRegion)
    Q_PROPERTY(QSizePolicy sizePolicy READ sizePolicy WRITE setSizePolicy)
    Q_PROPERTY(QSize minimumSize READ minimumSize WRITE setMinimumSize)
    Q_PROPERTY(QSize maximumSize READ maximumSize WRITE setMaximumSize)
    Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth STORED false DESIGNABLE false)
    Q_PROPERTY(int minimumHeight READ minimumHeight WRITE setMinimumHeight STORED false DESIGNABLE false)
    Q_PROPERTY(int maximumWidth READ maximumWidth WRITE setMaximumWidth STORED false DESIGNABLE false)
    Q_PROPERTY(int maximumHeight READ maximumHeight WRITE setMaximumHeight STORED false DESIGNABLE false)
    Q_PROPERTY(QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement)
    Q_PROPERTY(QSize baseSize READ baseSize WRITE setBaseSize)
#ifndef QT_NO_PALETTE
    Q_PROPERTY(QPalette palette READ palette WRITE setPalette)
#endif
    Q_PROPERTY(QFont font READ font WRITE setFont)
#ifndef QT_NO_CURSOR
    Q_PROPERTY(QCursor cursor READ cursor WRITE setCursor RESET unsetCursor)
#endif
    Q_PROPERTY(bool mouseTracking READ hasMouseTracking WRITE setMouseTracking)
    Q_PROPERTY(bool isActiveWindow READ isActiveWindow)
    Q_PROPERTY(bool focusEnabled READ isFocusEnabled)
    Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy)
    Q_PROPERTY(bool focus READ hasFocus)
    Q_PROPERTY(bool updatesEnabled READ isUpdatesEnabled WRITE setUpdatesEnabled DESIGNABLE false)
    Q_PROPERTY(bool visible READ isVisible)
    Q_PROPERTY(bool hidden READ isHidden WRITE setHidden DESIGNABLE false SCRIPTABLE false)
    Q_PROPERTY(bool shown READ isShown WRITE setShown DESIGNABLE false SCRIPTABLE false)
    Q_PROPERTY(bool minimized READ isMinimized)
    Q_PROPERTY(bool maximized READ isMaximized)
    Q_PROPERTY(bool fullScreen READ isFullScreen)
    Q_PROPERTY(QSize sizeHint READ sizeHint)
    Q_PROPERTY(QSize minimumSizeHint READ minimumSizeHint)
    Q_PROPERTY(bool acceptDrops READ acceptDrops WRITE setAcceptDrops)
    Q_PROPERTY(bool autoMask READ autoMask WRITE setAutoMask DESIGNABLE false SCRIPTABLE false)
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle DESIGNABLE isTopLevel)
    Q_PROPERTY(QPixmap windowIcon READ windowIcon WRITE setWindowIcon DESIGNABLE isTopLevel)
    Q_PROPERTY(QString windowIconText READ windowIconText WRITE setWindowIconText DESIGNABLE isTopLevel)
    Q_PROPERTY(double windowOpacity READ windowOpacity WRITE setWindowOpacity DESIGNABLE false)
    Q_PROPERTY(bool windowModified READ isWindowModified WRITE setWindowModified DESIGNABLE isTopLevel)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
#ifndef QT_NO_ACCESSIBILITY
    Q_PROPERTY(QString accessibleName READ accessibleName WRITE setAccessibleName)
    Q_PROPERTY(QString accessibleDescription READ accessibleDescription WRITE setAccessibleDescription)
#endif
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection WRITE setLayoutDirection RESET unsetLayoutDirection)


public:
    QWidget(QWidget* parent = 0, Qt::WFlags f = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QWidget(QWidget* parent, const char *name, Qt::WFlags f = 0);
#endif
    ~QWidget();

    WId winId() const;
#ifndef QT_NO_STYLE
    // GUI style setting

    QStyle *style() const;
    void setStyle(QStyle *);
    QStyle *setStyle(const QString&);
#endif
    // Widget types and states

    bool isTopLevel() const;
    bool isDialog() const;
    bool isPopup() const;
    bool isDesktop() const;
    bool isModal() const;

    bool isEnabled() const;
    bool isEnabledTo(QWidget*) const;
    bool isEnabledToTLW() const;

public slots:
    void setEnabled(bool);
    void setDisabled(bool);

    // Widget coordinates

public:
    QRect frameGeometry() const;
    const QRect &geometry() const;
    QRect normalGeometry() const;

    int x() const;
    int y() const;
    QPoint pos() const;
    QSize frameSize() const;
    QSize size() const;
    inline int width() const;
    inline int height() const;
    inline QRect rect() const;
    QRect childrenRect() const;
    QRegion childrenRegion() const;

    QSize minimumSize() const;
    QSize maximumSize() const;
    int minimumWidth() const;
    int minimumHeight() const;
    int maximumWidth() const;
    int maximumHeight() const;
    void setMinimumSize(const QSize &);
    void setMinimumSize(int minw, int minh);
    void setMaximumSize(const QSize &);
    void setMaximumSize(int maxw, int maxh);
    void setMinimumWidth(int minw);
    void setMinimumHeight(int minh);
    void setMaximumWidth(int maxw);
    void setMaximumHeight(int maxh);

    QSize sizeIncrement() const;
    void setSizeIncrement(const QSize &);
    void setSizeIncrement(int w, int h);
    QSize baseSize() const;
    void setBaseSize(const QSize &);
    void setBaseSize(int basew, int baseh);

    void setFixedSize(const QSize &);
    void setFixedSize(int w, int h);
    void setFixedWidth(int w);
    void setFixedHeight(int h);

    // Widget coordinate mapping

    QPoint mapToGlobal(const QPoint &) const;
    QPoint mapFromGlobal(const QPoint &) const;
    QPoint mapToParent(const QPoint &) const;
    QPoint mapFromParent(const QPoint &) const;
    QPoint mapTo(QWidget *, const QPoint &) const;
    QPoint mapFrom(QWidget *, const QPoint &) const;

    QWidget *topLevelWidget() const;

#ifndef QT_NO_PALETTE
    // Widget appearance functions
    const QPalette &palette() const;
    void setPalette(const QPalette &);

    void setBackgroundRole(QPalette::ColorRole);
    QPalette::ColorRole backgroundRole() const;

    void setForegroundRole(QPalette::ColorRole);
    QPalette::ColorRole foregroundRole() const;
#endif

    const QFont &font() const;
    void setFont(const QFont &);
    QFontMetrics fontMetrics() const;
    QFontInfo fontInfo() const;

#ifndef QT_NO_CURSOR
    QCursor cursor() const;
    void setCursor(const QCursor &);
    void unsetCursor();
#endif

    void setMouseTracking(bool enable);
    bool hasMouseTracking() const;
    bool underMouse() const;

    void setMask(const QBitmap &);
    void setMask(const QRegion &);
    QRegion mask() const;
    void clearMask();

    void setWindowTitle(const QString &);
    QString windowTitle() const;
    void setWindowIcon(const QPixmap &);
    const QPixmap &windowIcon() const;
    void setWindowIconText(const QString &);
    QString windowIconText() const;
    void setWindowRole(const QString &);
    QString windowRole() const;

    void setWindowOpacity(double level);
    double windowOpacity() const;

    bool isWindowModified() const;
    void setWindowModified(bool);

    void setToolTip(const QString &);
    QString toolTip() const;

    void setStatusTip(const QString &);
    QString statusTip() const;

    void setWhatsThis(const QString &);
    QString whatsThis() const;

#ifndef QT_NO_ACCESSIBILITY
    QString accessibleName() const;
    void setAccessibleName(const QString &name);
    QString accessibleDescription() const;
    void setAccessibleDescription(const QString &description);
#endif

    void setLayoutDirection(Qt::LayoutDirection direction);
    Qt::LayoutDirection layoutDirection() const;
    void unsetLayoutDirection();

    inline bool isRightToLeft() const { return layoutDirection() == Qt::RightToLeft; }
    inline bool isLeftToRight() const { return layoutDirection() == Qt::LeftToRight; }

public slots:
    void setFocus();

public:
    bool isActiveWindow() const;
    void setActiveWindow();
    bool isFocusEnabled() const;
    void clearFocus();

    Qt::FocusPolicy focusPolicy() const;
    void setFocusPolicy(Qt::FocusPolicy);
    bool hasFocus() const;
    static void setTabOrder(QWidget *, QWidget *);
    void setFocusProxy(QWidget *);
    QWidget *focusProxy() const;

    // Grab functions
    void grabMouse();
#ifndef QT_NO_CURSOR
    void grabMouse(const QCursor &);
#endif
    void releaseMouse();
    void grabKeyboard();
    void releaseKeyboard();
    int grabShortcut(const QKeySequence &key, Qt::ShortcutContext context = Qt::ShortcutOnActiveWindow);
    void releaseShortcut(int id);
    void setShortcutEnabled(int id, bool enable = true);
    static QWidget *mouseGrabber();
    static QWidget *keyboardGrabber();

    // Update/refresh functions
    bool isUpdatesEnabled() const;

#if 0 //def Q_WS_QWS
    void repaintUnclipped(const QRegion &, bool erase = true);
#endif

    void setUpdatesEnabled(bool enable);

public slots:
    void update();
    void repaint();

public:
    inline void update(int x, int y, int w, int h);
    void update(const QRect&);
    void update(const QRegion&);

    void repaint(int x, int y, int w, int h);
    void repaint(const QRect &);
    void repaint(const QRegion &);

public slots:
    // Widget management functions

    virtual void show();
    virtual void hide();
    void setShown(bool show);
    void setHidden(bool hide);

    void showMinimized();
    void showMaximized();
    void showFullScreen();
    void showNormal();

    bool close();
    void raise();
    void lower();

public:
    void stackUnder(QWidget*);
    void move(int x, int y);
    void move(const QPoint &);
    void resize(int w, int h);
    void resize(const QSize &);
    inline void setGeometry(int x, int y, int w, int h);
    void setGeometry(const QRect &);
    void adjustSize();
    bool isVisible() const;
    bool isVisibleTo(QWidget*) const;
    bool isHidden() const;
    bool isShown() const;
    bool isMinimized() const;
    bool isMaximized() const;
    bool isFullScreen() const;

    uint windowState() const;
    void setWindowState(uint windowState);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    void setSizePolicy(QSizePolicy);

    inline void setSizePolicy(QSizePolicy::SizeType horizontal, QSizePolicy::SizeType vertical);
    virtual int heightForWidth(int) const;

    QRegion clipRegion() const;

    void setContentsMargins(int left, int top, int right, int bottom);
    QRect contentsRect() const;
    QSize contentsMarginSize() const;

public:
#ifndef QT_NO_LAYOUT
    QLayout *layout() const;
#endif
    void updateGeometry();

    void setParent(QWidget *parent);
    void setParent(QWidget *parent, Qt::WFlags f);

    void scroll(int dx, int dy);
    void scroll(int dx, int dy, const QRect&);

    // Misc. functions

    QWidget *focusWidget() const;
    QWidget *nextInFocusChain() const;

    // drag and drop
    bool acceptDrops() const;
    void setAcceptDrops(bool on);

    //actions
    void addAction(QAction *action);
    void addActions(QList<QAction*> actions);
    void insertAction(QAction *before, QAction *action);
    void insertActions(QAction *before, QList<QAction*> actions);
    void removeAction(QAction *action);
    QList<QAction*> actions() const;

    // transparency and pseudo transparency
    void setAutoMask(bool);
    bool autoMask() const;

    QWidget *parentWidget() const;

    Qt::WState testWState(Qt::WState s) const;
    Qt::WFlags testWFlags(Qt::WFlags f) const;
    static QWidget *find(WId);
    static QWidgetMapper *wmapper();

    QWidget *childAt(int x, int y) const;
    inline QWidget *childAt(const QPoint &p) const
    { return childAt(p.x(), p.y()); }

#if defined(Q_WS_X11)
    const QX11Info &x11Info() const;
    Qt::HANDLE xftPictureHandle() const;
    Qt::HANDLE xftDrawHandle() const;
#endif

#if defined(Q_WS_WIN)
    HDC getDC() const;
    void releaseDC(HDC) const;
#else
    Qt::HANDLE handle() const;
#endif

    void setAttribute(Qt::WidgetAttribute, bool on = true);
    inline bool testAttribute(Qt::WidgetAttribute) const;

    QPaintEngine *paintEngine() const;

    void ensurePolished() const;

#if defined(Q_WS_X11)
    QInputContext *inputContext();
    void setInputContext(const QString &);
#endif

protected:
    // Event handlers
    bool event(QEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *);
#endif
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void moveEvent(QMoveEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void closeEvent(QCloseEvent *);
    virtual void contextMenuEvent(QContextMenuEvent *);
    virtual void tabletEvent(QTabletEvent *);
    virtual void actionEvent(QActionEvent *);

#ifndef QT_NO_DRAGANDDROP
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dragMoveEvent(QDragMoveEvent *);
    virtual void dragLeaveEvent(QDragLeaveEvent *);
    virtual void dropEvent(QDropEvent *);
#endif

    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    virtual void polishEvent(QEvent *);

    virtual void inputMethodEvent(QInputMethodEvent *);
public:
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery) const;
protected:

#if defined(Q_WS_MAC)
    virtual bool macEvent(EventHandlerCallRef, EventRef);
#endif
#if defined(Q_WS_WIN)
    virtual bool winEvent(MSG *message, long *result);
#endif
#if defined(Q_WS_X11)
    virtual bool x11Event(XEvent *);
#endif
#if defined(Q_WS_QWS)
    virtual bool qwsEvent(QWSEvent *);
    virtual unsigned char *scanLine(int) const;
    virtual int bytesPerLine() const;
#endif

    virtual void updateMask();

    // Misc. protected functions
    virtual void changeEvent(QEvent *);

    int metric(PaintDeviceMetric) const;

    void resetInputContext();
    void updateMicroFocus();

    void create(WId = 0, bool initializeWindow = true,
                         bool destroyOldWindow = true);
    void destroy(bool destroyWindow = true,
                 bool destroySubWindows = true);

    Qt::WState getWState() const;
    void setWState(Qt::WState f);
    void clearWState(Qt::WState f);

    inline Qt::WFlags getWFlags() const;
    void setWFlags(Qt::WFlags f);
    void clearWFlags(Qt::WFlags f);

    virtual bool focusNextPrevChild(bool next);

protected:
    QWidget(QWidgetPrivate &d, QWidget* parent, Qt::WFlags f);
private:
    void setWinId(WId);
    void showChildren(bool spontaneous);
    void hideChildren(bool spontaneous);
    void setParent_sys(QWidget *parent, Qt::WFlags);
    void deactivateWidgetCleanup();
    void setGeometry_sys(int, int, int, int, bool);
    void show_recursive();
    void show_helper();
    void show_sys();
    void hide_sys();
    void hide_helper();
    void setEnabled_helper(bool);
    void updateFrameStrut() const;

    bool testAttribute_helper(Qt::WidgetAttribute) const;

#if defined(Q_WS_QWS)
    void updateOverlappingChildren() const;
    void setChildrenAllocatedDirty();
    void setChildrenAllocatedDirty(const QRegion &r, const QWidget *dirty=0);
    bool isAllocatedRegionDirty() const;
    void updateRequestedRegion(const QPoint &gpos);
    QRegion requestedRegion() const;
    QRegion allocatedRegion() const;
    QRegion paintableRegion() const;

#ifndef QT_NO_CURSOR
    void updateCursor(const QRegion &r) const;
#endif

    // used to accumulate dirty region when children moved/resized.
    QRegion dirtyChildren;
    bool isSettingGeometry;
    friend class QWSManager;
    friend class QWSManagerPrivate;
    friend class QDecoration;
    friend class QWSPaintEngine;
#endif
    static int instanceCounter;  // Current number of widget instances
    static int maxInstances;     // Maximum number of widget instances

    static QWidgetMapper *mapper;

    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QBaseApplication;
    friend class QPainter;
    friend class QPixmap; // for QPixmap::fill()
    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QETWidget;
    friend class QLayout;
    friend class QWidgetItem;
    friend class QGLContext;
    friend class QFontEngineXft;
    friend class QX11PaintEngine;
    friend class QWin32PaintEngine;
    friend class QShortcutPrivate;

#ifdef Q_WS_MAC
    friend class QMacSavedPortInfo;
    friend class QQuickDrawPaintEngine;
    friend class QCoreGraphicsPaintEnginePrivate;
    friend QPoint posInWindow(const QWidget *w);
#endif

private:
    Q_DISABLE_COPY(QWidget)

    QWidgetData *data;

#ifdef QT_COMPAT
public:
    inline QT_COMPAT bool isVisibleToTLW() const;
    QT_COMPAT QRect visibleRect() const;
    inline QT_COMPAT void iconify() { showMinimized(); }
    inline QT_COMPAT void constPolish() const { ensurePolished(); }
    inline QT_COMPAT void polish() { ensurePolished(); }
    inline QT_COMPAT void reparent(QWidget *parent, Qt::WFlags f, const QPoint &p, bool showIt=false)
    { setParent(parent, f); setGeometry(p.x(),p.y(),width(),height()); if (showIt) show(); }
    inline QT_COMPAT void reparent(QWidget *parent, const QPoint &p, bool showIt=false)
    { setParent(parent, getWFlags() & ~Qt::WType_Mask); setGeometry(p.x(),p.y(),width(),height()); if (showIt) show(); }
    inline QT_COMPAT void recreate(QWidget *parent, Qt::WFlags f, const QPoint & p, bool showIt=false)
    { setParent(parent, f); setGeometry(p.x(),p.y(),width(),height()); if (showIt) show(); }
    inline QT_COMPAT void setSizePolicy(QSizePolicy::SizeType hor, QSizePolicy::SizeType ver, bool hfw)
    { QSizePolicy sp(hor, ver); sp.setHeightForWidth(hfw); setSizePolicy(sp);}
    inline QT_COMPAT bool hasMouse() const { return testAttribute(Qt::WA_UnderMouse); }
#ifndef QT_NO_CURSOR
    inline QT_COMPAT bool ownCursor() const { return testAttribute(Qt::WA_SetCursor); }
#endif
    inline QT_COMPAT bool ownFont() const { return testAttribute(Qt::WA_SetFont); }
    inline QT_COMPAT void unsetFont() { setFont(QFont()); }
    inline QT_COMPAT bool ownPalette() const { return testAttribute(Qt::WA_SetPalette); }
    inline QT_COMPAT void unsetPalette() { setPalette(QPalette()); }
    Qt::BackgroundMode QT_COMPAT backgroundMode() const;
    void QT_COMPAT setBackgroundMode(Qt::BackgroundMode, Qt::BackgroundMode = Qt::PaletteBackground);
    const QT_COMPAT QColor &eraseColor() const;
    void QT_COMPAT setEraseColor(const QColor &);
    const QT_COMPAT QColor &foregroundColor() const;
    const QT_COMPAT QPixmap *erasePixmap() const;
    void QT_COMPAT setErasePixmap(const QPixmap &);
#ifndef QT_NO_PALETTE
    const QT_COMPAT QColor &paletteForegroundColor() const;
    void QT_COMPAT setPaletteForegroundColor(const QColor &);
    const QT_COMPAT QColor &paletteBackgroundColor() const;
    void QT_COMPAT setPaletteBackgroundColor(const QColor &);
    const QT_COMPAT QPixmap *paletteBackgroundPixmap() const;
    void QT_COMPAT setPaletteBackgroundPixmap(const QPixmap &);
    const QT_COMPAT QBrush& backgroundBrush() const;
    const QT_COMPAT QColor &backgroundColor() const;
    const QT_COMPAT QPixmap *backgroundPixmap() const;
    void QT_COMPAT setBackgroundPixmap(const QPixmap &);
#endif
    QT_COMPAT void setBackgroundColor(const QColor &);
    QT_COMPAT QColorGroup colorGroup() const;
    QT_COMPAT QWidget *parentWidget(bool sameWindow) const;
    inline QT_COMPAT void setKeyCompression(bool b) { setAttribute(Qt::WA_KeyCompression, b); }
    inline QT_COMPAT void setFont(const QFont &f, bool) { setFont(f); }
#ifndef QT_NO_PALETTE
    inline QT_COMPAT void setPalette(const QPalette &p, bool) { setPalette(p); }
#endif
    enum BackgroundOrigin { WidgetOrigin, ParentOrigin, WindowOrigin, AncestorOrigin };
    inline QT_COMPAT void setBackgroundOrigin(BackgroundOrigin){};
    inline QT_COMPAT BackgroundOrigin backgroundOrigin() const { return WindowOrigin; }
    inline QT_COMPAT QPoint backgroundOffset() const { return QPoint(); }
    inline QT_COMPAT void repaint(bool) { repaint(); }
    inline QT_COMPAT void repaint(int x, int y, int w, int h, bool) { repaint(x,y,w,h); }
    inline QT_COMPAT void repaint(const QRect &r, bool) { repaint(r); }
    inline QT_COMPAT void repaint(const QRegion &rgn, bool) { repaint(rgn); }
    QT_COMPAT void erase();
    inline QT_COMPAT void erase(int x, int y, int w, int h) { erase_helper(x, y, w, h); }
    QT_COMPAT void erase(const QRect &);
    QT_COMPAT void erase(const QRegion &);
    QT_COMPAT void drawText(const QPoint &p, const QString &s)
    { drawText_helper(p.x(), p.y(), s); }
    inline QT_COMPAT void drawText(int x, int y, const QString &s)
    { drawText_helper(x, y, s); }
    QT_COMPAT bool close(bool);
    inline QT_COMPAT QWidget *childAt(int x, int y, bool includeThis) const
    {
        QWidget *w = childAt(x, y);
        return w ? w : ((includeThis && rect().contains(x,y))?const_cast<QWidget*>(this):0);
    }
    inline QT_COMPAT QWidget *childAt(const QPoint &p, bool includeThis) const
    {
        QWidget *w = childAt(p);
        return w ? w : ((includeThis && rect().contains(p))?const_cast<QWidget*>(this):0);
    }
    inline QT_COMPAT void setCaption(const QString &c)   { setWindowTitle(c); }
    inline QT_COMPAT void setIcon(const QPixmap &i)      { setWindowIcon(i); }
    inline QT_COMPAT void setIconText(const QString &it) { setWindowIconText(it); }
    inline QT_COMPAT QString caption() const             { return windowTitle(); }
    QT_COMPAT const QPixmap *icon() const;
    inline QT_COMPAT QString iconText() const            { return windowIconText(); }
    inline QT_COMPAT void setInputMethodEnabled(bool b) { setAttribute(Qt::WA_InputMethodEnabled, b); }
    inline QT_COMPAT bool isInputMethodEnabled() const { return testAttribute(Qt::WA_InputMethodEnabled); }

private:
    void drawText_helper(int x, int y, const QString &);
    void erase_helper(int x, int y, int w, int h);
#endif

protected:
#ifndef QT_NO_STYLE
    virtual void styleChange(QStyle&); // compat
#endif
    virtual void enabledChange(bool);  // compat
#ifndef QT_NO_PALETTE
    virtual void paletteChange(const QPalette &);  // compat
#endif
    virtual void fontChange(const QFont &); // compat
    virtual void windowActivationChange(bool);  // compat
    virtual void languageChange();  // compat
};

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QWidget *qt_cast_helper<QWidget*>(const QObject *o, QWidget *)
{
    if (!o || !o->isWidgetType()) return 0;
    return (QWidget*)(o);
}
#else
template <> inline QWidget *qt_cast<QWidget*>(QObject *o)
{
    if (!o || !o->isWidgetType()) return 0;
    return static_cast<QWidget*>(o);
}
template <> inline const QWidget *qt_cast<const QWidget*>(const QObject *o)
{
    if (!o || !o->isWidgetType()) return 0;
    return static_cast<const QWidget*>(o);
}
#endif

inline Qt::WState QWidget::testWState(Qt::WState s) const
{ return QFlag(data->widget_state & s); }

inline Qt::WFlags QWidget::testWFlags(Qt::WFlags f) const
{ return QFlag(data->widget_flags & f); }

inline WId QWidget::winId() const
{ return data->winid; }

inline bool QWidget::isTopLevel() const
{ return testWFlags(Qt::WType_TopLevel); }

inline bool QWidget::isDialog() const
{ return testWFlags(Qt::WType_Dialog); }

inline bool QWidget::isPopup() const
{ return testWFlags(Qt::WType_Popup); }

inline bool QWidget::isDesktop() const
{ return testWFlags(Qt::WType_Desktop); }

inline bool QWidget::isEnabled() const
{ return !testAttribute(Qt::WA_Disabled); }

inline bool QWidget::isModal() const
{ return testWFlags(Qt::WShowModal); }

inline bool QWidget::isEnabledToTLW() const
{ return isEnabled(); }

inline int QWidget::minimumWidth() const
{ return minimumSize().width(); }

inline int QWidget::minimumHeight() const
{ return minimumSize().height(); }

inline int QWidget::maximumWidth() const
{ return maximumSize().width(); }

inline int QWidget::maximumHeight() const
{ return maximumSize().height(); }

inline void QWidget::setMinimumSize(const QSize &s)
{ setMinimumSize(s.width(),s.height()); }

inline void QWidget::setMaximumSize(const QSize &s)
{ setMaximumSize(s.width(),s.height()); }

inline void QWidget::setSizeIncrement(const QSize &s)
{ setSizeIncrement(s.width(),s.height()); }

inline void QWidget::setBaseSize(const QSize &s)
{ setBaseSize(s.width(),s.height()); }

inline const QFont &QWidget::font() const
{ return data->fnt; }

inline QFontMetrics QWidget::fontMetrics() const
{ return QFontMetrics(data->fnt); }

inline QFontInfo QWidget::fontInfo() const
{ return QFontInfo(data->fnt); }

inline void QWidget::setMouseTracking(bool enable)
{ setAttribute(Qt::WA_MouseTracking, enable); }

inline bool QWidget::hasMouseTracking() const
{ return testAttribute(Qt::WA_MouseTracking); }

inline bool QWidget::underMouse() const
{ return testAttribute(Qt::WA_UnderMouse); }

inline bool  QWidget::isFocusEnabled() const
{ return focusPolicy() != Qt::NoFocus; }

inline bool QWidget::isUpdatesEnabled() const
{ return !testWState(Qt::WState_BlockUpdates); }

inline void QWidget::update(int x, int y, int w, int h)
{ update(QRect(x, y, w, h)); }

inline bool QWidget::isVisible() const
{ return testWState(Qt::WState_Visible); }

inline bool QWidget::isHidden() const
{ return testWState(Qt::WState_Hidden); }

inline bool QWidget::isShown() const
{ return !testWState(Qt::WState_Hidden); }

inline void QWidget::move(int x, int y)
{ move(QPoint(x, y)); }

inline void QWidget::resize(int w, int h)
{ resize(QSize(w, h)); }

inline void QWidget::setGeometry(int x, int y, int w, int h)
{ setGeometry(QRect(x, y, w, h)); }

inline QRect QWidget::rect() const
{ return QRect(0,0,data->crect.width(),data->crect.height()); }

inline const QRect &QWidget::geometry() const
{ return data->crect; }

inline QSize QWidget::size() const
{ return data->crect.size(); }

inline int QWidget::width() const
{ return data->crect.width(); }

inline int QWidget::height() const
{ return data->crect.height(); }

inline QWidget *QWidget::parentWidget() const
{ return static_cast<QWidget *>(QObject::parent()); }

inline QWidgetMapper *QWidget::wmapper()
{ return mapper; }

inline Qt::WState QWidget::getWState() const
{ return QFlag(data->widget_state); }

inline void QWidget::setWState(Qt::WState f)
{ data->widget_state |= f; }

inline void QWidget::clearWState(Qt::WState f)
{ data->widget_state &= ~f; }

inline Qt::WFlags QWidget::getWFlags() const
{ return QFlag(data->widget_flags); }

inline void QWidget::setWFlags(Qt::WFlags f)
{ data->widget_flags |= f; }

inline void QWidget::clearWFlags(Qt::WFlags f)
{ data->widget_flags &= ~f; }

inline void QWidget::setSizePolicy(QSizePolicy::SizeType hor, QSizePolicy::SizeType ver)
{ setSizePolicy(QSizePolicy(hor, ver)); }

inline bool QWidget::testAttribute(Qt::WidgetAttribute attribute) const
{
    if (attribute < int(8*sizeof(uint)))
        return data->widget_attributes & (1<<attribute);
    return testAttribute_helper(attribute);
}

#ifdef QT_COMPAT
inline bool QWidget::isVisibleToTLW() const
{ return isVisible(); }
inline QWidget *QWidget::parentWidget(bool sameWindow) const
{
    if (sameWindow && isTopLevel())
        return 0;
    return static_cast<QWidget *>(QObject::parent());
}
#ifndef QT_NO_PALETTE
inline QColorGroup QWidget::colorGroup() const
{ return QColorGroup(palette()); }
inline void QWidget::setPaletteForegroundColor(const QColor &c)
{ QPalette p = palette(); p.setColor(foregroundRole(), c); setPalette(p); }
inline const QBrush& QWidget::backgroundBrush() const { return palette().brush(backgroundRole()); }
inline void QWidget::setBackgroundPixmap(const QPixmap &pm)
{ QPalette p = palette(); p.setBrush(backgroundRole(), QBrush(pm)); setPalette(p); }
inline const QPixmap *QWidget::backgroundPixmap() const { return 0; }
inline void QWidget::setBackgroundColor(const QColor &c)
{ QPalette p = palette(); p.setColor(backgroundRole(), c); setPalette(p); }
inline const QColor & QWidget::backgroundColor() const { return palette().color(backgroundRole()); }
inline const QColor &QWidget::foregroundColor() const { return palette().color(foregroundRole());}
inline const QColor &QWidget::eraseColor() const { return palette().color(backgroundRole()); }
inline void QWidget::setEraseColor(const QColor &c)
{ QPalette p = palette(); p.setColor(backgroundRole(), c); setPalette(p); }
inline const QPixmap *QWidget::erasePixmap() const { return 0; }
inline void QWidget::setErasePixmap(const QPixmap &pm)
{ QPalette p = palette(); p.setBrush(backgroundRole(), QBrush(pm)); setPalette(p); }
inline const QColor &QWidget::paletteForegroundColor() const { return palette().color(foregroundRole());}
inline const QColor &QWidget::paletteBackgroundColor() const { return palette().color(backgroundRole()); }
inline void QWidget::setPaletteBackgroundColor(const QColor &c)
{ QPalette p = palette(); p.setColor(backgroundRole(), c); setPalette(p); }
inline const QPixmap *QWidget::paletteBackgroundPixmap() const
{ return 0; }
inline void QWidget::setPaletteBackgroundPixmap(const QPixmap &pm)
{ QPalette p = palette(); p.setBrush(backgroundRole(), QBrush(pm)); setPalette(p); }
#else
inline QT_COMPAT void QWidget::setBackgroundColor(const QColor &) {}
#endif
inline QT_COMPAT void QWidget::erase() { erase_helper(0, 0, data->crect.width(), data->crect.height()); }
inline QT_COMPAT void QWidget::erase(const QRect &r) { erase_helper(r.x(), r.y(), r.width(), r.height()); }
#endif

#define QWIDGETSIZE_MAX ((1<<24)-1)

#endif // QWIDGET_H
