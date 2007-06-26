/****************************************************************************
**
** copyright (c) 1992-$thisyear$ $trolltech$. all rights reserved.
**
** this file is part of the $module$ of the qt toolkit.
**
** $trolltech_dual_license$
**
** this file is provided as is with no warranty of any kind, including the
** warranty of design, merchantability and fitness for a particular purpose.
**
****************************************************************************/

#ifndef QTGROUPBOXPROPERTYBROWSER_H
#define QTGROUPBOXPROPERTYBROWSER_H

#include "qtpropertybrowser.h"

class QT_QTPROPERTYBROWSER_EXPORT QtGroupBoxPropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
public:

    QtGroupBoxPropertyBrowser(QWidget *parent = 0);
    ~QtGroupBoxPropertyBrowser();

protected:
    virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem);
    virtual void itemRemoved(QtBrowserItem *item);
    virtual void itemChanged(QtBrowserItem *item);

private:

    class QtGroupBoxPropertyBrowserPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGroupBoxPropertyBrowser)
    Q_DISABLE_COPY(QtGroupBoxPropertyBrowser)
    Q_PRIVATE_SLOT(d_func(), void slotUpdate())
    Q_PRIVATE_SLOT(d_func(), void slotEditorDestroyed())

};

#endif
