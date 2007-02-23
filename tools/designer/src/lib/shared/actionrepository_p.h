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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef ACTIONREPOSITORY_H
#define ACTIONREPOSITORY_H

#include "shared_global_p.h"
#include <QtCore/QMimeData>
#include <QtGui/QListWidget>

namespace qdesigner_internal {

class ResourceMimeData;

class QDESIGNER_SHARED_EXPORT ActionRepository: public QListWidget
{
    Q_OBJECT
public:
    enum   { ActionRole = Qt::UserRole + 1000 };

public:
    ActionRepository(QWidget *parent = 0);
    bool event ( QEvent * event );

signals:
    void contextMenuRequested(QContextMenuEvent *event, QListWidgetItem *item);
    void resourceImageDropped(const ResourceMimeData *data, QAction *action);

public slots:
    void filter(const QString &text);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual bool dropMimeData (int index, const QMimeData * data, Qt::DropAction action );
    virtual QMimeData *mimeData(const QList<QListWidgetItem*> items) const;
    virtual void focusInEvent(QFocusEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
};

class QDESIGNER_SHARED_EXPORT ActionRepositoryMimeData: public QMimeData
{
    Q_OBJECT
public:
    typedef QList<QAction*> ActionList;

    ActionRepositoryMimeData(const ActionList &);
    ActionRepositoryMimeData(QAction *);

    const ActionList &actionList() const { return m_actionList; }
    virtual QStringList formats() const;

private:
    ActionList m_actionList;
};
} // namespace qdesigner_internal

#endif // ACTIONREPOSITORY_H
