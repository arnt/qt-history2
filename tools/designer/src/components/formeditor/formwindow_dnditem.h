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

#ifndef FORMWINDOW_DNDITEM_H
#define FORMWINDOW_DNDITEM_H

#include <qdesigner_dnditem.h>

namespace qdesigner_internal {

class FormWindow;

class FormWindowDnDItem : public QDesignerDnDItem
{
public:
    FormWindowDnDItem(QDesignerDnDItemInterface::DropType type, FormWindow *form,
                        QWidget *widget, const QPoint &global_mouse_pos);
    virtual DomUI *domUi() const;
};

}  // namespace qdesigner_internal

#endif // FORMWINDOW_DNDITEM_H
