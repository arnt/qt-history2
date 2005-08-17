#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include <QtGui>

#include "tictactoedialog.h"
#include "tictactoe.h"

TicTacToeDialog::TicTacToeDialog(TicTacToe *tic, QWidget *parent)
    : QDialog(parent)
{
    ticTacToe = tic;
    editor = new TicTacToe(this);
    editor->setState(ticTacToe->state());

    cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    okButton = new QPushButton(tr("OK"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(saveState()));

    resetButton = new QPushButton(tr("Reset"));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetState()));

    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout();
    mainLayout->addWidget(editor);
    mainLayout->addLayout(buttonLayout);

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
