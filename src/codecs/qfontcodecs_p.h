/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontencodings_p.h#1 $
**
** Font utilities for X11
**
** Created : 20001101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QFONTCODECS_P_H
#define QFONTCODECS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qfontencodings_x11.cpp and qfont_x11.cpp.  This header file may
// change from version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include <qfeatures.h>
#include <qtextcodec.h>
#endif // QT_H


#ifndef QT_NO_CODECS


class QJpUnicodeConv;


class QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();
    ~QFontJis0208Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;


private:
    const QJpUnicodeConv *convJP;
};




class QFontJis0212Codec : public QTextCodec
{
public:
    QFontJis0212Codec();
    ~QFontJis0212Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;


private:
    const QJpUnicodeConv *convJP;
};





class QFontKsc5601Codec : public QTextCodec
{
public:
    QFontKsc5601Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;
};




class QFontGB2312Codec : public QTextCodec
{
public:
    QFontGB2312Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;
};




class QFontBig5Codec : public QTextCodec
{
public:
    QFontBig5Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;
};




// ------------------------------------------------------------------
// the shaping codec for iso8859-6.8x fonts (see www.langbox.com)

class QFontArabic68Codec : public QTextCodec
{
public:
    QFontArabic68Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;

    int heuristicContentMatch(const char *, int) const;
    QByteArray fromUnicode( const QString &str, int from, int len ) const;
    unsigned short characterFromUnicode(const QString &str, int pos) const;
};


#endif //QT_NO_CODECS

#endif // QFONTCODECS_P_H
