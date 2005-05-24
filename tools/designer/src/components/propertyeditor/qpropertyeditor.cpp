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

#include "qpropertyeditor.h"
#include "qpropertyeditor_model_p.h"
#include "qpropertyeditor_delegate_p.h"

#include <QtGui/QHeaderView>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <qdebug.h>

using namespace qdesigner_internal;

Q_GLOBAL_STATIC_WITH_ARGS(PropertyCollection, dummy_collection, (QLatin1String("<empty>")))

QPropertyEditor::QPropertyEditor(QWidget *parent)
    : QTreeView(parent)
{
    m_model = new QPropertyEditorModel(this);
    setModel(m_model);
    m_itemDelegate = new QPropertyEditorDelegate(this);
    setItemDelegate(m_itemDelegate);
    setInitialInput(0);

    setAlternatingRowColors(true);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(expand(QModelIndex)));

    connect(m_model, SIGNAL(propertyChanged(IProperty*)),
            this, SIGNAL(propertyChanged(IProperty*)));
}

QPropertyEditor::~QPropertyEditor()
{
}

bool QPropertyEditor::isReadOnly() const
{
    return m_itemDelegate->isReadOnly();
}

void QPropertyEditor::setReadOnly(bool readOnly)
{
    m_itemDelegate->setReadOnly(readOnly);
}

void QPropertyEditor::setInitialInput(IProperty *initialInput)
{
    if (!initialInput)
        initialInput = dummy_collection();

    m_model->setInitialInput(initialInput);

    setSelectionMode(QTreeView::SingleSelection);
    setSelectionBehavior(QTreeView::SelectRows);
    setRootIsDecorated(true);

    setEditTriggers(QAbstractItemView::CurrentChanged|QAbstractItemView::SelectedClicked);
    setRootIndex(m_model->indexOf(initialInput));

    resizeColumnToContents(0);
}


IProperty *QPropertyEditor::initialInput() const
{
    return m_model->initialInput();
}

void QPropertyEditor::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = viewOptions();
    QStyleOptionViewItem option = opt;

    IProperty *property = static_cast<const QPropertyEditorModel*>(model())->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    if (property && property->isSeparator()) {
        painter->fillRect(rect, option.palette.dark());
    } else if (selectionModel()->isSelected(index)) {
        painter->fillRect(rect, option.palette.brush(QPalette::Highlight));
    }

    if (model()->hasChildren(index)) {
        static const int size = 9;
        opt.state |= QStyle::State_Children;
        opt.rect.setRect(rect.width() - (indentation() + size) / 2,
                         rect.y() + (rect.height() - size) / 2, size, size);
        if (isExpanded(index))
            opt.state |= QStyle::State_Open;
        painter->fillRect(opt.rect, Qt::white);
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
    painter->drawLine(rect.x(), rect.bottom(), rect.right(), rect.bottom());
}

void QPropertyEditor::keyPressEvent(QKeyEvent *ev)
{
/*    QApplication::syncX();*/
    QTreeView::keyPressEvent(ev);
}
