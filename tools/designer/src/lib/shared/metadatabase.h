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

#ifndef METADATABASE_H
#define METADATABASE_H

#include "shared_global.h"

#include <QtDesigner/abstractmetadatabase.h>

#include <QtCore/QHash>
#include <QtGui/QCursor>

class QT_SHARED_EXPORT MetaDataBaseItem: public QDesignerMetaDataBaseItemInterface
{
public:
    MetaDataBaseItem(QObject *object);
    virtual ~MetaDataBaseItem();

    virtual QString name() const;
    virtual void setName(const QString &name);

    virtual QString author() const;
    virtual void setAuthor(const QString &author);

    virtual QString comment() const;
    virtual void setComment(const QString &comment);

    virtual QCursor cursor() const;
    virtual void setCursor(const QCursor &cursor);

    virtual QList<QWidget*> tabOrder() const;
    virtual void setTabOrder(const QList<QWidget*> &tabOrder);

    virtual bool enabled() const;
    virtual void setEnabled(bool b);

private:
    QObject *m_object;
    QString m_author;
    QString m_comment;
    QCursor m_cursor;
    QList<QWidget*> m_tabOrder;
    bool m_enabled;
};

class QT_SHARED_EXPORT MetaDataBase: public QDesignerMetaDataBaseInterface
{
    Q_OBJECT
public:
    MetaDataBase(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~MetaDataBase();

    virtual QDesignerFormEditorInterface *core() const;

    virtual QDesignerMetaDataBaseItemInterface *item(QObject *object) const;
    virtual void add(QObject *object);
    virtual void remove(QObject *object);

    virtual QList<QObject*> objects() const;

private slots:
    void slotDestroyed(QObject *object);

private:
    QDesignerFormEditorInterface *m_core;
    typedef QHash<QObject *, MetaDataBaseItem*> ItemMap;
    ItemMap m_items;
};

#endif // METADATABASE_H
