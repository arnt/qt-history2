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

#include "qabstractprintdialog.h"
#include "qabstractprintdialog_p.h"

/*!
    \class QAbstractPrintDialog
    \brief The QAbstractPrintDialog class provides a base implementation for
    print dialogs used to configure printers.
*/

/*!
    \enum QAbstractPrintDialog::PrintRange

    Used to specify the print range selection option.

    \value AllPages All pages should be printed.
    \value Selection Only the selection should be printed.
    \value PageRange The specified page range should be printed.
*/

/*!
    \enum QAbstractPrintDialog::PrintDialogOption

    Used to specify which parts of the print dialog should be enabled.

    \value None None of the options are enabled.
    \value PrintToFile The print to file option is enabled.
    \value PrintSelection The print selection option is enalbed.
    \value PrintPageRange The page range selection option is enabled.
    \value PrintCollateCopies
*/

/*!
    Constructs an abstract print dialog for \a printer with \a parent
    as parent widget
*/
QAbstractPrintDialog::QAbstractPrintDialog(QPrinter *printer, QWidget *parent)
    : QDialog(*(new QAbstractPrintDialogPrivate), parent)
{
    Q_D(QAbstractPrintDialog);
    d->printer = printer;
}

/*!
     \internal
*/
QAbstractPrintDialog::QAbstractPrintDialog(QAbstractPrintDialogPrivate &ptr,
                                           QPrinter *printer,
                                           QWidget *parent)
    : QDialog(ptr, parent)
{
    Q_D(QAbstractPrintDialog);
    d->printer = printer;
}


/*!
    Sets the set of options that should be enabled in the print dialog
    to \a options.
*/
void QAbstractPrintDialog::setEnabledOptions(PrintDialogOptions options)
{
    Q_D(QAbstractPrintDialog);
    d->options = options;
}

/*!
    Adds the option \a option to the set of enabled options in this dialog.
*/
void QAbstractPrintDialog::addEnabledOption(PrintDialogOption option)
{
    Q_D(QAbstractPrintDialog);
    d->options |= option;
}

/*!
    Returns the set of enabled options in this dialog.
*/
QAbstractPrintDialog::PrintDialogOptions QAbstractPrintDialog::enabledOptions() const
{
    Q_D(const QAbstractPrintDialog);
    return d->options;
}

/*!
    Returns true if the option \a option is enabled; otherwise returns false
*/
bool QAbstractPrintDialog::isOptionEnabled(PrintDialogOption option) const
{
    Q_D(const QAbstractPrintDialog);
    return d->options & option;
}

/*!
    Sets the print range option in to be \a range.
 */
void QAbstractPrintDialog::setPrintRange(PrintRange range)
{
    Q_D(QAbstractPrintDialog);
    d->printRange = range;
}

/*!
    Returns the print range.
*/
QAbstractPrintDialog::PrintRange QAbstractPrintDialog::printRange() const
{
    Q_D(const QAbstractPrintDialog);
    return d->printRange;
}

/*!
    Sets the page range in this dialog to be from \a min to \a max. This also
    enables the PrintPageRange option.
*/
void QAbstractPrintDialog::setMinMax(int min, int max)
{
    Q_D(QAbstractPrintDialog);
    Q_ASSERT_X(min <= max, "QAbstractPrintDialog::setMinMax",
               "'min' must be less than or equal to 'max'");
    d->minPage = min;
    d->maxPage = max;
    d->options |= PrintPageRange;
}

/*!
    Returns the minimum page in the page range.
*/
int QAbstractPrintDialog::minPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->minPage;
}

/*!
    Returns the maximum page in the page range.
*/
int QAbstractPrintDialog::maxPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->maxPage;
}

/*!
    Sets the range in the print dialog to be from \a from to \a to.
*/
void QAbstractPrintDialog::setFromTo(int from, int to)
{
    Q_D(QAbstractPrintDialog);
    Q_ASSERT_X(from <= to, "QAbstractPrintDialog::setFromTo",
               "'from' must be less than or equal to 'to'");
    d->fromPage = from;
    d->toPage = to;

    if (d->minPage == 0 && d->maxPage == 0)
        setMinMax(1, to);
}

/*!
    Returns the first page to be printed
*/
int QAbstractPrintDialog::fromPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->fromPage;
}

/*!
    Returns the last page to be printed.
*/
int QAbstractPrintDialog::toPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->toPage;
}

/*!
    Returns the printer that this printer dialog operates
    on.
*/
QPrinter *QAbstractPrintDialog::printer() const
{
    Q_D(const QAbstractPrintDialog);
    return d->printer;
}

/*!
    \fn int QAbstractPrintDialog::exec()

    This virtual function is called to pop up the dialog. It must be
    reimplemented in subclasses.
*/

/*!
    \class QPrintDialog qprintdialog.h

    \brief The QPrintDialog class provides a dialog for specifying
    the printer's configuration.

    \ingroup dialogs

    It encompasses both the sort of details needed for doing a simple
    print-out and some print configuration setup.

    Typical use of the QPrintDialog is to construct it on a QPrinter
    object and call exec() to execute it.

    \code
    QPrintDialog printDialog(printer, parent);
    if (printDialog.exec() == QDialog::Accept) {
        // print ...
    }
    \endcode

    The printer dialog in Motif style:

    \img qprintdlg-m.png
*/

/*!
    \fn QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)

    Constructs a new modal printer dialog for the given \a printer
    with the given \a parent.
*/

/*!
    \fn int QPrintDialog::exec()

    Launches the print dialog.

    \sa QDialog::DialogCode, accept(), reject()
*/
