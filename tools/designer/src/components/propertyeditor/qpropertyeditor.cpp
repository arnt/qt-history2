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

#include <qitemdelegate.h>
#include <qheaderview.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qscrollbar.h>

using namespace QPropertyEditor;

Q_GLOBAL_STATIC_WITH_ARGS(PropertyCollection, dummy_collection, (QLatin1String("<empty>")))

View::View(QWidget *parent)
    : QTreeView(parent)
{
    m_model = new Model(this);
    setModel(m_model);
    m_itemDelegate = new Delegate(this);
    setItemDelegate(m_itemDelegate);
    setInitialInput(0);

    setAlternatingRowColors(true);
    setEvenRowColor(QColor(237, 243, 254));
    setOddRowColor(QColor(255, 255, 255));

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(this, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(expand(const QModelIndex&)));

    connect(m_model, SIGNAL(propertyChanged(IProperty*)),
            this, SIGNAL(propertyChanged(IProperty*)));
}

View::~View()
{
}

bool View::isReadOnly() const
{
    return m_itemDelegate->isReadOnly();
}

void View::setReadOnly(bool readOnly)
{
    m_itemDelegate->setReadOnly(readOnly);
}

void View::setInitialInput(IProperty *initialInput)
{
    if (!initialInput)
        initialInput = dummy_collection();

    m_model->setInitialInput(initialInput);

    setSelectionMode(QTreeView::SingleSelection);
    setSelectionBehavior(QTreeView::SelectRows);
    setRootIsDecorated(true);

    setEditTriggers(QAbstractItemView::CurrentChanged|QAbstractItemView::SelectedClicked);
    setRootIndex(m_model->indexOf(initialInput));

    header()->setResizeMode(QHeaderView::Stretch, 1);
    resizeColumnToContents(0);
}


IProperty *View::initialInput() const
{
    return m_model->initialInput();
}

void View::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = viewOptions();
    QStyleOptionViewItem option = opt;

    IProperty *property = static_cast<const Model*>(model())->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    if (selectionModel()->isSelected(index))
        painter->fillRect(rect, QColor(230, 230, 230));

    painter->drawLine(rect.x(), rect.bottom(),
                      rect.right(), rect.bottom());

    if (model()->hasChildren(index)) {
        static const int size = 9;
        opt.state |= QStyle::State_Children;
        opt.rect.setRect(rect.width() - (indentation() + size) / 2,
                         rect.y() + (rect.height() - size) / 2, size, size);
        if (isExpanded(index))
            opt.state |= QStyle::State_Open;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
}

void View::keyPressEvent(QKeyEvent *ev)
{
/*    QApplication::syncX();*/
    QTreeView::keyPressEvent(ev);
}
