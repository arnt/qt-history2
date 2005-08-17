#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <QWidget>

class QSize;
class QRect;

class TicTacToe : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state WRITE setState)
public:
    TicTacToe(QWidget *parent = 0);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void setState(const QString &newState);
    QString state() const;
    void clearBoard();

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    enum {Empty = '-', Cross = 'X', Nought = 'O' };

    QRect cellRect(int row, int col) const;
    int cellWidth() const {return width()/ 3;}
    int cellHeight() const {return height()/ 3;}

    QString myState;
    int turnNumber;
};

#endif
