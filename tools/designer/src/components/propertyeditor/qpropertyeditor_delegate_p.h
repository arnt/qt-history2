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

#ifndef QPROPERTYEDITOR_DELEGATE_P_H
#define QPROPERTYEDITOR_DELEGATE_P_H

#include <qitemdelegate.h>

namespace QPropertyEditor
{

class Delegate : public QItemDelegate
{
    Q_OBJECT
public:
    Delegate(QObject *parent = 0);
    virtual ~Delegate();

    virtual bool eventFilter(QObject *object, QEvent *event);

    bool isReadOnly() const;
    void setReadOnly(bool readOnly);

//
// QItemDelegate Interface
//
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;

    virtual void setEditorData(QWidget *editor,
                               const QModelIndex &index) const;

    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const;
                              
public slots:
    void sync();

protected:
    virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QPixmap &pixmap) const;

private:
    bool m_readOnly;
};

} // namespace QPropertyEditor

#endif // QPROPERTYEDITOR_DELEGATE_P_H
