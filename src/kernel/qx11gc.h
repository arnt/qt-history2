#include "qabstractgc.h"
#include "qshared.h"

#ifndef QX11GC_H
#define QX11GC_H

struct QX11GCData;
class QX11GCPrivate;
class QPainterState;

class QX11GC : public QAbstractGC
{
public:
    QX11GC(const QPaintDevice *);
    ~QX11GC();

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool begin = FALSE);
    bool end();

    void updatePen(QPainterState *ps);
    void updateBrush(QPainterState *ps);
    void updateFont(QPainterState *ps);
    void updateRasterOp(QPainterState *ps);
    void updateBackground(QPainterState *ps);
    void updateXForm(QPainterState *ps);
    void updateClipRegion(QPainterState *ps);

    void setRasterOp(RasterOp r);

    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int w, int h);
    void drawPoint(int x, int y);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &penColor);
    void drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd);
    void drawEllipse(int x, int y, int w, int h);
    void drawArc(int x, int y, int w, int h, int a, int alen);
    void drawPie(int x, int y, int w, int h, int a, int alen);
    void drawChord(int x, int y, int w, int h, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif

    void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh);
    void drawTextItem(int x, int y, const QTextItem &ti, int textflags);

    virtual Qt::HANDLE handle() const;

    Display *x11Display() const;
    int x11Screen() const;
    int x11Depth() const;
    int x11Cells() const;
    Qt::HANDLE x11Colormap() const;
    bool x11DefaultColormap() const;
    void *x11Visual() const;
    bool x11DefaultVisual() const;

    static void initialize();
    static void cleanup();

    static Display *x11AppDisplay();
    static int x11AppScreen();

    static int x11AppDepth(int screen = -1);
    static int x11AppCells(int screen = -1);
    static Qt::HANDLE x11AppRootWindow(int screen = -1);
    static Qt::HANDLE x11AppColormap(int screen = -1);
    static void *x11AppVisual(int screen = -1);
    static bool x11AppDefaultColormap(int screen = -1);
    static bool x11AppDefaultVisual(int screen = -1);
    static int x11AppDpiX(int screen = -1);
    static int x11AppDpiY(int screen = -1);
    static void x11SetAppDpiX(int screen, int xdpi);
    static void x11SetAppDpiY(int screen, int ydpi);

protected:
    void copyX11Data(const QX11GC *);
    void cloneX11Data(const QX11GC *);
    virtual void setX11Data(const QX11GCData *);
    QX11GCData* getX11Data(bool def = FALSE) const;
    
    friend void qt_init( QApplicationPrivate *, int, Display *, Qt::HANDLE, Qt::HANDLE );
    friend void qt_cleanup();
    friend class QWidget;
    friend class QFontEngineBox;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;
    
private:
    static Display *x_appdisplay;
    static int x_appscreen;

    static int *x_appdepth_arr;
    static int *x_appcells_arr;
    static Qt::HANDLE *x_approotwindow_arr;
    static Qt::HANDLE *x_appcolormap_arr;
    static bool *x_appdefcolormap_arr;
    static void **x_appvisual_arr;
    static bool *x_appdefvisual_arr;

    QX11GCPrivate *d;
    QX11GCData *x11Data;

private:
#if defined(Q_DISABLE_COPY)
    QX11GC(const QX11GC &);
    QX11GC &operator=(const QX11GC &);
#endif
};

struct QX11GCData : public QShared {
    Display *x_display;
    int x_screen;
    int x_depth;
    int x_cells;
    Qt::HANDLE x_colormap;
    bool x_defcolormap;
    void *x_visual;
    bool x_defvisual;
};

//
// inline functions
//

inline Display *QX11GC::x11Display() const
{ return x11Data ? x11Data->x_display : x_appdisplay; }

inline int QX11GC::x11Screen() const
{ return x11Data ? x11Data->x_screen : x_appscreen; }

inline int QX11GC::x11Depth() const
{ return x11Data ? x11Data->x_depth : x11AppDepth(); }

inline int QX11GC::x11Cells() const
{ return x11Data ? x11Data->x_cells : x11AppCells(); }

inline Qt::HANDLE QX11GC::x11Colormap() const
{ return x11Data ? x11Data->x_colormap : x11AppColormap(); }

inline bool QX11GC::x11DefaultColormap() const
{ return x11Data ? x11Data->x_defcolormap : x11AppDefaultColormap(); }

inline void *QX11GC::x11Visual() const
{ return x11Data ? x11Data->x_visual : x11AppVisual(); }

inline bool QX11GC::x11DefaultVisual() const
{ return x11Data ? x11Data->x_defvisual : x11AppDefaultVisual(); }

inline Display *QX11GC::x11AppDisplay()
{ return x_appdisplay; }

inline int QX11GC::x11AppScreen()
{ return x_appscreen; }

inline int QX11GC::x11AppDepth(int screen)
{ return x_appdepth_arr[screen == -1 ? x_appscreen : screen]; }

inline int QX11GC::x11AppCells(int screen)
{ return x_appcells_arr[screen == -1 ? x_appscreen : screen]; }

inline Qt::HANDLE QX11GC::x11AppRootWindow(int screen)
{ return x_approotwindow_arr[screen == -1 ? x_appscreen : screen]; }

inline Qt::HANDLE QX11GC::x11AppColormap(int screen)
{ return x_appcolormap_arr[screen == -1 ? x_appscreen : screen]; }

inline bool QX11GC::x11AppDefaultColormap(int screen)
{ return x_appdefcolormap_arr[screen == -1 ? x_appscreen : screen]; }

inline void *QX11GC::x11AppVisual(int screen)
{ return x_appvisual_arr[screen == -1 ? x_appscreen : screen]; }

inline bool QX11GC::x11AppDefaultVisual(int screen)
{ return x_appdefvisual_arr[screen == -1 ? x_appscreen : screen]; }

#endif // QX11GC_H
