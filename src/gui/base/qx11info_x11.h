#ifndef Q_X11INFO_X11_H
#define Q_X11INFO_X11_H

#include "qnamespace.h"
#include "qshared.h"

struct QX11InfoData;
class QPaintDevice;
class QApplicationPrivate;
class QX11InfoPrivate;

typedef struct _XDisplay Display;

class QX11Info
{
public:
    QX11Info();
    ~QX11Info();
    QX11Info(const QX11Info &other);
    QX11Info &operator=(const QX11Info &other);

    Display *display() const;
    int screen() const;
    int depth() const;
    int cells() const;
    Qt::HANDLE colormap() const;
    bool defaultColormap() const;
    void *visual() const;
    bool defaultVisual() const;

    static Display *appDisplay();
    static int appScreen();
    static int appDepth(int screen = -1);
    static int appCells(int screen = -1);
    static Qt::HANDLE appColormap(int screen = -1);
    static void *appVisual(int screen = -1);
    static Qt::HANDLE appRootWindow(int screen = -1);
    static bool appDefaultColormap(int screen = -1);
    static bool appDefaultVisual(int screen = -1);
    static int appDpiX(int screen = -1);
    static int appDpiY(int screen = -1);
    static void setAppDpiX(int screen, int dpi);
    static void setAppDpiY(int screen, int dpi);

protected:
    void copyX11Data(const QPaintDevice *);
    void cloneX11Data(const QPaintDevice *);
    void setX11Data(const QX11InfoData *);
    QX11InfoData* getX11Data(bool def = false) const;

    static Display *x_appdisplay;
    static int x_appscreen;
    static int *x_appdepth_arr;
    static int *x_appcells_arr;
    static Qt::HANDLE *x_approotwindow_arr;
    static Qt::HANDLE *x_appcolormap_arr;
    static bool *x_appdefcolormap_arr;
    static void **x_appvisual_arr;
    static bool *x_appdefvisual_arr;

    QX11InfoData *x11data;

    friend class QX11PaintEngine;
    friend class QPixmap;
    friend class QWidget;
    friend class QGLWidget;
    friend void qt_init(QApplicationPrivate *priv, int, Display *display, Qt::HANDLE visual,
			Qt::HANDLE colormap);
    friend void qt_cleanup();
};

// ### hm. maybe move this - needed for inlining
struct QX11InfoData : public QShared {
    Display *x_display;
    int x_screen;
    int x_depth;
    int x_cells;
    Qt::HANDLE x_colormap;
    bool x_defcolormap;
    void *x_visual;
    bool x_defvisual;
};

inline Display *QX11Info::appDisplay()
{ return x_appdisplay; }

inline int QX11Info::appScreen()
{ return x_appscreen; }

inline Qt::HANDLE QX11Info::appColormap(int screen)
{ return x_appcolormap_arr[screen == -1 ? x_appscreen : screen]; }

inline void *QX11Info::appVisual(int screen)
{ return x_appvisual_arr[screen == -1 ? x_appscreen : screen]; }

inline Qt::HANDLE QX11Info::appRootWindow(int screen)
{ return x_approotwindow_arr[screen == -1 ? x_appscreen : screen]; }

inline int QX11Info::appDepth(int screen)
{ return x_appdepth_arr[screen == -1 ? x_appscreen : screen]; }

inline int QX11Info::appCells(int screen)
{ return x_appcells_arr[screen == -1 ? x_appscreen : screen]; }

inline bool QX11Info::appDefaultColormap(int screen)
{ return x_appdefcolormap_arr[screen == -1 ? x_appscreen : screen]; }

inline bool QX11Info::appDefaultVisual(int screen)
{ return x_appdefvisual_arr[screen == -1 ? x_appscreen : screen]; }

inline Display *QX11Info::display() const
{ return x11data ? x11data->x_display : QX11Info::appDisplay(); }

inline int QX11Info::screen() const
{ return x11data ? x11data->x_screen : QX11Info::appScreen(); }

inline int QX11Info::depth() const
{ return x11data ? x11data->x_depth : QX11Info::appDepth(); }

inline int QX11Info::cells() const
{ return x11data ? x11data->x_cells : QX11Info::appCells(); }

inline Qt::HANDLE QX11Info::colormap() const
{ return x11data ? x11data->x_colormap : QX11Info::appColormap(); }

inline bool QX11Info::defaultColormap() const
{ return x11data ? x11data->x_defcolormap : QX11Info::appDefaultColormap(); }

inline void *QX11Info::visual() const
{ return x11data ? x11data->x_visual : QX11Info::appVisual(); }

inline bool QX11Info::defaultVisual() const
{ return x11data ? x11data->x_defvisual : QX11Info::appDefaultVisual(); }

#endif // Q_X11INFO_X11_H
