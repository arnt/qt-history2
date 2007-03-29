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

#ifndef DESIGNEROBJECTINSPECTOR_H
#define DESIGNEROBJECTINSPECTOR_H

#include "shared_global_p.h"
#include <QtDesigner/QDesignerObjectInspectorInterface>
#include <QtCore/QList>

class QDesignerDnDItemInterface;

namespace qdesigner_internal {

struct QDESIGNER_SHARED_EXPORT Selection {
    typedef QList<QObject *> ObjectList;

    bool empty() const;
    void clear();
    // Merge lists
    ObjectList selection() const;

    // Selection in cursor
    QWidgetList m_cursorSelection;
    // Remaining selected objects (non-widgets)
    ObjectList m_selectedObjects;
};

// Extends the QDesignerObjectInspectorInterface by functionality
// to access the selection

class QDESIGNER_SHARED_EXPORT QDesignerObjectInspector: public QDesignerObjectInspectorInterface
{
    Q_OBJECT
public:
    QDesignerObjectInspector(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    // Select a qobject unmanaged by form window
    virtual bool selectObject(QObject *o) = 0;
    virtual void getSelection(Selection &s) const = 0;
    virtual void clearSelection() = 0;

    virtual QWidget *widgetAt(const QPoint &global_mouse_pos) = 0;

public slots:
    virtual void mainContainerChanged();
};

}  // namespace qdesigner_internal

#endif // DESIGNEROBJECTINSPECTOR_H
