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

#ifndef DCLOCK_H
#define DCLOCK_H

#include <qlcdnumber.h>


class DigitalClock : public QLCDNumber		// digital clock widget
{
    Q_OBJECT
public:
    DigitalClock( QWidget *parent=0, const char *name=0 );

protected:					// event handlers
    void	timerEvent( QTimerEvent * );
    void	mousePressEvent( QMouseEvent * );

private slots:					// internal slots
    void	stopDate();
    void	showTime();

private:					// internal data
    void	showDate();

    bool	showingColon;
    int		normalTimer;
    int		showDateTimer;
};


#endif // DCLOCK_H
