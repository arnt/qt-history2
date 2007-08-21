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

#ifndef LAYOUTINFO_H
#define LAYOUTINFO_H

#include "shared_global_p.h"

class QWidget;
class QLayout;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT LayoutInfo
{
public:
    enum Type
    {
        NoLayout,
        HSplitter,
        VSplitter,
        HBox,
        VBox,
        Grid
    };

    static void deleteLayout(const QDesignerFormEditorInterface *core, QWidget *widget);

    static Type layoutType(const QDesignerFormEditorInterface *core, const QWidget *w);
    static Type layoutType(const QDesignerFormEditorInterface *core, const QLayout *layout);

    static QWidget *layoutParent(const QDesignerFormEditorInterface *core, QLayout *layout);

    static Type laidoutWidgetType(const QDesignerFormEditorInterface *core, QWidget *widget, bool *isManaged = 0);
    static bool inline isWidgetLaidout(const QDesignerFormEditorInterface *core, QWidget *widget) { return laidoutWidgetType(core, widget) != NoLayout; }

    static QLayout *managedLayout(const QDesignerFormEditorInterface *core, const QWidget *widget);
    static QLayout *managedLayout(const QDesignerFormEditorInterface *core, QLayout *layout);
    static QLayout *internalLayout(const QWidget *widget);
};

} // namespace qdesigner_internal

#endif // LAYOUTINFO_H
