#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include "qtextformat.h"

class QTextFormatCollection : public QObject
{
    Q_OBJECT
public:
    QTextFormatCollection(QObject *parent = 0);
    ~QTextFormatCollection();

    int createReferenceIndex(const QTextFormat &newFormat);
    void updateReferenceIndex(int index, const QTextFormat &newFormat);

    int indexForFormat(const QTextFormat &f);

    QTextFormat format(int idx, int defaultFormatType = -1) const;

    QTextBlockFormat blockFormat(int index) const
    { return format(index, QTextFormat::BlockFormat).toBlockFormat(); }
    QTextCharFormat charFormat(int index) const
    { return format(index, QTextFormat::CharFormat).toCharFormat(); }
    QTextListFormat listFormat(int index) const
    { return format(index, QTextFormat::ListFormat).toListFormat(); }
    QTextTableFormat tableFormat(int index) const
    { return format(index, QTextFormat::TableFormat).toTableFormat(); }
    QTextImageFormat imageFormat(int index) const
    { return format(index, QTextFormat::ImageFormat).toImageFormat(); }

    int numFormats() const;

private:
#if defined(Q_DISABLE_COPY)
    QTextFormatCollection(const QTextFormatCollection &);
    QTextFormatCollection &operator=(const QTextFormatCollection &);
#endif
    Q_DECL_PRIVATE(QTextFormatCollection);
};

#endif // QTEXTFORMAT_P_H
