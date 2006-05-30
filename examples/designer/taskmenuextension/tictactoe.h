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

#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <QWidget>

class QRect;
class QSize;

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
    enum { Empty = '-', Cross = 'X', Nought = 'O' };

    QRect cellRect(int row, int col) const;
    int cellWidth() const { return width() / 3; }
    int cellHeight() const { return height() / 3; }

    QString myState;
    int turnNumber;
};

#endif
