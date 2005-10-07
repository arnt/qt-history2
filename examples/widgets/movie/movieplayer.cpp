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

#include <QtGui>

#include "movieplayer.h"

MoviePlayer::MoviePlayer(QWidget *parent)
    : QWidget(parent)
{
    movieScreen = new QLabel;
    movieScreen->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);
    movieScreen->setBackgroundRole(QPalette::Base);
    movieScreen->setText(tr("Use the Eject button to select a movie."));
    movieScreen->setAlignment(Qt::AlignCenter);
    movieScreen->setWordWrap(true);

    currentMovieDirectory = "movies";

    createSliders();
    createCheckBox();
    createButtons();

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(movieScreen);
    mainLayout->addWidget(scaleMovieCheckbox);
    mainLayout->addLayout(slidersLayout);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    speedSlider->setDisabled(true);
    scaleMovieCheckbox->setDisabled(true);
    playButton->setDisabled(true);
    stopButton->setDisabled(true);

    resize(400, 400);
    setWindowTitle(tr("Movie"));
}

void MoviePlayer::browse()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this, tr("Select a Movie"),
                                            currentMovieDirectory,
                                            tr("Movies (*.gif *.mng)"));

    if (!fileName.isEmpty()) {
        int index = fileName.length() - fileName.lastIndexOf("/");
        if (!(index < 0)) {
            currentMovieDirectory = fileName;
            currentMovieDirectory.chop(index);
        }

        if (movieScreen->movie() &&
                    movieScreen->movie()->state() != QMovie::NotRunning)
            movieScreen->movie()->stop();

        QMovie *movie = new QMovie(fileName, QByteArray(), this);
        movie->setCacheMode(QMovie::CacheAll);

        movie->setSpeed(100);
        speedSlider->setValue(100);
        connect(speedSlider, SIGNAL(valueChanged(int)),
                movie, SLOT(setSpeed(int)));

        movieScreen->setMovie(movie);

        bool supportsFrames = movie->frameCount() > 0;
        if (supportsFrames){
            frameSlider->setMaximum(movie->frameCount()-1);
            connect(movie, SIGNAL(frameChanged(int)),
                    frameSlider, SLOT(setValue(int)));
        }
        frameSlider->setVisible(supportsFrames);
        frameLabel->setVisible(supportsFrames);

        speedSlider->setDisabled(false);
        scaleMovieCheckbox->setDisabled(false);
        playButton->setDisabled(false);
        stopButton->setDisabled(false);

        movie->start();
    }
}

void MoviePlayer::start()
{
    QMovie *movie = movieScreen->movie();

    if (movie->state() == QMovie::NotRunning)
        movie->start();
    else
        movie->setPaused(!(movie->state() == QMovie::Paused));
}

void MoviePlayer::stop()
{
    QMovie *movie = movieScreen->movie();

    if (!(movie->state() == QMovie::NotRunning))
        movie->stop();
}

void MoviePlayer::goToFrame(int frame)
{
    movieScreen->movie()->jumpToFrame(frame);
}

void MoviePlayer::scaleMovie()
{
    movieScreen->setScaledContents(scaleMovieCheckbox->isChecked());
}

void MoviePlayer::createSliders()
{
    frameSlider = new QSlider(Qt::Horizontal);
    frameSlider->setTickPosition(QSlider::TicksBelow);
    frameSlider->setTickInterval(1);
    connect(frameSlider, SIGNAL(valueChanged(int)),
            this, SLOT(goToFrame(int)));
    frameSlider->setVisible(false);

    frameLabel = new QLabel(tr("Jump to frame"));
    frameLabel->setVisible(false);

    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setMaximum(1000);
    speedSlider->setTickInterval(50);
    speedSlider->setTickPosition(QSlider::TicksBelow);
    speedSlider->setMinimum(50);

    speedLabel = new QLabel(tr("Set speed"));

    slidersLayout = new QGridLayout;
    slidersLayout->addWidget(frameLabel, 0, 0);
    slidersLayout->addWidget(frameSlider, 0, 1);
    slidersLayout->addWidget(speedLabel, 1, 0);
    slidersLayout->addWidget(speedSlider, 1, 1);
}

void MoviePlayer::createCheckBox()
{
    scaleMovieCheckbox = new QCheckBox(tr("Scale movie"));
    connect(scaleMovieCheckbox, SIGNAL(clicked()),
            this, SLOT(scaleMovie()));
}


void MoviePlayer::createButtons()
{
    browseButton = new QToolButton;
    browseButton->setToolTip(tr("Eject/Open File..."));
    browseButton->setIcon(QIcon(":/icons/eject.png"));
    browseButton->setIconSize(QSize(32, 32));
    connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    playButton = new QToolButton;
    playButton->setToolTip(tr("Play/Pause"));
    playButton->setIcon(QIcon(":/icons/play-pause.png"));
    playButton->setIconSize(QSize(49, 32));
    connect(playButton, SIGNAL(clicked()), this, SLOT(start()));

    stopButton = new QToolButton;
    stopButton->setToolTip(tr("Stop"));
    stopButton->setIcon(QIcon(":/icons/stop.png"));
    stopButton->setIconSize(QSize(32, 32));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));

    quitButton = new QToolButton;
    quitButton->setToolTip(tr("Quit"));
    quitButton->setIcon(QIcon(":/icons/quit.png"));
    quitButton->setIconSize(QSize(32, 32));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(playButton);
    buttonsLayout->addWidget(stopButton);
    buttonsLayout->addWidget(browseButton);
    buttonsLayout->addWidget(quitButton);
    buttonsLayout->addStretch();
}

