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

#include "treewalker.h"
#include "ui4.h"

void TreeWalker::accept(DomUI *ui)
{
    accept(ui->elementWidget());
    accept(ui->elementTabStops());

    if (ui->elementImages())
        accept(ui->elementImages());
}

void TreeWalker::accept(DomLayoutDefault *layoutDefault)
{
    Q_UNUSED(layoutDefault);
}

void TreeWalker::accept(DomLayoutFunction *layoutFunction)
{
    Q_UNUSED(layoutFunction);
}

void TreeWalker::accept(DomTabStops *tabStops)
{
    Q_UNUSED(tabStops);
}

void TreeWalker::accept(DomLayout *layout)
{
    for (int i=0; i<layout->elementProperty().size(); ++i)
        accept(layout->elementProperty().at(i));

    for (int i=0; i<layout->elementItem().size(); ++i)
        accept(layout->elementItem().at(i));
}

void TreeWalker::accept(DomLayoutItem *layoutItem)
{
    switch (layoutItem->kind()) {
        case DomLayoutItem::Widget:
            accept(layoutItem->elementWidget());
            return;
        case DomLayoutItem::Layout:
            accept(layoutItem->elementLayout());
            return;
        case DomLayoutItem::Spacer:
            accept(layoutItem->elementSpacer());
            return;
        case DomLayoutItem::Unknown:
            break;
    }

    Q_ASSERT( 0 );
}

void TreeWalker::accept(DomWidget *widget)
{
    for (int i=0; i<widget->elementAction().size(); ++i)
        accept(widget->elementAction().at(i));

    for (int i=0; i<widget->elementActionGroup().size(); ++i)
        accept(widget->elementActionGroup().at(i));

    for (int i=0; i<widget->elementAddAction().size(); ++i)
        accept(widget->elementAddAction().at(i));

    for (int i=0; i<widget->elementProperty().size(); ++i)
        accept(widget->elementProperty().at(i));

    for (int i=0; i<widget->elementWidget().size(); ++i)
        accept(widget->elementWidget().at(i));

    if (!widget->elementLayout().isEmpty())
        accept(widget->elementLayout().at(0));
}

void TreeWalker::accept(DomSpacer *spacer)
{
    for (int i=0; i<spacer->elementProperty().size(); ++i)
        accept(spacer->elementProperty().at(i));
}

void TreeWalker::accept(DomColor *color)
{
    Q_UNUSED(color);
}

void TreeWalker::accept(DomColorGroup *colorGroup)
{
    Q_UNUSED(colorGroup);
}

void TreeWalker::accept(DomPalette *palette)
{
    accept(palette->elementActive());
    accept(palette->elementInactive());
    accept(palette->elementDisabled());
}

void TreeWalker::accept(DomFont *font)
{
    Q_UNUSED(font);
}

void TreeWalker::accept(DomPoint *point)
{
    Q_UNUSED(point);
}

void TreeWalker::accept(DomRect *rect)
{
    Q_UNUSED(rect);
}

void TreeWalker::accept(DomSizePolicy *sizePolicy)
{
    Q_UNUSED(sizePolicy);
}

void TreeWalker::accept(DomSize *size)
{
    Q_UNUSED(size);
}

void TreeWalker::accept(DomDate *date)
{
    Q_UNUSED(date);
}

void TreeWalker::accept(DomTime *time)
{
    Q_UNUSED(time);
}

void TreeWalker::accept(DomDateTime *dateTime)
{
    Q_UNUSED(dateTime);
}

void TreeWalker::accept(DomProperty *property)
{
    switch (property->kind()) {
        case DomProperty::Bool:
        case DomProperty::Color:
        case DomProperty::Cstring:
        case DomProperty::Cursor:
        case DomProperty::Enum:
        case DomProperty::Font:
        case DomProperty::Pixmap:
        case DomProperty::IconSet:
        case DomProperty::Palette:
        case DomProperty::Point:
        case DomProperty::Rect:
        case DomProperty::Set:
        case DomProperty::SizePolicy:
        case DomProperty::Size:
        case DomProperty::String:
        case DomProperty::Number:
        case DomProperty::Date:
        case DomProperty::Time:
        case DomProperty::DateTime:
        case DomProperty::Unknown:
        case DomProperty::StringList:
            break;
    }
}

void TreeWalker::accept(DomCustomWidgets *customWidgets)
{
    for (int i=0; i<customWidgets->elementCustomWidget().size(); ++i)
        accept(customWidgets->elementCustomWidget().at(i));
}

void TreeWalker::accept(DomCustomWidget *customWidget)
{
    Q_UNUSED(customWidget);
}

void TreeWalker::accept(DomAction *action)
{
    Q_UNUSED(action);
}

void TreeWalker::accept(DomActionGroup *actionGroup)
{
    for (int i=0; i<actionGroup->elementAction().size(); ++i)
        accept(actionGroup->elementAction().at(i));

    for (int i=0; i<actionGroup->elementActionGroup().size(); ++i)
        accept(actionGroup->elementActionGroup().at(i));
}

void TreeWalker::accept(DomActionRef *actionRef)
{
    Q_UNUSED(actionRef);
}

void TreeWalker::accept(DomImages *images)
{
    for (int i=0; i<images->elementImage().size(); ++i)
        accept(images->elementImage().at(i));
}

void TreeWalker::accept(DomImage *image)
{
    Q_UNUSED(image);
}

void TreeWalker::accept(DomIncludes *includes)
{
    for (int i=0; i<includes->elementInclude().size(); ++i)
        accept(includes->elementInclude().at(i));
}

void TreeWalker::accept(DomInclude *incl)
{
    Q_UNUSED(incl);
}
