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

class QTextFormatCollectionState
{
public:
    QTextFormatCollectionState() {}
    QTextFormatCollectionState(QDataStream &stream);
    QTextFormatCollectionState(const QTextFormatCollection *collection, const QVarLengthArray<int> &formatIndices);

    QMap<int, int> insertIntoOtherCollection(QTextFormatCollection *collection) const;

    typedef QMap<Q_INT32, QTextFormat> FormatMap;
    typedef QMap<Q_INT32, Q_INT32> GroupMap;

    FormatMap formats;
    // maps from group index to index (key) in 'formats' map
    GroupMap groups;
};

inline QDataStream &operator<<(QDataStream &stream, const QTextFormatCollectionState &state)
{ return stream << state.formats << state.groups; }

inline QDataStream &operator>>(QDataStream &stream, QTextFormatCollectionState &state)
{ return stream >> state.formats >> state.groups; }

class QTextDocumentFragmentPrivate
{
public:
    QTextDocumentFragmentPrivate() {}
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);

    void insert(QTextCursor &cursor) const;

    void appendBlock(int blockFormatIndex, int charFormatIndex);
    void appendText(const QString &text, int formatIdx);

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
        Block() : createBlockUponInsertion(true), blockFormat(-1), charFormat(-1) {}
        Q_INT8 createBlockUponInsertion;
        Q_INT32 blockFormat;
        Q_INT32 charFormat;
        FragmentVector fragments;
    };
    typedef QVector<Block> BlockVector;

    BlockVector blocks;
    QString localBuffer;
    QTextFormatCollectionState formats;
};

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate::TextFragment &fragment)
{ return stream << fragment.position << fragment.size << fragment.format; }
inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate::TextFragment &fragment)
{ return stream >> fragment.position >> fragment.size >> fragment.format; }

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate::Block &block)
{ return stream << block.createBlockUponInsertion << block.blockFormat << block.charFormat << block.fragments; }
inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate::Block &block)
{ return stream >> block.createBlockUponInsertion >> block.blockFormat >> block.charFormat >> block.fragments; }

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate &priv)
{ return stream << priv.formats << priv.blocks << priv.localBuffer; }
inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate &priv)
{ return stream >> priv.formats >> priv.blocks >> priv.localBuffer; }

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
    { appendText(QString(QTextObjectReplacementChar), format); }

    QTextDocumentFragmentPrivate *d;
    QVarLengthArray<int> listReferences;
    int indent;
    QVarLengthArray<int> tableIndices;

    QTextFormatCollection formats;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
