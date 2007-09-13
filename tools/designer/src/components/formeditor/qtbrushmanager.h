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

#ifndef QTBRUSHMANAGER_H
#define QTBRUSHMANAGER_H

#include <QtDesigner/QDesignerBrushManagerInterface>
#include "formeditor_global.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtGui/QBrush>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QtBrushManagerPrivate;

class QT_FORMEDITOR_EXPORT QtBrushManager : public QDesignerBrushManagerInterface
{
    Q_OBJECT
public:
    QtBrushManager(QObject *parent = 0);
    ~QtBrushManager();

    QBrush brush(const QString &name) const;
    QMap<QString, QBrush> brushes() const;
    QString currentBrush() const;

    QString addBrush(const QString &name, const QBrush &brush);
    void removeBrush(const QString &name);
    void setCurrentBrush(const QString &name);

    QPixmap brushPixmap(const QBrush &brush) const;
signals:
    void brushAdded(const QString &name, const QBrush &brush);
    void brushRemoved(const QString &name);
    void currentBrushChanged(const QString &name, const QBrush &brush);

private:
    QtBrushManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushManager)
    Q_DISABLE_COPY(QtBrushManager)
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
