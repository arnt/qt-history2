/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

class QByteArray;
class QFile;
class QTimerEvent;

#include <QList>
#include <QMutex>
#include <QSet>
#include <QThread>
#include <QWaitCondition>

#include "metainfo.h"

class FileManager : public QThread
{
    Q_OBJECT
public:
    FileManager(QObject *parent = 0);
    virtual ~FileManager();

    void setMetaInfo(const MetaInfo &info);
    void setDestinationFolder(const QString &directory);
    
    int read(int pieceIndex, int offset, int length);
    void write(int pieceIndex, int offset, const QByteArray &data);
    void verifyPiece(int pieceIndex);
    qint64 totalSize() const;

    int pieceCount() const;
    int pieceLengthAt(int pieceIndex) const;
    QSet<int> completedPieces() const;

    QString errorString() const;

public slots:
    void startDataVerification();

signals:
    void dataRead(int id, int pieceIndex, int offset, const QByteArray &data);
    void error();
    void verificationProgress(int percent);
    void verificationDone();
    void pieceVerified(int pieceIndex, bool verified);

protected:
    void run();

private slots:
    bool verifySinglePiece(int pieceIndex);
    void wakeUp();

private:
    void parseMetaInfo();
    QByteArray readBlock(int pieceIndex, int offset, int length);
    bool writeBlock(int pieceIndex, int offset, const QByteArray &data);
    void verifyFileContents();

    struct WriteRequest {
	int pieceIndex;
	int offset;
	QByteArray data;
    };
    struct ReadRequest {
	int pieceIndex;
	int offset;
	int length;
	int id;
    };

    QString errString;
    QString destinationPath;
    MetaInfo metaInfo;
    QList<QFile *> files;
    QList<QByteArray> sha1s;
    QSet<int> verifiedPieces;

    bool newFile;
    int pieceLength;
    qint64 totalLength;
    int numPieces;
    int readId;
    bool startVerification;
    bool quit;
    bool wokeUp;

    QList<WriteRequest> writeRequests;
    QList<ReadRequest> readRequests;
    QList<int> pendingVerificationRequests;
    QList<int> newPendingVerificationRequests;

    mutable QMutex mutex;
    mutable QWaitCondition cond;
};

#endif
