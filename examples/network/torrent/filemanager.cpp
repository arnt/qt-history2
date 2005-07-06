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
#include "filemanager.h"
#include "metainfo.h"
#include "sha1.h"

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSet>
#include <QTimer>
#include <QTimerEvent>

FileManager::FileManager(QObject *parent)
    : QThread(parent)
{
    quit = false;
    totalLength = 0;
    readId = 0;
    startVerification = false;
    wokeUp = false;
    newFile = false;
    numPieces = 0;
}

FileManager::~FileManager()
{
    quit = true;
    cond.wakeOne();
    wait();

    foreach (QFile *file, files) {
        file->close();
        delete file;
    }
}

void FileManager::setMetaInfo(const MetaInfo &info)
{
    metaInfo = info;
}

void FileManager::setDestinationFolder(const QString &directory)
{
    destinationPath = directory;
}

int FileManager::read(int pieceIndex, int offset, int length)
{
    ReadRequest request;
    request.pieceIndex = pieceIndex;
    request.offset = offset;
    request.length = length;

    QMutexLocker locker(&mutex);
    request.id = readId++;
    readRequests << request;

    if (!wokeUp) {
	wokeUp = true;
	QTimer::singleShot(0, this, SLOT(wakeUp()));
    }

    return request.id;
}

void FileManager::write(int pieceIndex, int offset, const QByteArray &data)
{
    WriteRequest request;
    request.pieceIndex = pieceIndex;
    request.offset = offset;
    request.data = data;

    QMutexLocker locker(&mutex);
    writeRequests << request;

    if (!wokeUp) {
	wokeUp = true;
	QTimer::singleShot(0, this, SLOT(wakeUp()));
    }
}

void FileManager::verifyPiece(int pieceIndex)
{
    QMutexLocker locker(&mutex);
    pendingVerificationRequests << pieceIndex;
    startVerification = true;

    if (!wokeUp) {
	wokeUp = true;
	QTimer::singleShot(0, this, SLOT(wakeUp()));
    }
}

qint64 FileManager::totalSize() const
{
    return totalLength;
}

int FileManager::pieceCount() const
{
    return numPieces;
}

int FileManager::pieceLengthAt(int pieceIndex) const
{
    QMutexLocker locker(&mutex);
    return (sha1s.size() == pieceIndex + 1)
	? (totalLength % pieceLength) : pieceLength;
}

QSet<int> FileManager::completedPieces() const
{
    QMutexLocker locker(&mutex);
    return verifiedPieces;
}

QString FileManager::errorString() const
{
    return errString;
}

void FileManager::run()
{
    // Parse torrent data
    parseMetaInfo();

    do {
	{
	    QMutexLocker locker(&mutex);
	    if (!quit && readRequests.isEmpty() && writeRequests.isEmpty() && !startVerification)
		cond.wait(&mutex);
	}

	// Read pending read requests
	mutex.lock();
	QList<ReadRequest> newReadRequests = readRequests;
	readRequests.clear();
	mutex.unlock();
	while (!newReadRequests.isEmpty()) {
	    ReadRequest request = newReadRequests.takeFirst();
	    QByteArray block = readBlock(request.pieceIndex, request.offset, request.length);
	    emit dataRead(request.id, request.pieceIndex, request.offset, block);
	}

	// Write pending write requests
	mutex.lock();
	QList<WriteRequest> newWriteRequests = writeRequests;
	writeRequests.clear();
	while (!newWriteRequests.isEmpty()) {
	    WriteRequest request = newWriteRequests.takeFirst();
	    if (!writeBlock(request.pieceIndex, request.offset, request.data)) {
		emit error();
		break;
	    }
	}

	// Process pending verification requests
	if (startVerification) {
            newPendingVerificationRequests = pendingVerificationRequests;
            pendingVerificationRequests.clear();
	    verifyFileContents();
	    startVerification = false;
	}
	mutex.unlock();
	newPendingVerificationRequests.clear();

    } while (!quit);

    // Write pending write requests
    mutex.lock();
    QList<WriteRequest> newWriteRequests = writeRequests;
    writeRequests.clear();
    mutex.unlock();
    while (!newWriteRequests.isEmpty()) {
	WriteRequest request = newWriteRequests.takeFirst();
	if (!writeBlock(request.pieceIndex, request.offset, request.data)) {
	    emit error();
	    break;
	}
    }
}

void FileManager::startDataVerification()
{
    QMutexLocker locker(&mutex);
    startVerification = true;
    cond.wakeOne();
}

void FileManager::parseMetaInfo()
{
    // Set up the thread local data
    if (metaInfo.fileForm() == MetaInfo::SingleFileForm) {
	QMutexLocker locker(&mutex);
        MetaInfoSingleFile singleFile = metaInfo.singleFile();

	QString prefix;
	if (!destinationPath.isEmpty()) {
	    prefix = destinationPath;
	    if (!prefix.endsWith("/"))
		prefix += "/";
	    QDir dir;
	    if (!dir.mkpath(prefix)) {
                errString = tr("Failed to create directory %1").arg(prefix);
                emit error();
                return;
            }
	}
        QFile *file = new QFile(prefix + singleFile.name);
        if (!file->open(QFile::ReadWrite)) {
            errString = tr("Failed to open/create file %1: %2").arg(file->fileName()).arg(file->errorString());
            emit error();
            return;
        }
        
	if (file->size() != singleFile.length) {
            newFile = true;
	    if (!file->resize(singleFile.length)) {
                errString = tr("Failed to resize file %1: %2").arg(file->fileName()).arg(file->errorString());
                emit error();
                return;
            }
        }
        files << file;

        pieceLength = singleFile.pieceLength;
	totalLength = singleFile.length;
	sha1s = singleFile.sha1Sums;
    } else {
	QMutexLocker locker(&mutex);
        QDir dir;
        QString prefix;

        if (!destinationPath.isEmpty()) {
            prefix = destinationPath;
            if (!prefix.endsWith("/"))
                prefix += "/";
	}
	if (!metaInfo.name().isEmpty()) {
            prefix += metaInfo.name();
            if (!prefix.endsWith("/"))
                prefix += "/";
	}
        if (!dir.mkpath(prefix)) {
            errString = tr("Failed to create directory %1").arg(prefix);
            emit error();
            return;
        }

        foreach (const MetaInfoMultiFile &entry, metaInfo.multiFiles()) {
            QFile *file = new QFile(prefix + entry.path);
            if (!file->open(QFile::ReadWrite)) {
                errString = tr("Failed to open/create file %1: %2").arg(file->fileName()).arg(file->errorString());
                emit error();
                return;
            }
            
	    if (file->size() != entry.length) {
                newFile = true;
                if (!file->resize(entry.length)) {
                    errString = tr("Failed to resize file %1: %2").arg(file->fileName()).arg(file->errorString());
                    emit error();
                    return;
                }
            }
            files << file;   
            totalLength += entry.length;
	}

	sha1s = metaInfo.sha1Sums();
        pieceLength = metaInfo.pieceLength();
    }
    numPieces = sha1s.size();
}

QByteArray FileManager::readBlock(int pieceIndex, int offset, int length)
{
    QByteArray block;
    qint64 startReadIndex = (pieceIndex * pieceLength) + offset;
    qint64 currentIndex = 0;
	    
    for (int i = 0; !quit && i < files.size(); ++i) {
	QFile *file = files[i];
	qint64 currentFileSize = file->size();
	if ((currentIndex + currentFileSize) > startReadIndex) {
	    file->seek(startReadIndex - currentIndex);
	    QByteArray chunk = file->read(qMin<qint64>(length, currentFileSize - file->pos()));
	    block += chunk;
	    length -= chunk.size();
	    startReadIndex += chunk.size();
	    if (length <= 0) {
		emit error();
		break;
	    }
	}
	currentIndex += currentFileSize;
    }
    return block;
}

bool FileManager::writeBlock(int pieceIndex, int offset, const QByteArray &data)
{
    qint64 startWriteIndex = (pieceIndex * pieceLength) + offset;
    qint64 currentIndex = 0;
    int bytesToWrite = data.size();
    int written = 0;

    for (int i = 0; !quit && i < files.size(); ++i) {
	QFile *file = files[i];
	qint64 currentFileSize = file->size();

	if ((currentIndex + currentFileSize) > startWriteIndex) {
	    file->seek(startWriteIndex - currentIndex);
	    qint64 bytesWritten = file->write(data.constData() + written,
					      qMin<qint64>(bytesToWrite, currentFileSize - file->pos()));
	    if (bytesWritten <= 0)
		return false;

	    written += bytesWritten;
	    startWriteIndex += bytesWritten;
	    bytesToWrite -= bytesWritten;
	    if (bytesToWrite == 0)
		break;
	}
	currentIndex += currentFileSize;
    }
    return true;
}

void FileManager::verifyFileContents()
{
    // Verify all pieces the first time
    if (newPendingVerificationRequests.isEmpty()) {
	int oldPercent = 0;
	verifiedPieces.clear();
        if (!newFile) {
            int numPieces = (totalLength / pieceLength) + 1;
            for (int index = 0; index < numPieces; ++index) {
                verifySinglePiece(index);
	    
                int percent = ((index + 1) * 100) / numPieces;
                if (oldPercent != percent) {
                    emit verificationProgress(percent);
                    oldPercent = percent;
                }
            }
        }
	emit verificationDone();
	return;
    }

    // Verify all pending pieces
    foreach (int index, newPendingVerificationRequests)
	emit pieceVerified(index, verifySinglePiece(index));
}

bool FileManager::verifySinglePiece(int pieceIndex)
{
    QByteArray sha1Sum = sha1Checksum(readBlock(pieceIndex, 0, pieceLength));
    if (sha1Sum != sha1s.at(pieceIndex))
	return false;
    
    verifiedPieces << pieceIndex;
    return true;
}

void FileManager::wakeUp()
{
    QMutexLocker locker(&mutex);
    wokeUp = false;
    cond.wakeOne();
}
