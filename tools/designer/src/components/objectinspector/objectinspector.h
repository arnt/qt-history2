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
#include <abstractobjectinspector.h>
#include <QPointer>

class AbstractFormEditor;
class AbstractFormWindow;
class ObjectItem;
class TreeWidget;

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public AbstractObjectInspector
{
    Q_OBJECT
public:
    ObjectInspector(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    virtual AbstractFormEditor *core() const;

    void setFormWindow(AbstractFormWindow *formWindow);

private slots:
    void slotSelectionChanged();

protected:
    virtual void showEvent(QShowEvent *enent);

private:
    static bool sortEntry(const QObject *a, const QObject *b);

private:
    AbstractFormEditor *m_core;
    TreeWidget *m_treeWidget;
    QPointer<AbstractFormWindow> m_formWindow;
    ObjectItem *m_root;
    bool m_ignoreUpdate;
};


#endif // OBJECTINSPECTOR_H
