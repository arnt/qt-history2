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
#ifndef PLAY_H
#define PLAY_H

#include "qsound.h"
#include <qmainwindow.h>

class SoundPlayer : public QMainWindow {
    Q_OBJECT
public:
    SoundPlayer();

public slots:
    void doPlay1();
    void doPlay2();
    void doPlay3();
    void doPlay4();
    void doPlay34();
    void doPlay1234();

private:
    QSound bucket3;
    QSound bucket4;
};

#endif
