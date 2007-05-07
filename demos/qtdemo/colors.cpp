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

#include "colors.h"

#ifndef QT_NO_OPENGL    
    #include <QGLWidget>
#endif
//#define QT_NO_OPENGL

// Colors:
QColor Colors::sceneBg1(QColor(91, 91, 91));
QColor Colors::sceneBg1Line(QColor(114, 108, 104));
QColor Colors::sceneBg2(QColor(0, 0, 0));
QColor Colors::sceneLine(255, 255, 255);
QColor Colors::paperBg(QColor(100, 100, 100));
QColor Colors::menuTextFg(QColor(255, 0, 0));
QColor Colors::buttonBgLow(QColor(255, 255, 255, 90));
QColor Colors::buttonBgHigh(QColor(255, 255, 255, 20));
QColor Colors::buttonText(QColor(255, 255, 255));
QColor Colors::tt_green(QColor(166, 206, 57));
QColor Colors::fadeOut(QColor(206, 246, 117, 0));
QColor Colors::heading(QColor(190,230,80));
QString Colors::contentColor("<font color='#eeeeee'>");
QString Colors::glVersion("Not detected!");

// Guides:
int Colors::stageStartY = 8;
int Colors::stageHeight = 536;
int Colors::stageStartX = 8;
int Colors::stageWidth = 785;
int Colors::contentStartY = 22;
int Colors::contentHeight = 510;

// Properties:
bool Colors::noOpenGl = false;
bool Colors::noDirect3d = false;
bool Colors::noTicker = false;
bool Colors::noRescale = false;
bool Colors::noAnimations = false;
bool Colors::noBlending = false;
bool Colors::noScreenSync = false;
bool Colors::fullscreen = false;
bool Colors::usePixmaps = false;
bool Colors::useLoop = false;
bool Colors::showBoundingRect = false;
bool Colors::showFps = false;
bool Colors::noAdapt = false;
bool Colors::noWindowMask = true;
bool Colors::useButtonBalls = false;
bool Colors::low = false;
bool Colors::useEightBitPalette = false;
bool Colors::noTimerUpdate = false;
bool Colors::noTickerMorph = false;
bool Colors::adapted = false;
bool Colors::verbose = false;
int Colors::fps = 100;
float Colors::animSpeed = 1.0;
float Colors::animSpeedButtons = 1.0;
float Colors::benchmarkFps = -1;
int Colors::tickerLetterCount = 80;
float Colors::tickerMoveSpeed = 0.4f;
float Colors::tickerMorphSpeed = 2.5f;
QString Colors::tickerText = ".EROM ETAERC .SSEL EDOC";

QFont Colors::contentFont()
{
    QFont font;
    font.setStyleStrategy(QFont::PreferAntialias);
#if defined(Q_OS_MAC)
    font.setPixelSize(14);
    font.setFamily("Arial");
#else
    font.setPixelSize(13);
    font.setFamily("Verdana");
#endif
    return font;
}

QFont Colors::headingFont()
{
    QFont font;
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setPixelSize(23);
    font.setBold(true);
    font.setFamily("Verdana");
    return font;
}

QFont Colors::buttonFont()
{
    QFont font;
    font.setStyleStrategy(QFont::PreferAntialias);
#if 0//defined(Q_OS_MAC)
    font.setPixelSize(11);
    font.setFamily("Silom");
#else
    font.setPixelSize(11);
    font.setFamily("Verdana");
#endif    
    return font;
}

QFont Colors::tickerFont()
{
    QFont font;
    font.setStyleStrategy(QFont::PreferAntialias);
#if defined(Q_OS_MAC)
    font.setPixelSize(11);
    font.setBold(true);
    font.setFamily("Arial");
#else
    font.setPixelSize(10);
    font.setBold(true);
    font.setFamily("sans serif");
#endif    
    return font;
}

float parseFloat(const QString &argument, const QString &name)
{
    if (name.length() == argument.length()){
        QMessageBox::warning(0, "Arguments",
                                 QString("No argument number found for ")
                                 + name
                                 + ". Remember to put name and value adjacent! (e.g. -fps100)");
        exit(0);
    }
    float value = argument.mid(name.length()).toFloat();
    return value;
}

QString parseText(const QString &argument, const QString &name)
{
    if (name.length() == argument.length()){
        QMessageBox::warning(0, "Arguments",
                                 QString("No argument number found for ")
                                 + name
                                 + ". Remember to put name and value adjacent! (e.g. -fps100)");
        exit(0);
    }
    QString value = argument.mid(name.length());
    return value;
}

void Colors::parseArgs(int argc, char *argv[])
{
    // some arguments should be processed before
    // others. Handle them now:
    for (int i=1; i<argc; i++){
        QString s(argv[i]);
        if (s == "-no-adapt")
             Colors::noAdapt = true;
        else if (s == "-low")
             Colors::low = true;
        else if (s == "-verbose")
            Colors::verbose = true;
    }

    Colors::adaptAccordingToEnvironment();

    // Handle the rest of the arguments. They may
    // override attributes already set:
    for (int i=1; i<argc; i++){
        QString s(argv[i]);
        if (s == "-no-opengl")
            Colors::noOpenGl = true;
        else if (s == "-no-direct3d")
            Colors::noDirect3d = true;
        else if (s == "-no-ticker")
            Colors::noTicker = true;
        else if (s == "-no-rescale")
            Colors::noRescale = true;
        else if (s == "-use-pixmaps")
            Colors::usePixmaps = true;
        else if (s == "-fullscreen")
            Colors::fullscreen = true;
        else if (s == "-show-br")
            Colors::showBoundingRect = true;
        else if (s == "-show-fps")
            Colors::showFps = true;
        else if (s == "-no-animations")
            Colors::noAnimations = true;
        else if (s == "-no-blending")
            Colors::noBlending = true;
        else if (s == "-no-sync")
            Colors::noScreenSync = true;
        else if (s.startsWith("-use-timer-update"))
            Colors::noTimerUpdate = !bool(parseFloat(s, "-use-timer-update"));
        else if (s == "-no-ticker-morph")
            Colors::noTickerMorph = true;
        else if (s == "-use-window-mask")
            Colors::noWindowMask = false;
        else if (s == "-use-loop")
            Colors::useLoop = true;
        else if (s == "-use-8bit")
            Colors::useEightBitPalette = true;
        else if (s == "-use-balls")
            Colors::useButtonBalls = true;
        else if (s.startsWith("-ticker-letters"))
            Colors::tickerLetterCount = int(parseFloat(s, "-ticker-letters"));
        else if (s.startsWith("-ticker-text"))
            Colors::tickerText = parseText(s, "-ticker-text");
        else if (s.startsWith("-ticker-speed"))
            Colors::tickerMoveSpeed = parseFloat(s, "-ticker-speed");
        else if (s.startsWith("-ticker-morph-speed"))
            Colors::tickerMorphSpeed = parseFloat(s, "-ticker-morph-speed");
        else if (s.startsWith("-animation-speed"))
            Colors::animSpeed = parseFloat(s, "-animation-speed");
        else if (s.startsWith("-fps"))
            Colors::fps = int(parseFloat(s, "-fps"));
        else if (s.startsWith("-h") || s.startsWith("-help")){
            QMessageBox::warning(0, "Arguments",
                                 QString("Usage: qtdemo [-no-adapt] [-no-opengl] [-no-direct3d] [-no-ticker] [-no-rescale] ")
                                 + "[-no-animations] [-no-blending] [-no-sync] [-verbose] [-use-timer-update[0|1]] [-use-window-mask] [-fullscreen] "
                                 + "[-use-pixmaps] [-show-fps] [-show-br] [-use-8bit] [-use-loop] [-use-balls] [-animation-speed<float>] [-fps<int>] "
                                 + "[-low] [-ticker-letters<int>] [-ticker-speed<float>] [-no-ticker-morph] "
                                 + "[-ticker-morph-speed<float>] [-ticker-text<string>]");
            exit(0);
        }   
    }

#ifndef QT_NO_OPENGL
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0)
        Colors::glVersion = "2.0 or higher";
    else if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_5)
        Colors::glVersion = "1.5";
    else if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_4)
        Colors::glVersion = "1.4";
    else if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_3)
        Colors::glVersion = "1.3 or lower";

    if (!QGLFormat::hasOpenGL())
#endif        
        Colors::noOpenGl = true;
}

void Colors::adaptAccordingToEnvironment()
{
    if (Colors::noAdapt)
        return;

#if defined(Q_WS_X11)
    // check if X render is present:
    QPixmap tmp(1, 1);
    if (!tmp.x11PictureHandle()){
        Colors::low = true;
        Colors::useEightBitPalette = true;
        Colors::adapted = true;
        if (Colors::verbose)
            qDebug("- Adapt: X render not present.");
    }
#endif

#ifndef QT_NO_OPENGL    
    QGLWidget glw;
    if (Colors::low
        || !QGLFormat::hasOpenGL()
        || !glw.format().directRendering()
        || !(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_5)
        || glw.depth() < 24
    )
#endif    
    {
        // Since we are not going to use OpenGL,
        // turn off the most requiering stuff:
        Colors::noOpenGl = true;
        Colors::noTicker = true;
	    Colors::fps = 50;
        Colors::usePixmaps = true;
        Colors::adapted = true;
        if (Colors::verbose)
            qDebug("- Adapt: not using OpenGL.");
    }

    if (Colors::low){
        Colors::noOpenGl = true;
        Colors::noTicker = true;
	    Colors::noTimerUpdate = true;
	    Colors::fps = 30;
        Colors::usePixmaps = true;
        Colors::noAnimations = true;
        Colors::noBlending = true;
        Colors::low = true;
        Colors::adapted = true;
        if (Colors::verbose)
            qDebug("- Adapt: using setting 'low'.");
    }

    QWidget w;
    if (w.depth() < 16){
        Colors::useEightBitPalette = true;
        Colors::adapted = true;
        if (Colors::verbose)
            qDebug("- Adapt: color depth < 16. Using 8 bit palette.");
    }
}

