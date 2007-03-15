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

#ifndef OBJECTINSPECTOR_H
#define OBJECTINSPECTOR_H

#include "objectinspector_global.h"
#include "qdesigner_objectinspector_p.h"

#include <QtCore/QPointer>
#include <QtCore/QList>
#include <QtCore/QSet>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QTreeWidgetItem;

namespace qdesigner_internal {

class TreeWidget;

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public QDesignerObjectInspector
{
    Q_OBJECT
public:
    explicit ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    virtual QDesignerFormEditorInterface *core() const;

    virtual void getSelection(Selection &s) const;
    virtual bool selectObject(QObject *o);
    virtual void clearSelection();

    void setFormWindow(QDesignerFormWindowInterface *formWindow);

    QWidget *widgetAt(const QPoint &global_mouse_pos);

private slots:
    void slotSelectionChanged();
    void slotPopupContextMenu(const QPoint &pos);
    void slotHeaderDoubleClicked(int column);

private:
    static bool sortEntry(const QObject *a, const QObject *b);
    void showContainersCurrentPage(QWidget *widget);

private:
    typedef QSet<const QObject *> PreviousSelection;
    PreviousSelection previousSelection(QDesignerFormWindowInterface *fw, bool formWindowChanged) const;

    typedef QList<QTreeWidgetItem *> ItemList;
    static void findRecursion(QTreeWidgetItem *item, QObject *o, ItemList &matchList);

    ItemList findItemsOfObject(QObject *o) const;

    QDesignerFormEditorInterface *m_core;
    TreeWidget *m_treeWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
};

}  // namespace qdesigner_internal

#endif // OBJECTINSPECTOR_H
