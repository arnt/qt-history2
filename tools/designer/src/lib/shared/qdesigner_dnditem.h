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

#ifndef QDESIGNER_DNDITEM_H
#define QDESIGNER_DNDITEM_H

#include <QtCore/QPoint>

#include "shared_global.h"
#include <abstractdnditem.h>

class QT_SHARED_EXPORT QDesignerDnDItem: public AbstractDnDItem
{
public:
    QDesignerDnDItem(DropType type, QWidget *source = 0);
    virtual ~QDesignerDnDItem();

    virtual DomUI *domUi() const;
    virtual QWidget *decoration() const;
    virtual QWidget *widget() const;
    virtual QPoint hotSpot() const;
    virtual QWidget *source() const;

    virtual DropType type() const;

protected:
    virtual void init(DomUI *ui, QWidget *widget, QWidget *decoration, const QPoint &global_mouse_pos);

private:
    QWidget *m_source;
    DropType m_type;
    DomUI *m_dom_ui;
    QWidget *m_widget;
    QWidget *m_decoration;
    QPoint m_hot_spot;

    Q_DISABLE_COPY(QDesignerDnDItem)
};
#endif // QDESIGNER_DNDITEM_H
