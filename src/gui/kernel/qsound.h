/****************************************************************************
**
** Definition of QSound class and QAuServer internal class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QSOUND_H
#define QSOUND_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_SOUND

class QSoundData;

class Q_GUI_EXPORT QSound : public QObject {
    Q_OBJECT
public:
    static bool isAvailable();
    static void play(const QString& filename);

    QSound(const QString& filename, QObject* parent = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QSound(const QString& filename, QObject* parent, const char* name);
#endif
    ~QSound();

    /* Coming soon...
        ?
    QSound(int hertz, Type type=Mono);
    int play(const ushort* data, int samples);
    bool full();
    signal void notFull();
        ?
    */

#ifdef QT_COMPAT
    static QT_COMPAT bool available() { return isAvailable(); }
#endif

    int loops() const;
    int loopsRemaining() const;
    void setLoops(int);
    QString fileName() const;

    bool isFinished() const;

public slots:
    void play();
    void stop();

private:
    QSoundData* d;
    friend class QAuServer;
};


/*
  QAuServer is an INTERNAL class.  If you wish to provide support for
  additional audio servers, you can make a subclass of QAuServer to do
  so, HOWEVER, your class may need to be re-engineered to some degree
  with each new Qt release, including minor releases.

  QAuBucket is whatever you want.
*/

class QAuBucket {
public:
    virtual ~QAuBucket();
};

class QAuServer : public QObject {
    Q_OBJECT

public:
    QAuServer(QObject* parent);
    ~QAuServer();

    virtual void init(QSound*);
    virtual void play(const QString& filename);
    virtual void play(QSound*)=0;
    virtual void stop(QSound*)=0;
    virtual bool okay()=0;

protected:
    void setBucket(QSound*, QAuBucket*);
    QAuBucket* bucket(QSound*);
    int decLoop(QSound*);
};

#endif // QT_NO_SOUND

#endif
