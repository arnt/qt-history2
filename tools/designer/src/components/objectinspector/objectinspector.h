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

#ifndef OBJECTINSPECTOR_H
#define OBJECTINSPECTOR_H

#include "objectinspector_global.h"
#include <QtDesigner/abstractobjectinspector.h>
#include <QPointer>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class ObjectItem;
class TreeWidget;

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public QDesignerObjectInspectorInterface
{
    Q_OBJECT
public:
    ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    virtual QDesignerFormEditorInterface *core() const;

    void setFormWindow(QDesignerFormWindowInterface *formWindow);

private slots:
    void slotSelectionChanged();

protected:
    virtual void showEvent(QShowEvent *enent);

private:
    static bool sortEntry(const QObject *a, const QObject *b);

private:
    QDesignerFormEditorInterface *m_core;
    TreeWidget *m_treeWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    ObjectItem *m_root;
    bool m_ignoreUpdate;
};


#endif // OBJECTINSPECTOR_H
