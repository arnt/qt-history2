/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FontDisplayer_H
#define FontDisplayer_H

#include <qframe.h>
#include <qmainwindow.h>

class QSlider;

class FontRowTable : public QFrame {
    Q_OBJECT
public:
    FontRowTable( QWidget* parent=0, const char* name=0 );

    QSize sizeHint() const;

signals:
    void fontInformation(const QString&);

public slots:
    void setRow(int);
    void chooseFont();


protected:
    QSize cellSize() const;
    void paintEvent( QPaintEvent* );
private:
    QFont tablefont;
    int row;
};

class FontDisplayer : public QMainWindow {
    Q_OBJECT
public:
    FontDisplayer( QWidget* parent=0, const char* name=0 );
};

#endif
