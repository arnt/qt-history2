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

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#include "dialog.h"

CustomDialog::CustomDialog(Widget parent, Qt::WFlags flags)
    : QMotifDialog(parent, flags)
{
    setWindowTitle(tr("Custom Dialog"));

    QLabel *label = new QLabel(tr("<p><h3>Custom Dialog</h3></p>"
                                  "<p>This is a custom Qt-based dialog using "
                                  "QMotifDialog with a Motif-based parent.</p>"),
                               this);
    label->setAlignment(Qt::AlignCenter);

    QPushButton *button = new QPushButton(tr("OK"), this);
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    connect(button, SIGNAL(clicked()), this, SLOT(accept()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(button, 0, Qt::AlignCenter);

    setMinimumSize(minimumSizeHint());
}
