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

#ifndef QPAGESETUPDIALOG_H
#define QPAGESETUPDIALOG_H

#include <QtGui/qabstractpagesetupdialog.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_PRINTDIALOG

class QPageSetupDialogPrivate;

class Q_GUI_EXPORT QPageSetupDialog : public QAbstractPageSetupDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPageSetupDialog)

public:
    enum PageSetupDialogOption {
        None                    = 0x0000,
        DontUseSheet            = 0x0001
    };

    Q_DECLARE_FLAGS(PageSetupDialogOptions, PageSetupDialogOption)

    explicit QPageSetupDialog(QPrinter *printer, QWidget *parent = 0);

    void addEnabledOption(PageSetupDialogOption option);
    void setEnabledOptions(PageSetupDialogOptions options);
    PageSetupDialogOptions enabledOptions() const;
    bool isOptionEnabled(PageSetupDialogOption option) const;

    virtual int exec();
#ifdef qdoc
    QPrinter *printer();
#endif
};

#endif // QT_NO_PRINTDIALOG

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPAGESETUPDIALOG_H
