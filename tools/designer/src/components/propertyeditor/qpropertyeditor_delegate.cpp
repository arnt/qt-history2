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

#include "qpropertyeditor_delegate_p.h"
#include "qpropertyeditor_model_p.h"

#include <QPainter>
#include <QFrame>
#include <QKeyEvent>
#include <QApplication>
#include <QSpinBox>

#include <qdrawutil.h>
#include <qdebug.h>
#include <limits.h>

using namespace QPropertyEditor;

Delegate::Delegate(QObject *parent)
    : QItemDelegate(parent),
      m_readOnly(false)
{
}

Delegate::~Delegate()
{
}

bool Delegate::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if (!(ke->modifiers() & Qt::ControlModifier)
                && (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down)) {
                QWidget *p = static_cast<QWidget*>(object)->parentWidget();
                QApplication::sendEvent(p, ke);
                return true;
            }
            if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
                QWidget *widget = static_cast<QWidget*>(object);
                if (QSpinBox *spinBox = qt_cast<QSpinBox*>(widget)) { // ### hack (remove me)
                    spinBox->interpretText();
                }
                emit commitData(widget);
                return true;
            }
        } break; 
        default:
            break;
    }

    return QItemDelegate::eventFilter(object, event);
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &opt,
                     const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const Model*>(model)->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    option.state &= ~(QStyle::Style_Selected | QStyle::Style_HasFocus);

    if (opt.state & QStyle::Style_Selected)
        painter->fillRect(option.rect, QColor(230, 230, 230));

    painter->drawLine(option.rect.x(), option.rect.bottom(),
                      option.rect.right(), option.rect.bottom());

    painter->drawLine(option.rect.right(), option.rect.y(),
                      option.rect.right(), option.rect.bottom());

    QItemDelegate::paint(painter, option, index);
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &opt,
                         const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const Model*>(model)->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    option.state &= ~(QStyle::Style_Selected | QStyle::Style_HasFocus);

    return QItemDelegate::sizeHint(option, index) + QSize(4,4);
}


bool Delegate::isReadOnly() const
{
    return m_readOnly;
}

void Delegate::setReadOnly(bool readOnly)
{
    // ### close the editor
    m_readOnly = readOnly;
}

QWidget *Delegate::editor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index)
{
    Q_UNUSED(option);

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const Model*>(model)->privateData(index);
    if (!isReadOnly() && property && property->hasEditor()) { // ### always true
        QWidget *editor = property->createEditor(parent, this, SLOT(sync()));
        Q_ASSERT(editor);

        editor->installEventFilter(this);
        return editor;
    }

    return 0;
}

void Delegate::setEditorData(QWidget *editor,
                             const QModelIndex &index) const
{
    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const Model*>(model)->privateData(index);
    if (property && property->hasEditor()) {
        property->updateEditorContents(editor);
    }
}

void Delegate::setModelData(QWidget *editor,
                            QAbstractItemModel *model,
                            const QModelIndex &index) const
{
    if (IProperty *property = static_cast<const Model*>(model)->privateData(index)) {
        property->updateValue(editor);
        model->setData(index, property->value(), Model::EditRole);
    }
}

void Delegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                              const QRect &rect, const QPixmap &pixmap) const
{
    QItemDelegate::drawDecoration(painter, option, rect, pixmap);
}

void Delegate::sync()
{
    QWidget *w = static_cast<QWidget*>(sender());
    emit commitData(w);
}
