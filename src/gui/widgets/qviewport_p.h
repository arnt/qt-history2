/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVIEWPORT_P_H
#define QVIEWPORT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

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

#endif // QVIEWPORT_P_H
