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

#include "shared_global.h"
#include <abstractformwindowmanager.h>

class QT_SHARED_EXPORT QDesignerDnDItem: public AbstractDnDItem
{
    Q_OBJECT
public:
    QDesignerDnDItem();
    virtual ~QDesignerDnDItem();

    virtual DomUI *domUi() const = 0;
    virtual QWidget *decoration() const = 0;
    virtual QPoint hotSpot() const = 0;

    static QDesignerDnDItem *create(QWidget *widget, const QPoint &hotSpot);
    static QDesignerDnDItem *create(DomUI *ui, QWidget *widget, const QPoint &hotSpot);

protected:
    void createDecoration(const QPoint &globalPos);
    void setupDecoration(const QPoint &globalPos);

private:
    DomUI *m_domUi;
    QWidget *m_widget;
    QWidget *m_decoration;
    QPoint m_hotSpot;

    bool m_ownWidget;
};
#endif // QDESIGNER_DNDITEM_H
