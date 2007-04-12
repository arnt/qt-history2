/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef COLORS_H
#define COLORS_H

#include <QtGui>
#include <QBrush>

class Colors
{
private:
    Colors(){};

public:
    static void parseArgs(int argc, char *argv[]);
    static void adaptAccordingToEnvironment();

    // Colors:
    static QColor sceneBg1;
    static QColor sceneBg2;
    static QColor sceneBg1Line;
    static QColor paperBg;
    static QColor menuTextFg;
    static QColor buttonText;
    static QColor buttonBgLow;
    static QColor buttonBgHigh;
    static QColor tt_green;
    static QColor fadeOut;
    static QColor sceneLine;
    static QColor heading;
    static QString contentColor;

    // Guides:
    static int stageStartY;
    static int stageHeight;
    static int stageStartX;
    static int stageWidth;
    static int contentStartY;
    static int contentHeight;

    // properties:
    static bool noOpenGl;
    static bool noAdapt;
    static bool noTicker;
    static bool noRescale;
    static bool noAnimations;
    static bool noBlending;
    static bool noScreenSync;
    static bool useLoop;
    static bool noWindowMask;
    static bool usePixmaps;
    static bool useEightBitPalette;
    static bool fullscreen;
    static bool showBoundingRect;
    static bool showFps;
    static bool low;
    static bool noTimerUpdate;
    static bool noTickerMorph;

    static float animSpeed;
    static float animSpeedButtons;
    static int tickerLetterCount;
    static int fps;
    static float tickerMoveSpeed;
    static float tickerMorphSpeed;
    static QString tickerText;

    // fonts
    static QFont contentFont();
    static QFont headingFont();
    static QFont buttonFont();
    static QFont tickerFont();

};

#endif // COLORS_H

