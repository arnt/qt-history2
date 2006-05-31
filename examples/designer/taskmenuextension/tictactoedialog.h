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

#ifndef TICTACTOEDIALOG_H
#define TICTACTOEDIALOG_H

#include <QDialog>

class QDialogButtonBox;
class TicTacToe;

class TicTacToeDialog : public QDialog
{
    Q_OBJECT

public:
    TicTacToeDialog(TicTacToe *plugin = 0, QWidget *parent = 0);

    QSize sizeHint() const;

private slots:
    void resetState();
    void saveState();

private:
    TicTacToe *editor;
    TicTacToe *ticTacToe;
    QDialogButtonBox *buttonBox;
};

#endif
