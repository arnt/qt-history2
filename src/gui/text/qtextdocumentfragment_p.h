#ifndef QTEXTDOCUMENTFRAGMENT_P_H
#define QTEXTDOCUMENTFRAGMENT_P_H

#ifndef QT_H
#include <qlist.h>
#include <qmap.h>
#include <qpointer.h>
#include <qvarlengtharray.h>

#include "qtextdocument.h"
#include "qtexthtmlparser_p.h"
#include "qtextpiecetable_p.h"
#endif // QT_H

class QTextFormatCollectionState
{
public:
    QTextFormatCollectionState() {}
    QTextFormatCollectionState(const QTextHtmlParser &parser, int formatsNode);
    QTextFormatCollectionState(const QTextFormatCollection *collection, const QList<int> &formatIndices);

    QMap<int, int> insertIntoOtherCollection(QTextFormatCollection *collection) const;

    void save(QTextStream &stream) const;

private:
    static QTextFormat::PropertyType stringToPropertyType(const QString &typeString);

    typedef QMap<int, QTextFormat> FormatMap;
    typedef QMap<int, int> ReferenceMap;

    FormatMap formats;
    // maps from reference index to index (key) in 'formats' map
    ReferenceMap references;
};

class QTextDocumentFragmentPrivate
{
public:
    QTextDocumentFragmentPrivate() {
        localFormatCollection = new QTextFormatCollection;
        ++localFormatCollection->ref;
    }
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);
    ~QTextDocumentFragmentPrivate();
    QTextDocumentFragmentPrivate(const QTextDocumentFragmentPrivate &);
    QTextDocumentFragmentPrivate &operator =(const QTextDocumentFragmentPrivate &);

    void insert(QTextCursor &cursor) const;

    QTextFormatCollectionState formatCollectionState() const;

    struct Fragment;

    inline void appendBlock(const QTextBlockFormat &format)
    { appendText(QString(QTextParagraphSeparator), format); }
    inline void appendBlock(int formatIdx)
    { appendText(QString(QTextParagraphSeparator), formatIdx); }
    inline void appendText(const QString &text, const QTextFormat &format)
    { appendText(text, localFormatCollection->indexForFormat(format)); }
    void appendText(const QString &text, int formatIdx, int origPos = -1);
    inline void appendImage(const QTextImageFormat &format)
    { appendText(QString(QTextObjectReplacementChar), format); }

    struct Fragment
    {
        int position;
        int originalPosition;
        Q_UINT32 size;
        int format;
    };
    typedef QList<Fragment> FragmentList;

    FragmentList fragments;

    QString localBuffer;
    QTextFormatCollection *localFormatCollection;

    QPointer<QTextPieceTable> pieceTable;
};

class QTextHTMLImporter : public QTextHtmlParser
{
public:
    QTextHTMLImporter(QTextDocumentFragmentPrivate *d, const QString &html);

    void import();

private:
    void closeTag(int i);

    QTextDocumentFragmentPrivate *d;
    QVarLengthArray<int> listReferences;
    int indent;
    QVarLengthArray<int> tableIndices;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
