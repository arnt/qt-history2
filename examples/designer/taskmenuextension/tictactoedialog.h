#ifndef TICTACTOEDIALOG_H
#define TICTACTOEDIALOG_H

#include <QDialog>

class QHBoxLayout;
class QPushButton;
class TicTacToe;
class QVBoxLayout;

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

    QPushButton *cancelButton;
    QPushButton *okButton;
    QPushButton *resetButton;

    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

#endif
