/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_GRID_H
#define QDESIGNER_GRID_H

#include "shared_global_p.h"

#include <QtCore/QVariantMap>

class QWidget;
class QPaintEvent;

namespace qdesigner_internal {

// Designer grid which is able to serialize to QVariantMap
class QDESIGNER_SHARED_EXPORT Grid
{
public:
    Grid();

    void fromVariantMap(const QVariantMap& vm);

    void addToVariantMap(QVariantMap& vm) const;
    QVariantMap toVariantMap() const;

    inline bool visible() const   { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    inline bool snapX() const     { return m_snapX; }
    void setSnapX(bool snap)      { m_snapX = snap; }

    inline bool snapY() const     { return m_snapY; }
    void setSnapY(bool snap)      { m_snapY = snap; }

    inline int deltaX() const     { return m_deltaX; }
    void setDeltaX(int dx)        { m_deltaX = dx; }

    inline int deltaY() const     { return m_deltaY; }
    void setDeltaY(int dy)        { m_deltaY = dy; }

    void paint(QWidget *widget, QPaintEvent *e, bool needFrame = false) const;

    QPoint snapPoint(const QPoint &p) const;

    int widgetHandleAdjustX(int x) const;
    int widgetHandleAdjustY(int y) const;

private:
    bool m_visible;
    bool m_snapX;
    bool m_snapY;
    int m_deltaX;
    int m_deltaY;
};
} // namespace qdesigner_internal

#endif // QDESIGNER_GRID_H
