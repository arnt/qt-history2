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
#include "qtextpiecetable_p.h"
#endif // QT_H

class QTextDocumentFragmentPrivate
{
public:
    QTextDocumentFragmentPrivate() : hasTitle(false) {}
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);

    void insert(QTextCursor &cursor) const;

    void appendBlock(int blockFormatIndex, int charFormatIndex, const QChar &separator = QChar::ParagraphSeparator);
    void appendText(const QString &text, int formatIdx);

    void readFormatCollection(const QTextFormatCollection *collection, const QVarLengthArray<int> &formatIndices);
    QMap<int, int> fillFormatCollection(QTextFormatCollection *collection) const;

    // ### TODO: merge back into one big vector.

    struct TextFragment
    {
        Q_INT32 position;
        Q_UINT32 size;
        Q_INT32 format;
    };
    typedef QVector<TextFragment> FragmentVector;

    struct Block
    {
        Block() : createBlockUponInsertion(true), blockFormat(-1), charFormat(-1), separator(QChar::ParagraphSeparator) {}
        Q_INT8 createBlockUponInsertion;
        Q_INT32 blockFormat;
        Q_INT32 charFormat;
        Q_UINT16 separator;
        FragmentVector fragments;
    };
    typedef QVector<Block> BlockVector;

    BlockVector blocks;
    QString localBuffer;

    typedef QMap<Q_INT32, QTextFormat> FormatMap;
    typedef QMap<Q_INT32, Q_INT32> GroupMap;

    FormatMap formats;
    // maps from group index to index (key) in 'formats' map
    GroupMap formatGroups;

    Q_INT8 hasTitle;
    QString title;
};

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate::TextFragment &fragment)
{ return stream << fragment.position << fragment.size << fragment.format; }
inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate::TextFragment &fragment)
{ return stream >> fragment.position >> fragment.size >> fragment.format; }

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate::Block &block)
{ return stream << block.createBlockUponInsertion << block.blockFormat << block.charFormat << block.separator << block.fragments; }
inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate::Block &block)
{ return stream >> block.createBlockUponInsertion >> block.blockFormat >> block.charFormat >> block.separator >> block.fragments; }

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate &priv)
{ return stream << priv.formats << priv.formatGroups << priv.blocks << priv.localBuffer << priv.hasTitle << priv.title; }
inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate &priv)
{ return stream >> priv.formats >> priv.formatGroups >> priv.blocks >> priv.localBuffer >> priv.hasTitle >> priv.title; }

class QTextHTMLImporter : public QTextHtmlParser
{
public:
    QTextHTMLImporter(QTextDocumentFragmentPrivate *d, const QString &html);

    void import();

private:
    void closeTag(int i);

    void appendBlock(const QTextBlockFormat &format, const QTextCharFormat &charFmt = QTextCharFormat());
    void appendText(const QString &text, const QTextFormat &format);
    inline void appendImage(const QTextImageFormat &format)
    { appendText(QString(QChar::ObjectReplacementCharacter), format); }

    QTextDocumentFragmentPrivate *d;
    QVarLengthArray<int> listReferences;
    int indent;
    QVarLengthArray<int> tableIndices;

    QTextFormatCollection formats;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
