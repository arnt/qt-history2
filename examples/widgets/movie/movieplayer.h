/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MOVIEPLAYER_H
#define MOVIEPLAYER_H

#include <QWidget>

class QCheckBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QSlider;
class QToolButton;
class QVBoxLayout;

class MoviePlayer : public QWidget
{
    Q_OBJECT

public:
    MoviePlayer(QWidget *parent = 0);

public slots:
    void browse();
    void start();
    void stop();
    void goToFrame(int frame);
    void scaleMovie();

private:
    QLabel *movieScreen;
    QString currentMovieDirectory;

    void createButtons();
    void createCheckBox();
    void createSliders();

    QToolButton *browseButton;
    QToolButton *playButton;
    QToolButton *quitButton;
    QToolButton *stopButton;
    QCheckBox *scaleMovieCheckbox;
    QSlider *frameSlider;
    QSlider *speedSlider;
    QLabel *frameLabel;
    QLabel *speedLabel;

    QHBoxLayout *buttonsLayout;
    QGridLayout *slidersLayout;
    QVBoxLayout *mainLayout;
};

#endif
