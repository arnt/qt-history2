/****************************************************************************
**
** Definition of QAbstractPrintDialogPrivate class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractprintdialog.h"
#include "qabstractprintdialog_p.h"

#define d d_func()

QAbstractPrintDialog::QAbstractPrintDialog(QPrinter *printer, QWidget *parent)
    : QDialog(*(new QAbstractPrintDialogPrivate), parent)
{
    d->printer = printer;
}

QAbstractPrintDialog::QAbstractPrintDialog(QAbstractPrintDialogPrivate &ptr,
                                           QPrinter *printer,
                                           QWidget *parent)
    : QDialog(ptr, parent)
{
    d->printer = printer;
}

void QAbstractPrintDialog::setEnabledOptions(PrintDialogOptions options)
{
    d->options = options;
}

void QAbstractPrintDialog::addEnabledOption(PrintDialogOption option)
{
    d->options |= option;
}

QAbstractPrintDialog::PrintDialogOptions QAbstractPrintDialog::enabledOptions() const
{
    return d->options;
}

bool QAbstractPrintDialog::isOptionEnabled(PrintDialogOption option) const
{
    return d->options & option;
}

void QAbstractPrintDialog::setPageRange(PageRange range)
{
    d->pageRange = range;
}

QAbstractPrintDialog::PageRange QAbstractPrintDialog::pageRange() const
{
    return d->pageRange;
}

void QAbstractPrintDialog::setMinMax(int min, int max)
{
    Q_ASSERT_X(min <= max, "QAbstractPrintDialog::setMinMax",
               "'min' must be less than or equal to 'max'");
    d->minPage = min;
    d->maxPage = max;
    d->options |= PrintPageRange;
}

int QAbstractPrintDialog::minPage() const
{
    return d->minPage;
}

int QAbstractPrintDialog::maxPage() const
{
    return d->maxPage;
}

void QAbstractPrintDialog::setFromTo(int from, int to)
{
    Q_ASSERT_X(from <= to, "QAbstractPrintDialog::setFromTo",
               "'from' must be less than or equal to 'to'");
    d->fromPage = from;
    d->toPage = to;

    if (d->minPage == 0 && d->maxPage == 0)
        setMinMax(1, to);
}

int QAbstractPrintDialog::fromPage() const
{
    return d->fromPage;
}

int QAbstractPrintDialog::toPage() const
{
    return d->toPage;
}

QPrinter *QAbstractPrintDialog::printer() const
{
    return d->printer;
}
