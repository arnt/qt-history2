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

/*
TRANSLATOR qdesigner_internal::QDesignerWidgetBox
*/

#include "qdesigner_widgetbox_p.h"

namespace qdesigner_internal {
QDesignerWidgetBox::QDesignerWidgetBox(QWidget *parent, Qt::WindowFlags flags)
    : QDesignerWidgetBoxInterface(parent, flags),
      m_loadMode(LoadMerge)
{

}

QDesignerWidgetBox::LoadMode QDesignerWidgetBox::loadMode() const
{
    return m_loadMode;
}

void QDesignerWidgetBox::setLoadMode(LoadMode lm)
{
     m_loadMode = lm;
}
}  // namespace qdesigner_internal
