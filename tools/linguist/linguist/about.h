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

#ifndef ABOUT_H
#define ABOUT_H

#include <qvariant.h>
#include "ui_about.h"


class AboutDialog : public QDialog, public Ui::AboutDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~AboutDialog() {}

protected slots:
    virtual void languageChange();
};

#endif // ABOUT_H
