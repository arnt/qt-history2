#ifndef QVIEWPORT_P_H
#define QVIEWPORT_P_H
#include <private/qframe_p.h>
class QScrollBar;
class QViewportHelper;
class QViewportPrivate: public QFramePrivate
{
    Q_DECLARE_PUBLIC(QViewport);

public:
    QViewportPrivate();
    QScrollBar *hbar, *vbar;
    ScrollBarPolicy vbarpolicy, hbarpolicy;

    QViewportHelper *viewport;
    int left, top, right, bottom; // viewport margin

    int xoffset, yoffset;

    void init();
    void layoutChildren();
    bool viewportEvent(QEvent *);
};

#endif
