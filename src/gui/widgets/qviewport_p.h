#ifndef QVIEWPORT_P_H
#define QVIEWPORT_P_H
#include <private/qframe_p.h>
class QScrollBar;
class QViewportPrivate: public QFramePrivate
{
    Q_DECLARE_PUBLIC(QViewport)

public:
    QViewportPrivate();
    QScrollBar *hbar, *vbar;
    Qt::ScrollBarPolicy vbarpolicy, hbarpolicy;

    QWidget *viewport;
    int left, top, right, bottom; // viewport margin

    int xoffset, yoffset;

    void init();
    void layoutChildren();
    bool viewportEvent(QEvent *);

    void hslide(int);
    void vslide(int);
    void showOrHideScrollBars();
};

#endif
