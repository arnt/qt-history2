#ifndef LIGHTSTYLE_H
#define LIGHTSTYLE_H


#ifndef QT_H
#include <qstyle.h>
#include <qwindowsstyle.h>
#endif // QT_H

#ifdef QT_PLUGIN_STYLE_LIGHT
#  define Q_EXPORT_STYLE_LIGHT
#else
#  define Q_EXPORT_STYLE_LIGHT Q_EXPORT
#endif // QT_PLUGIN_STYLE_LIGHT


class Q_EXPORT_STYLE_LIGHT LightStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    LightStyle();
    virtual ~LightStyle();

    void polish(QPalette &);
    void polish(QWidget *widget);
    void unPolish(QWidget*widget);

    void polish(QApplication *app);
    void unPolish(QApplication *app);

    void polishPopupMenu(QPopupMenu *menu);

    void drawPushButton(QPushButton *button, QPainter *p);
    void drawButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken = FALSE,
                    const QBrush *fill = 0);
    void drawBevelButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken = FALSE,
                         const QBrush *fill = 0);
    void getButtonShift(int &x, int &y) const;

    void drawComboButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken = FALSE,
                         bool editable = FALSE, bool = TRUE,
                         const QBrush *fill = 0);
    QRect comboButtonRect(int x, int y, int w, int h) const;
    QRect comboButtonFocusRect(int x, int y, int w, int h) const;

    void drawIndicator(QPainter *p, int x, int y ,int w, int h,
                       const QColorGroup &g, int state,
                       bool = FALSE, bool = TRUE);
    QSize indicatorSize() const;

    void drawExclusiveIndicator(QPainter *p, int x, int y ,int w, int h,
                                const QColorGroup &g, bool on,
                                bool = FALSE, bool = TRUE);
    QSize exclusiveIndicatorSize() const;

    void drawPanel(QPainter * p, int x, int y, int w, int h,
                   const QColorGroup &g, bool sunken = FALSE,
                   int = 1, const QBrush * = 0);

    void scrollBarMetrics( const QScrollBar *,
                           int &, int &, int &, int & ) const;
    void drawScrollBarControls(QPainter* p, const QScrollBar* sb,
                               int sliderStart, uint controls,
                               uint activeControl);
    QStyle::ScrollControl scrollBarPointOver(const QScrollBar *, int, const QPoint& p);

    void drawTab(QPainter *p, const QTabBar *tabbar, QTab *tab, bool selected);

    void drawSlider(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, Orientation orientation,
                    bool, bool);
    void drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup& g, QCOORD,
                          Orientation );

    void drawMenuBarPanel(QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &cg, const QBrush *fill = 0);

    QSize scrollBarExtent() const;
    int buttonDefaultIndicatorWidth() const;
    int buttonMargin() const;
    int sliderThickness() const;
    int sliderLength() const;
    int defaultFrameWidth() const;


protected:
    bool eventFilter(QObject *, QEvent *);
};


#endif // LIGHTSTYLE_H
