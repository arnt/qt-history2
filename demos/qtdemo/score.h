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

#ifndef SCORE_H
#define SCORE_H

#include <QList>
#include <QHash>
#include "demoitemanimation.h"

typedef QList<DemoItemAnimation *> Movie;
typedef QHash<QString, Movie*> MovieIndex;

class PlayListMember
{
public:
    PlayListMember(Movie *movie, int runMode) : movie(movie), runMode(runMode){};
    Movie *movie;
    int runMode;
};
typedef QList<PlayListMember> PlayList;

class Score
{
public:
    enum LOCK_MODE {LOCK_ITEMS, UNLOCK_ITEMS, SKIP_LOCK};
    enum RUN_MODE {FROM_CURRENT, FROM_START, NEW_ANIMATION_ONLY};
    
    Score();
    virtual ~Score();
    
    void playMovie(const QString &indexName, RUN_MODE runMode = FROM_START, LOCK_MODE lockMode = SKIP_LOCK);
    void insertMovie(const QString &indexName, Movie *movie);
    Movie *insertMovie(const QString &indexName);
    void queueMovie(const QString &indexName, RUN_MODE runMode = FROM_START, LOCK_MODE lockMode = SKIP_LOCK);
    void playQue();
    bool hasQueuedMovies(){ return this->playList.size() > 0; };
    
    MovieIndex index;
    PlayList playList;
    
private:
    void prepare(Movie *movie, LOCK_MODE lockMode);
    void play(Movie *movie, RUN_MODE runMode);  
};

#endif // SCORE_H

