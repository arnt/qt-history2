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

#ifndef DEFAULT_LAYOUTDECORATION_H
#define DEFAULT_LAYOUTDECORATION_H

#include "formeditor_global.h"
#include "layoutdecoration.h"
#include <default_extensionfactory.h>

class QLayoutWidget;
class QLayoutSupport;
class FormWindow;

class QT_FORMEDITOR_EXPORT QDesignerLayoutDecoration: public QObject, public ILayoutDecoration
{
    Q_OBJECT
    Q_INTERFACES(ILayoutDecoration)
public:
    QDesignerLayoutDecoration(QLayoutWidget *widget, QObject *parent = 0);
    QDesignerLayoutDecoration(FormWindow *formWindow, QWidget *widget, QObject *parent = 0);

    virtual InsertMode currentInsertMode() const;
    virtual int currentIndex() const;
    virtual QPair<int, int> currentCell() const;
    virtual void insertWidget(QWidget *widget);
    virtual void removeWidget(QWidget *widget);

    virtual int findItemAt(const QPoint &pos) const;
    virtual void adjustIndicator(const QPoint &pos, int index);

private:
    QLayoutSupport *m_layoutSupport;
};

class QT_FORMEDITOR_EXPORT QDesignerLayoutDecorationFactory: public DefaultExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    QDesignerLayoutDecorationFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // DEFAULT_LAYOUTDECORATION_H
