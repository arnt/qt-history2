/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTDOCUMENTFRAGMENT_P_H
#define QTEXTDOCUMENTFRAGMENT_P_H

#ifndef QT_H
#include <qlist.h>
#include <qmap.h>
#include <qpointer.h>
#include <qvarlengtharray.h>
#include <qdatastream.h>

#include "qtextdocument.h"
#include "qtexthtmlparser_p.h"
#include "qtextdocument_p.h"
#endif // QT_H

class QTextDocumentFragmentPrivate
{
public:
    QTextDocumentFragmentPrivate() : hasTitle(false) {}
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);

    void insert(QTextCursor &cursor) const;

    void appendFragments(QTextDocumentPrivate *priv, int pos, int endPos);
    int appendFragment(QTextDocumentPrivate *priv, int pos, int endPos, int objectIndex = -1);
    void appendText(const QString &text, int formatIdx, int blockIdx = -2);

    QMap<int, int> fillFormatCollection(QTextFormatCollection *collection) const;

    // ### TODO: merge back into one big vector.

    struct TextFragment
    {
        TextFragment()
            : position(0), size(0),
              charFormat(-1), blockFormat(-2) {}
        Q_INT32 position;
        Q_UINT32 size;
        Q_INT32 charFormat;
        Q_INT32 blockFormat;
    };
    typedef QVector<TextFragment> FragmentVector;

    FragmentVector fragments;

    QString localBuffer;

    QTextFormatCollection formatCollection;

    Q_INT8 hasTitle;
    QString title;
};

// ###### Versioning!

inline QDataStream &operator<<(QDataStream &stream,
                               const QTextDocumentFragmentPrivate::TextFragment &fragment)
{
    return stream << fragment.position
                  << fragment.size
                  << fragment.charFormat
                  << fragment.blockFormat;
}

inline QDataStream &operator>>(QDataStream &stream,
                               QTextDocumentFragmentPrivate::TextFragment &fragment)
{
    return stream >> fragment.position
                  >> fragment.size
                  >> fragment.charFormat
                  >> fragment.blockFormat;
}

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate &priv)
{
    return stream << priv.formatCollection
                  << priv.fragments
                  << priv.localBuffer
                  << priv.hasTitle
                  << priv.title;
}

inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate &priv)
{
    return stream >> priv.formatCollection
                  >> priv.fragments
                  >> priv.localBuffer
                  >> priv.hasTitle
                  >> priv.title;
}

class QTextHTMLImporter : public QTextHtmlParser
{
    struct Table;
public:
    QTextHTMLImporter(QTextDocumentFragmentPrivate *d, const QString &html);

    void import();

private:
    void closeTag(int i);

    void scanTable(int tableNodeIdx, Table *table);

    void appendBlock(const QTextBlockFormat &format, const QTextCharFormat &charFmt = QTextCharFormat(), const QChar &separator = QChar::ParagraphSeparator);
    void appendText(const QString &text, const QTextFormat &format);
    inline void appendImage(const QTextImageFormat &format)
    { appendText(QString(QChar::ObjectReplacementCharacter), format); }

    QTextDocumentFragmentPrivate *d;
    QVector<int> listReferences;
    int indent;

    struct Table
    {
        Table() : tableIndex(-1), currentColumnCount(0) {}
        int tableIndex; // objectIndex
        int currentColumnCount;
        int columns;
    };
    QVarLengthArray<Table> tables;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
