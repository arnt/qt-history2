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

#include <private/qabstractpagesetupdialog_p.h>

/*!
    \enum QPageSetupDialog::PageSetupDialogOption

    Used to specify options to the page setup dialog

    \value None None of the options are enabled.
    \value DontUseSheet Do not make the native print dialog a sheet. By default
    on Mac OS X, the native dialog is made a sheet if it has a parent that can
    accept sheets and is visible. Internally, Mac OS X tracks whether
    a printing <em>session</em> and not which particular dialog should have a sheet or not.
    Therefore, make sure this value matches between the page setup dialog and
    the print dialog or you can potentially end up in a modal loop that you can't break.
*/

/*!
    Adds the option \a option to the set of enabled options in this dialog.
*/
void QPageSetupDialog::addEnabledOption(PageSetupDialogOption option)
{
    reinterpret_cast<QAbstractPageSetupDialogPrivate *>(d_func())->addEnabledOption(option);
}

/*!
    Sets the set of options that should be enabled for the page setup dialog
    to \a options.
*/
void QPageSetupDialog::setEnabledOptions(PageSetupDialogOptions options)
{
    reinterpret_cast<QAbstractPageSetupDialogPrivate *>(d_func())->setEnabledOptions(options);
}

/*!
    Returns the set of enabled options in this dialog.
*/
QPageSetupDialog::PageSetupDialogOptions QPageSetupDialog::enabledOptions() const
{
    return reinterpret_cast<const QAbstractPageSetupDialogPrivate *>(d_func())->enabledOptions();
}

/*!
    Returns true if the specified \a option is enabled; otherwise returns false
*/
bool QPageSetupDialog::isOptionEnabled(PageSetupDialogOption option) const
{
    return reinterpret_cast<const QAbstractPageSetupDialogPrivate *>(d_func())->isOptionEnabled(option);
}
