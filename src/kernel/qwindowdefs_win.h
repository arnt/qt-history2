#ifndef QWINDOWDEFS_WIN_H
#ifndef QT_H
#endif // QT_H
#define QWINDOWDEFS_WIN_H

#ifndef QT_H
#endif // QT_H

#if defined(Q_CC_BOR) || defined(Q_CC_WAT)
#define NEEDS_QMAIN
#endif

#if !defined(Q_NOWINSTRICT)
#define Q_WINSTRICT
#endif

#if defined(Q_WINSTRICT)

#if !defined(STRICT)
#define STRICT
#endif
#undef NO_STRICT
#define Q_DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name

#else

#if !defined(NO_STRICT)
#define NO_STRICT
#endif
#undef  STRICT
#define Q_DECLARE_HANDLE(name) typedef HANDLE name

#endif

Q_DECLARE_HANDLE(HINSTANCE);
Q_DECLARE_HANDLE(HDC);
Q_DECLARE_HANDLE(HWND);
Q_DECLARE_HANDLE(HFONT);
Q_DECLARE_HANDLE(HPEN);
Q_DECLARE_HANDLE(HBRUSH);
Q_DECLARE_HANDLE(HBITMAP);
Q_DECLARE_HANDLE(HICON);
typedef HICON HCURSOR;
Q_DECLARE_HANDLE(HPALETTE);
Q_DECLARE_HANDLE(HRGN);
Q_DECLARE_HANDLE(HMONITOR);

typedef struct tagMSG MSG;
typedef HWND WId;

Q_EXPORT HINSTANCE qWinAppInst();
Q_EXPORT HINSTANCE qWinAppPrevInst();
Q_EXPORT int	   qWinAppCmdShow();
Q_EXPORT HDC	   qt_display_dc();

#define QT_WIN_PAINTER_MEMBERS \
    HDC		hdc;		\
    HPEN	hpen;		\
    HFONT	hfont;		\
    HBRUSH	hbrush;		\
    HBITMAP	hbrushbm;	\
    HPALETTE	holdpal;	\
    void       *textmet;	\
    uint	killFont    : 1;\
    uint	nocolBrush  : 1;\
    uint	pixmapBrush : 1;\
    void       *textMetric();	\
    void	nativeXForm( bool );

#endif
