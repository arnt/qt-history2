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

#ifndef QTBUTTONCONTAINERPROPERTYBROWSER_H
#define QTBUTTONCONTAINERPROPERTYBROWSER_H

#include "qtpropertybrowser.h"

class QtButtonContainerPropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
public:

    QtButtonContainerPropertyBrowser(QWidget *parent = 0);
    ~QtButtonContainerPropertyBrowser();

protected:
    virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem);
    virtual void itemRemoved(QtBrowserItem *item);
    virtual void itemChanged(QtBrowserItem *item);

private:

    class QtButtonContainerPropertyBrowserPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtButtonContainerPropertyBrowser)
    Q_DISABLE_COPY(QtButtonContainerPropertyBrowser)
    Q_PRIVATE_SLOT(d_func(), void slotUpdate())
    Q_PRIVATE_SLOT(d_func(), void slotEditorDestroyed())
    Q_PRIVATE_SLOT(d_func(), void slotToggled(bool))

};

#endif
