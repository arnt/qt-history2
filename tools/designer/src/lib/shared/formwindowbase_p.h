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

#ifndef FORMWINDOWBASE_H
#define FORMWINDOWBASE_H

#include "shared_global_p.h"
#include "grid_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtCore/QVariantMap>
#include <QtCore/QList>

class QDesignerDnDItemInterface;
class QMenu;
class QPoint;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT FormWindowBase: public QDesignerFormWindowInterface
{
    Q_OBJECT

public:
    enum HighlightMode  { Restore, Highlight };

    FormWindowBase(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    QVariantMap formData();
    void setFormData(const QVariantMap &vm);

    // Deprecated
    virtual QPoint grid() const;

    // Deprecated
    virtual void setGrid(const QPoint &grid);

    virtual bool hasFeature(Feature f) const;
    virtual Feature features() const;
    virtual void setFeatures(Feature f);

    const Grid &designerGrid() const { return m_grid; }
    void setDesignerGrid(const  Grid& grid);

    bool hasFormGrid() const { return m_hasFormGrid; }
    void setHasFormGrid(bool b) { m_hasFormGrid = b; }

    bool gridVisible() const;

    static const Grid &defaultDesignerGrid() { return m_defaultGrid; }
    static void setDefaultDesignerGrid(const  Grid& grid);

    // Overwrite to initialize and return a popup menu for a managed widget
    virtual QMenu *initializePopupMenu(QWidget *managedWidget);

    virtual bool dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list, QWidget *target,
                             const QPoint &global_mouse_pos) = 0;

    // Helper to find the widget at the mouse position with some flags.
    enum WidgetUnderMouseMode { FindSingleSelectionDropTarget, FindMultiSelectionDropTarget };
    QWidget *widgetUnderMouse(const QPoint &formPos, WidgetUnderMouseMode m);

    virtual QWidget *widgetAt(const QPoint &pos) = 0;
    virtual QWidget *findContainer(QWidget *w, bool excludeLayout) const = 0;

    void deleteWidgetList(const QWidgetList &widget_list);

    virtual void highlightWidget(QWidget *w, const QPoint &pos, HighlightMode mode = Highlight) = 0;

private:
    void syncGridFeature();
    static Grid m_defaultGrid;

    Feature m_feature;
    Grid m_grid;
    bool m_hasFormGrid;
};
}  // namespace qdesigner_internal

#endif // FORMWINDOWBASE_H
