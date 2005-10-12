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

#ifndef QABSTRACTPROXYMODEL_H
#define QABSTRACTPROXYMODEL_H

#include <QtCore/qabstractitemmodel.h>

QT_MODULE(Gui)

class QAbstractProxyModelPrivate;

class Q_GUI_EXPORT QAbstractProxyModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    QAbstractProxyModel(QObject *parent = 0);
    ~QAbstractProxyModel();

    virtual void setSourceModel(QAbstractItemModel *sourceModel);
    QAbstractItemModel *sourceModel() const;

    bool submit();
    void revert();

protected:
    QAbstractProxyModel(QAbstractProxyModelPrivate &, QObject *parent);
    
private:
    Q_DECLARE_PRIVATE(QAbstractProxyModel)
    Q_DISABLE_COPY(QAbstractProxyModel)
};

#endif // QABSTRACTPROXYMODEL_H
