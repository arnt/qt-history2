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

#include <QList>
#include <QMetaType>

BencodeParser::BencodeParser()
{
}

bool BencodeParser::parse(const QByteArray &content)
{
    if (content.isEmpty()) {
        errString = QString("No content");
        return false;
    }

    this->content = content;
    index = 0;
    infoStart = 0;
    infoLength = 0;
    return getDictionary(&dictionaryValue);
}

QString BencodeParser::errorString() const
{
    return errString;
}

QMap<QByteArray, QVariant> BencodeParser::dictionary() const
{
    return dictionaryValue;
}

QByteArray BencodeParser::infoSection() const
{
    return content.mid(infoStart, infoLength);
}

bool BencodeParser::getByteString(QByteArray *byteString)
{
    const int contentSize = content.size();
    int size = -1;
    do {
	char c = content.at(index);
	if (c < '0' || c > '9') {
	    if (size == -1)
		return false;
	    if (c != ':') {
		errString = QString("Unexpected character at pos %1: %2")
		    .arg(index).arg(c);
		return false;
	    }
	    ++index;
	    break;
	}
	if (size == -1)
	    size = 0;
	size *= 10;
	size += c - '0';
    } while (++index < contentSize);

    if (byteString)
	*byteString = content.mid(index, size);
    index += size;
    return true;
}

bool BencodeParser::getInteger(int *integer)
{
    const int contentSize = content.size();
    if (content.at(index) != 'i')
	return false;

    ++index;
    int num = -1;
    bool negative = false;

    do {
	char c = content.at(index);
	if (c < '0' || c > '9') {
	    if (num == -1) {
		if (c != '-' || negative)
		    return false;
		negative = true;
		continue;
	    } else {
		if (c != 'e') {
		    errString = QString("Unexpected character at pos %1: %2")
			.arg(index).arg(c);
		    return false;
		}
		++index;
		break;
	    }
	}
	if (num == -1)
	    num = 0;
	num *= 10;
	num += c - '0';
    } while (++index < contentSize);

    if (integer)
	*integer = negative ? -num : num;
    return true;
}

bool BencodeParser::getList(QList<QVariant> *list)
{
    const int contentSize = content.size();
    if (content.at(index) != 'l')
	return false;

    QList<QVariant> tmp;    
    ++index;

    do {
	if (content.at(index) == 'e') {
	    ++index;
	    break;
	}

	int number;
	QByteArray byteString;
	QList<QVariant> tmpList;
	QMap<QByteArray, QVariant> dictionary;

	if (getInteger(&number))
	    tmp << number;
	else if (getByteString(&byteString))
	    tmp << byteString;
	else if (getList(&tmpList))
	    tmp << tmpList;
	else if (getDictionary(&dictionary))
	    tmp << qVariantFromValue<QMap<QByteArray, QVariant> >(dictionary);
	else {
	    errString = QString("error at index %1").arg(index);
	    return false;
	}
    } while (index < contentSize);

    if (list)
	*list = tmp;
    return true;
}

bool BencodeParser::getDictionary(QMap<QByteArray, QVariant> *dictionary)
{
    const int contentSize = content.size();
    if (content.at(index) != 'd')
	return false;

    QMap<QByteArray, QVariant> tmp;    
    ++index;

    do {
	if (content.at(index) == 'e') {
	    ++index;
	    break;
	}

	QByteArray key;
	if (!getByteString(&key))
	    break;

	if (key == "info")
	  infoStart = index;

	int number;
	QByteArray byteString;
	QList<QVariant> tmpList;
	QMap<QByteArray, QVariant> dictionary;

	if (getInteger(&number))
	    tmp.insert(key, number);
	else if (getByteString(&byteString))
	    tmp.insert(key, byteString);
	else if (getList(&tmpList))
	    tmp.insert(key, tmpList);
	else if (getDictionary(&dictionary))
	    tmp.insert(key, qVariantFromValue<QMap<QByteArray, QVariant> >(dictionary));
	else {
	    errString = QString("error at index %1").arg(index);
	    return false;
	}

	if (key == "info")
	  infoLength = index - infoStart;

    } while (index < contentSize);

    if (dictionary)
	*dictionary = tmp;
    return true;
}
