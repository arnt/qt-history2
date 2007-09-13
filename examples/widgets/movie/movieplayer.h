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

#ifndef MOVIEPLAYER_H
#define MOVIEPLAYER_H

#include <QWidget>

QT_DECLARE_CLASS(QCheckBox)
QT_DECLARE_CLASS(QGridLayout)
QT_DECLARE_CLASS(QHBoxLayout)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QMovie)
QT_DECLARE_CLASS(QSlider)
QT_DECLARE_CLASS(QSpinBox)
QT_DECLARE_CLASS(QToolButton)
QT_DECLARE_CLASS(QVBoxLayout)

class MoviePlayer : public QWidget
{
    Q_OBJECT

public:
    MoviePlayer(QWidget *parent = 0);
    void openFile(const QString &fileName);

private slots:
    void open();
    void goToFrame(int frame);
    void fitToWindow();
    void updateButtons();
    void updateFrameSlider();

private:
    void createControls();
    void createButtons();

    QString currentMovieDirectory;
    QLabel *movieLabel;
    QMovie *movie;
    QToolButton *openButton;
    QToolButton *playButton;
    QToolButton *pauseButton;
    QToolButton *stopButton;
    QToolButton *quitButton;
    QCheckBox *fitCheckBox;
    QSlider *frameSlider;
    QSpinBox *speedSpinBox;
    QLabel *frameLabel;
    QLabel *speedLabel;

    QGridLayout *controlsLayout;
    QHBoxLayout *buttonsLayout;
    QVBoxLayout *mainLayout;
};

#endif
