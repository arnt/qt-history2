/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef VARIANTDELEGATE_H
#define VARIANTDELEGATE_H

#include <QItemDelegate>
#include <QRegExp>

class VariantDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    VariantDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    static bool isSupportedType(QVariant::Type type);
    static QString displayText(const QVariant &value);

private slots:
    void commitAndCloseEditor();

private:
    mutable QRegExp boolExp;
    mutable QRegExp byteArrayExp;
    mutable QRegExp charExp;
    mutable QRegExp colorExp;
    mutable QRegExp dateExp;
    mutable QRegExp dateTimeExp;
    mutable QRegExp doubleExp;
    mutable QRegExp pointExp;
    mutable QRegExp rectExp;
    mutable QRegExp signedIntegerExp;
    mutable QRegExp sizeExp;
    mutable QRegExp timeExp;
    mutable QRegExp unsignedIntegerExp;
};

#endif
