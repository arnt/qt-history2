#ifndef QFRAME_P_H
#define QFRAME_P_H

#include <private/qwidget_p.h>

class QFramePrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QFrame)
public:
    QFramePrivate();

    void        updateFrameWidth();

    QRect       frect;
    int         frameStyle;
    short       lineWidth;
    short       midLineWidth;
    short       frameWidth;

};

#endif
