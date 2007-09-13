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

#ifndef WIDGETBOX_DNDITEM_H
#define WIDGETBOX_DNDITEM_H

#include <qdesigner_dnditem_p.h>
#include "widgetbox_global.h"

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class DomWidget;

namespace qdesigner_internal {

class QT_WIDGETBOX_EXPORT WidgetBoxDnDItem : public QDesignerDnDItem
{
public:
    WidgetBoxDnDItem(QDesignerFormEditorInterface *core,
                        DomWidget *dom_widget,
                        const QPoint &global_mouse_pos);
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETBOX_DNDITEM_H
