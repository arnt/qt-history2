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

#include <private/qabstractbutton_p.h>


class QToolButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QToolButton)
public:
    void init();
    void popupPressed();
    void popupTimerDone();
    QStyleOptionToolButton getStyleOption() const;
    QPointer<QMenu> menu; //the menu set by the user (setMenu)
    QPointer<QMenu> actualMenu; //the menu being displayed (could be the same as menu above)
    QBasicTimer popupTimer;
    int delay;
    Qt::ArrowType arrow;
    Qt::IconSize iconSize;
    Qt::ToolButtonStyle toolButtonStyle;
    QToolButton::ToolButtonPopupMode popupMode;
    uint instantPopup          : 1;
    uint autoRaise             : 1;
    uint repeat                : 1;
    uint hasArrow              : 1;
    uint discardNextMouseEvent : 1;
};
