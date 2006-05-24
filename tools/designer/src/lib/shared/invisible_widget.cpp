/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "invisible_widget_p.h"

namespace qdesigner_internal {

InvisibleWidget::InvisibleWidget(QWidget *parent)
    : QWidget()
{
    setAttribute(Qt::WA_NoChildEventsForParent);
    setParent(parent);
}

} // namespace qdesigner_internal
