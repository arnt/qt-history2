#ifndef METADATABASE_H
#define METADATABASE_H

#include "formeditor_global.h"

#include <abstractmetadatabase.h>

#include <QHash>
#include <QCursor>

struct MetaDataBaseItem: public AbstractMetaDataBaseItem
{
    MetaDataBaseItem();
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

    virtual int spacing() const;
    virtual void setSpacing(int spacing);

    virtual int margin() const;
    virtual void setMargin(int margin);
    
    virtual bool enabled() const;
    virtual void setEnabled(bool b);

private:
    QString m_name;
    QString m_author;
    QString m_comment;
    QCursor m_cursor;
    QList<QWidget*> m_tabOrder;
    int m_spacing;
    int m_margin;
    bool m_enabled;
};

class QT_FORMEDITOR_EXPORT MetaDataBase: public AbstractMetaDataBase
{
    Q_OBJECT
public:
    MetaDataBase(AbstractFormEditor *core, QObject *parent = 0);
    virtual ~MetaDataBase();

    virtual AbstractFormEditor *core() const;

    virtual MetaDataBaseItem *item(QObject *object) const;
    virtual void add(QObject *object);
    virtual void remove(QObject *object);

    virtual QList<QObject*> objects() const;

private slots:
    void slotDestroyed(QObject *object);

private:
    AbstractFormEditor *m_core;
    typedef QHash<QObject *, MetaDataBaseItem*> ItemMap;
    ItemMap m_items;
};


#endif

