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

#include <QPointer>
#include <QTreeWidget>

class AbstractFormEditor;
class AbstractFormWindow;
class ObjectItem;

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public QTreeWidget
{
    Q_OBJECT
public:
    ObjectInspector(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    virtual AbstractFormEditor *core() const;

public slots:
    void setFormWindow(AbstractFormWindow *formWindow);
    
private slots:
    void slotSelectionChanged();
    
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

private:
    AbstractFormEditor *m_core;
    QPointer<AbstractFormWindow> m_formWindow;
    ObjectItem *m_root;
    bool m_ignoreUpdate;
};


#endif // OBJECTINSPECTOR_H
