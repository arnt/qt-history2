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

#ifndef QPROPERTYEDITOR_H
#define QPROPERTYEDITOR_H

#include "propertyeditor_global.h"
#include "qpropertyeditor_items_p.h"

#include <QTreeView>

namespace QPropertyEditor
{

class Model;
class Delegate;

class QT_PROPERTYEDITOR_EXPORT View: public QTreeView
{
    Q_OBJECT
public:
    View(QWidget *parent = 0);
    ~View();

    IProperty *initialInput() const;
    bool isReadOnly() const;

    inline Model *editorModel() const
    { return m_model; }

signals:
    void propertyChanged(IProperty *property);

public slots:
    void setInitialInput(IProperty *initialInput);
    void setReadOnly(bool readOnly);

protected:
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    virtual void keyPressEvent(QKeyEvent *ev);

private:
    Model *m_model;
    Delegate *m_itemDelegate;
};

}; // namespace QPropertyEditor

#endif // QPROPERTYEDITOR_H
