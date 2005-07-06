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
#include "bencodeparser.h"
#include "metainfo.h"

#include <QDateTime>
#include <QDebug>
#include <QMetaType>
#include <QString>

MetaInfo::MetaInfo()
{
    clear();
}

void MetaInfo::clear()
{
    errString = "Unknown error";
    content.clear();
    infoData.clear();
    metaInfoMultiFiles.clear();
    metaInfoAnnounce.clear();
    metaInfoAnnounceList.clear();
    metaInfoCreationDate = QDateTime();
    metaInfoComment.clear();
    metaInfoCreatedBy.clear();
    metaInfoName.clear();
    metaInfoPieceLength = 0;
    metaInfoSha1Sums.clear();
}

bool MetaInfo::parse(const QByteArray &data)
{
    clear();
    content = data;

    BencodeParser parser;
    if (!parser.parse(content)) {
	errString = parser.errorString();
	return false;
    }

    infoData = parser.infoSection();

    QMap<QByteArray, QVariant> dict = parser.dictionary();
    if (!dict.contains("info"))
	return false;

    QMap<QByteArray, QVariant> info = qVariantValue<Dictionary>(dict.value("info"));

    if (info.contains("files")) {
	metaInfoFileForm = MultiFileForm;

	QList<QVariant> files = info.value("files").toList();

	for (int i = 0; i < files.size(); ++i) {
	    QMap<QByteArray, QVariant> file = qVariantValue<Dictionary>(files.at(i));
	    QList<QVariant> pathElements = file.value("path").toList();
	    QByteArray path;
	    foreach (QVariant p, pathElements) {
		if (!path.isEmpty())
		    path += "/";
		path += p.toByteArray();
	    }

	    MetaInfoMultiFile multiFile;
	    multiFile.length = file.value("length").toInt();
	    multiFile.path = QString::fromUtf8(path);
	    multiFile.md5sum = file.value("md5sum").toByteArray();
	    metaInfoMultiFiles << multiFile;
	}

	metaInfoName = QString::fromUtf8(info.value("name").toByteArray());
	metaInfoPieceLength = info.value("piece length").toInt();
	QByteArray pieces = info.value("pieces").toByteArray();
	for (int i = 0; i < pieces.size(); i += 20)
	    metaInfoSha1Sums << pieces.mid(i, 20);
    } else if (info.contains("length")) {
        metaInfoFileForm = SingleFileForm;
        metaInfoSingleFile.length = info.value("length").toInt();
        metaInfoSingleFile.md5sum = info.value("md5sum").toByteArray();
	metaInfoSingleFile.name = QString::fromUtf8(info.value("name").toByteArray());
	metaInfoSingleFile.pieceLength = info.value("piece length").toInt();

	QByteArray pieces = info.value("pieces").toByteArray();
	for (int i = 0; i < pieces.size(); i += 20)
	    metaInfoSingleFile.sha1Sums << pieces.mid(i, 20);
    }

    metaInfoAnnounce = QString::fromUtf8(dict.value("announce").toByteArray());

    if (dict.contains("announce-list")) {
	// ### unimplemented
    }

    if (dict.contains("creation date"))
	metaInfoCreationDate.setTime_t(dict.value("creation date").toInt());
    if (dict.contains("comment"))
	metaInfoComment = QString::fromUtf8(dict.value("comment").toByteArray());
    if (dict.contains("created by"))
	metaInfoCreatedBy = QString::fromUtf8(dict.value("created by").toByteArray());

    return true;
}

QByteArray MetaInfo::infoValue() const
{
    return infoData;
}

QString MetaInfo::errorString() const
{
    return errString;
}

MetaInfo::FileForm MetaInfo::fileForm() const
{
    return metaInfoFileForm;
}

QString MetaInfo::announceUrl() const
{
    return metaInfoAnnounce;
}

QStringList MetaInfo::announceList() const
{
    return metaInfoAnnounceList;
}

QDateTime MetaInfo::creationDate() const
{
    return metaInfoCreationDate;
}

QString MetaInfo::comment() const
{
    return metaInfoComment;
}

QString MetaInfo::createdBy() const
{
    return metaInfoCreatedBy;
}

MetaInfoSingleFile MetaInfo::singleFile() const
{
    return metaInfoSingleFile;
}

QList<MetaInfoMultiFile> MetaInfo::multiFiles() const
{
    return metaInfoMultiFiles;
}

QString MetaInfo::name() const
{
    return metaInfoName;
}

int MetaInfo::pieceLength() const
{
    return metaInfoPieceLength;
}

QList<QByteArray> MetaInfo::sha1Sums() const
{
    return metaInfoSha1Sums;
}

qint64 MetaInfo::totalSize() const
{
    if (fileForm() == SingleFileForm)
	return singleFile().length;

    qint64 size = 0;
    foreach (MetaInfoMultiFile file, multiFiles())
	size += file.length;
    return size;
}


