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
#include <iconloader.h>

#include <QtGui/QPainter>
#include <QtGui/QFrame>
#include <QtGui/QKeyEvent>
#include <QtGui/QApplication>
#include <QtGui/QSpinBox>
#include <QtGui/QToolButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QLabel>

#include <QtGui/qdrawutil.h>
#include <QtCore/qdebug.h>
#include <limits.h>

#ifndef Q_MOC_RUN
using namespace qdesigner_internal;
#endif

class EditorWithReset : public QWidget
{
    Q_OBJECT
public:
    EditorWithReset(const QString &prop_name, QWidget *parent = 0);
    void setChildEditor(QWidget *child_editor);
    QWidget *childEditor() const { return m_child_editor; }
private slots:
    void emitResetProperty();
signals:
    void sync();
    void resetProperty(const QString &prop_name);
private:
    QWidget *m_child_editor;
    QHBoxLayout *m_layout;
    QString m_prop_name;
};

EditorWithReset::EditorWithReset(const QString &prop_name, QWidget *parent)
    : QWidget(parent)
{
    m_prop_name = prop_name;
    m_child_editor = 0;
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    QToolButton *button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIcon(createIconSet(QLatin1String("resetproperty.png")));
    button->setIconSize(QSize(8,8));
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
    m_layout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(emitResetProperty()));
}

void EditorWithReset::emitResetProperty()
{
    emit resetProperty(m_prop_name);
}

void EditorWithReset::setChildEditor(QWidget *child_editor)
{
    m_child_editor = child_editor;

    m_child_editor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    m_layout->insertWidget(0, m_child_editor);
    setFocusProxy(m_child_editor);
}

QPropertyEditorDelegate::QPropertyEditorDelegate(QObject *parent)
    : QItemDelegate(parent),
      m_readOnly(false)
{
}

QPropertyEditorDelegate::~QPropertyEditorDelegate()
{
}

bool QPropertyEditorDelegate::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if (!(ke->modifiers() & Qt::ControlModifier)
                && (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down)) {
                event->ignore();
                return true;
            }
            if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
                QWidget *widget = static_cast<QWidget*>(object);
                if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(widget)) { // ### hack (remove me)
                    spinBox->interpretText();
                }
                emit commitData(widget);
                return true;
            }
        } break;
        default:
            break;
    }

    QObject *editor = object;
    if (qobject_cast<EditorWithReset*>(editor->parent()) != 0)
        editor = editor->parent();
    return QItemDelegate::eventFilter(editor, event);
}

void QPropertyEditorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const QPropertyEditorModel*>(model)->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    if (property && property->isSeparator()) {
        option.palette.setColor(QPalette::Text, option.palette.color(QPalette::BrightText));
        option.font.setBold(true);
    }

    option.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);

    if (property->isSeparator()) {
        QBrush bg = option.palette.dark();
        painter->fillRect(option.rect, bg);
    } else if (opt.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(230, 230, 230));
    }

    painter->drawLine(option.rect.x(), option.rect.bottom(),
            option.rect.right(), option.rect.bottom());

    if (index.column() == 1 || !property->isSeparator()) {
        painter->drawLine(option.rect.right(), option.rect.y(),
                option.rect.right(), option.rect.bottom());
    }

    QItemDelegate::paint(painter, option, index);
}

QSize QPropertyEditorDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const QPropertyEditorModel*>(model)->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    option.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);

    QSize sz = QItemDelegate::sizeHint(option, index) + QSize(4, 4); // ####
    return sz;
}


bool QPropertyEditorDelegate::isReadOnly() const
{
    return m_readOnly;
}

void QPropertyEditorDelegate::setReadOnly(bool readOnly)
{
    // ### close the editor
    m_readOnly = readOnly;
}

QWidget *QPropertyEditorDelegate::createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    Q_UNUSED(option);

    const QPropertyEditorModel *model = static_cast<const QPropertyEditorModel*>(index.model());
    const IProperty *property = model->privateData(index);
    if (property == 0)
        return 0;

    QWidget *editor = 0;

    if (!isReadOnly() && property->hasEditor()) { // ### always true
        if (property->hasReset()) {
            EditorWithReset *editor_w_reset
                = new EditorWithReset(property->propertyName(), parent);
            QWidget *child_editor
                = property->createEditor(editor_w_reset, editor_w_reset, SIGNAL(sync()));
            editor_w_reset->setChildEditor(child_editor);
            connect(editor_w_reset, SIGNAL(sync()), this, SLOT(sync()));
            connect(editor_w_reset, SIGNAL(resetProperty(QString)),
                        model, SIGNAL(resetProperty(QString)));

            editor = editor_w_reset;
            child_editor->installEventFilter(const_cast<QPropertyEditorDelegate *>(this));
        } else {
            editor = property->createEditor(parent, this, SLOT(sync()));
            editor->installEventFilter(const_cast<QPropertyEditorDelegate *>(this));
        }
    }


    return editor;
}

void QPropertyEditorDelegate::setEditorData(QWidget *editor,
                             const QModelIndex &index) const
{
    if (EditorWithReset *editor_w_reset = qobject_cast<EditorWithReset*>(editor))
        editor = editor_w_reset->childEditor();

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const QPropertyEditorModel*>(model)->privateData(index);
    if (property && property->hasEditor()) {
        property->updateEditorContents(editor);
    }
}

void QPropertyEditorDelegate::setModelData(QWidget *editor,
                            QAbstractItemModel *model,
                            const QModelIndex &index) const
{
    if (EditorWithReset *editor_w_reset = qobject_cast<EditorWithReset*>(editor))
        editor = editor_w_reset->childEditor();

    if (IProperty *property = static_cast<const QPropertyEditorModel*>(model)->privateData(index)) {
        property->updateValue(editor);
        model->setData(index, property->value(), Qt::EditRole);
    }
}

void QPropertyEditorDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                              const QRect &rect, const QPixmap &pixmap) const
{
    QItemDelegate::drawDecoration(painter, option, rect, pixmap);
}

void QPropertyEditorDelegate::sync()
{
    QWidget *w = qobject_cast<QWidget*>(sender());
    if (w == 0)
        return;
    emit commitData(w);
}

void QPropertyEditorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QItemDelegate::updateEditorGeometry(editor, option, index);
    editor->setGeometry(editor->geometry().adjusted(0, 0, -1, -1));
}


#include "qpropertyeditor_delegate.moc"
