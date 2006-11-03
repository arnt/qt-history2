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

#ifndef METADATABASE_H
#define METADATABASE_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerMetaDataBaseInterface>

#include <QtCore/QHash>
#include <QtGui/QCursor>

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT MetaDataBaseItem: public QDesignerMetaDataBaseItemInterface
{
public:
    MetaDataBaseItem(QObject *object);
    virtual ~MetaDataBaseItem();

    virtual QString name() const;
    virtual void setName(const QString &name);

    virtual TabOrder tabOrder() const;
    virtual void setTabOrder(const TabOrder &tabOrder);

    virtual bool enabled() const;
    virtual void setEnabled(bool b);
    
    virtual QString customClassName() const;
    virtual void setCustomClassName(const QString &customClassName);

    virtual QString propertyComment(const QString &name) const;
    virtual void setPropertyComment(const QString &name, const QString &comment);

    typedef QHash<QString, QString> PropertyComments;
    
    const PropertyComments &comments() const { return m_comments; }

private:
    QObject *m_object;
    TabOrder m_tabOrder;
    PropertyComments m_comments;
    bool m_enabled;
    QString m_customClassName;
};

class QDESIGNER_SHARED_EXPORT MetaDataBase: public QDesignerMetaDataBaseInterface
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

    void dump();

private slots:
    void slotDestroyed(QObject *object);

private:
    QDesignerFormEditorInterface *m_core;
    typedef QHash<QObject *, MetaDataBaseItem*> ItemMap;
    ItemMap m_items;
};
    
    // promotion convenience
    bool promoteWidget(QDesignerFormEditorInterface *core,QWidget *widget,const QString &customClassName);
    void demoteWidget(QDesignerFormEditorInterface *core,QWidget *widget); 
    bool isPromoted(QDesignerFormEditorInterface *core, QWidget* w);
    QString promotedCustomClassName(QDesignerFormEditorInterface *core, QWidget* w);
    QString promotedExtends(QDesignerFormEditorInterface *core, QWidget* w);
    

} // namespace qdesigner_internal

#endif // METADATABASE_H
