#ifndef QMACSTYLECG_MAC_H
#define QMACSTYLECG_MAC_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#if QT_MACOSX_VERSION >= 0x1030

#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)

class QPalette;

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_MAC
#else
#define Q_GUI_EXPORT_STYLE_MAC Q_GUI_EXPORT
#endif

class QMacStyleCGPrivate;

class Q_GUI_EXPORT_STYLE_MAC QMacStyleCG : public QCommonStyle
{
public:
    QMacStyleCG();
    ~QMacStyleCG();

    void polish(QWidget * w);
    void unPolish(QWidget * w);
    void polish(QApplication*);

    void drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r, const QPalette &pal,
			SFlags flags, const QStyleOption& = QStyleOption::Default) const;

    void drawControl(ControlElement element, QPainter *p, const QWidget *widget, const QRect &r,
		      const QPalette &pal, SFlags how, const QStyleOption& = QStyleOption::Default) const;

    void drawComplexControl(ComplexControl control, QPainter* p, const QWidget* w, const QRect& r,
			    const QPalette& pal, SFlags flags, SCFlags sub, SCFlags subActive,
			    const QStyleOption& = QStyleOption::Default) const;

    int pixelMetric(PixelMetric metric, const QWidget *widget) const;


    QRect querySubControlMetrics(ComplexControl control, const QWidget *w, SubControl sc,
				  const QStyleOption& = QStyleOption::Default) const;

    QRect subRect(SubRect, const QWidget *w) const;

    SubControl querySubControl(ComplexControl control,const QWidget *widget, const QPoint &pos,
				const QStyleOption& = QStyleOption::Default) const;

    int styleHint(StyleHint sh, const QWidget *, const QStyleOption &, QStyleHintReturn *) const;

    QSize sizeFromContents(ContentsType contents, const QWidget *w, const QSize &contentsSize,
			    const QStyleOption& = QStyleOption::Default) const;

    QPixmap stylePixmap(StylePixmap sp, const QWidget *widget, const QStyleOption& = QStyleOption::Default) const;

    QPixmap stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
			const QPalette &pal, const QStyleOption& = QStyleOption::Default) const;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMacStyleCG(const QMacStyleCG &);
    QMacStyleCG& operator=(const QMacStyleCG &);
#endif

protected:
    QMacStyleCGPrivate *d;

};

#endif // Q_WS_MAC

#endif /* QMACSTYLECG_MAC_H */
#endif
