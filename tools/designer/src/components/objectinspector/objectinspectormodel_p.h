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

#ifndef OBJECTINSPECTORMODEL_H
#define OBJECTINSPECTORMODEL_H

#include <QtGui/QStandardItemModel>
#include <QtGui/QIcon>
#include <QtCore/QModelIndex>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMultiMap>
#include <QtCore/QPointer>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
    struct ObjectData {
        typedef QList<QStandardItem *> StandardItemList;

        ObjectData(QObject *parent = 0, QObject *object = 0);

        bool equals(const ObjectData & me) const;
        bool operator==(const ObjectData &e2) const { return equals(e2); }
        bool operator!=(const ObjectData &e2) const { return !equals(e2); }

        enum ChangedMask { ClassNameChanged = 1, ObjectNameChanged = 2, IconChanged = 4 };
        unsigned compare(const ObjectData & me) const;

        void setItems(const StandardItemList &row) const;
        void setItemsDisplayData(const StandardItemList &row, unsigned mask) const;

        QObject *m_parent;
        QObject *m_object;
        QString m_className;
        QString m_objectName;
        QIcon m_icon;
    };

    typedef QList<ObjectData> ObjectModel;

    // QStandardItemModel for ObjectInspector. Uses ObjectData/ObjectModel
    // internally for its updates.
    class ObjectInspectorModel : public QStandardItemModel {
    public:
        typedef QList<QStandardItem *> StandardItemList;
        enum { ObjectNameColumn, ClassNameColumn, NumColumns };

        explicit ObjectInspectorModel(QObject *parent);

        enum UpdateResult { NoForm, Rebuilt, Updated };
        UpdateResult update(QDesignerFormWindowInterface *fw);

        const QModelIndexList indexesOf(QObject *o) const { return m_objectIndexMultiMap.values(o); }
        QObject *objectAt(const QModelIndex &index) const;

        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    private:
        void rebuild(const ObjectModel &newModel);
        void updateItemContents(ObjectModel &oldModel, const ObjectModel &newModel);
        void clearItems();
        StandardItemList rowAt(QModelIndex index) const;

        typedef QMultiMap<QObject *,QModelIndex> ObjectIndexMultiMap;
        ObjectIndexMultiMap m_objectIndexMultiMap;
        ObjectModel m_model;
        QPointer<QDesignerFormWindowInterface> m_formWindow;
    };
}  // namespace qdesigner_internal

#endif // OBJECTINSPECTORMODEL_H
