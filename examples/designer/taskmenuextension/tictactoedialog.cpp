/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include <QtDesigner>

#include "tictactoe.h"
#include "tictactoedialog.h"

TicTacToeDialog::TicTacToeDialog(TicTacToe *tic, QWidget *parent)
    : QDialog(parent)
{
    ticTacToe = tic;
    editor = new TicTacToe;
    editor->setState(ticTacToe->state());

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel
                                     | QDialogButtonBox::Reset);

    connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(resetState()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveState()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(editor);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Edit State"));
}

QSize TicTacToeDialog::sizeHint() const
{
    return QSize(250, 250);
}

void TicTacToeDialog::resetState()
{
    editor->clearBoard();
}

void TicTacToeDialog::saveState()
{
    if (QDesignerFormWindowInterface *formWindow
            = QDesignerFormWindowInterface::findFormWindow(ticTacToe)) {
        formWindow->cursor()->setProperty("state", editor->state());
    }
    accept();
}
