#ifndef QWINDOWSXPSTYLE_H
#define QWINDOWSXPSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_WINDOWSXP

#if defined(QT_PLUGIN_STYLE_WINDOWSXP)
#define Q_EXPORT_STYLE_WINDOWSXP
#else
#define Q_EXPORT_STYLE_WINDOWSXP Q_EXPORT
#endif

class Q_EXPORT_STYLE_WINDOWSXP QWindowsXPStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QWindowsXPStyle();
    ~QWindowsXPStyle();

    void drawButton( QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken = FALSE,
                     const QBrush *fill = 0 );

    void drawPushButton( QPushButton* btn, QPainter *p);

    void drawPanel( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &, bool sunken=FALSE,
                    int lineWidth = 1, const QBrush *fill = 0 );

    // RadioButton
    QSize exclusiveIndicatorSize() const;

    void drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
		    const QColorGroup &g, bool on, bool down = FALSE, bool enabled = TRUE );

    // CheckBox
    QSize indicatorSize() const;

    void drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
			    int state, bool down = FALSE, bool enabled = TRUE );

private:	// Disabled copy constructor and operator=
    class Private;
    Private *d;

#if defined(Q_DISABLE_COPY)
    QWindowsXPStyle( const QWindowsXPStyle & );
    QWindowsXPStyle& operator=( const QWindowsXPStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWSXP

#endif // QWINDOWSXPSTYLE_H
