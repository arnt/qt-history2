#ifndef QFRAME_P_H
#define QFRAME_P_H

#include <private/qwidget_p.h>

class QFramePrivate : public QWidgetPrivate
{
public:
    QFramePrivate();

    void        updateFrameWidth(bool=FALSE);

    QRect       frect;
    int         frameStyle;
    short       lineWidth;
    short       margin;
    short       midLineWidth;
    short       frameWidth;

    Q_DECL_PUBLIC(QFrame);
};

#endif
