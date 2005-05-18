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

#ifndef DIALOG_H
#define DIALOG_H

#include <QtMotif/QMotifDialog>

class CustomDialog : public QMotifDialog
{
    Q_OBJECT

public:
    CustomDialog(Widget parent, Qt::WFlags flags = 0);
};

#endif // DIALOG_H
