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

#include "formwindow.h"

class FormWindowDnDItem : public AbstractDnDItem
{
    Q_OBJECT
public:
    FormWindowDnDItem(QWidget *widget, const QPoint &pos);
    FormWindowDnDItem(DomUI *dom_ui, QWidget *widget, const QPoint &pos);

    virtual ~FormWindowDnDItem();

    virtual DomUI *domUi() const;
    virtual QWidget *decoration() const;
    virtual QWidget *widget() const;
    virtual QPoint hotSpot() const;

private:
    QWidget *m_decoration, *m_widget;
    DomUI *m_dom_ui;
    QPoint m_hot_spot;
};

#endif // FORMWINDOW_DNDITEM_H
