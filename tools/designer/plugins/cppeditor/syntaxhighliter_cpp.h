#ifndef QCPPSYNTAXHIGHLIGHTER_H
#define QCPPSYNTAXHIGHLIGHTER_H

#include "qrichtext_p.h"

class SyntaxHighlighter_CPP : public QTextPreProcessor
{
public:
    enum CppIds {
	Comment = 1,
	Number,
	String,
	Type,
	Keyword,
	PreProcessor,
	Label
    };

    SyntaxHighlighter_CPP();
    virtual ~SyntaxHighlighter_CPP() {}
    void process( QTextDocument *doc, QTextParag *string, int start, bool invalidate = TRUE );

    static QString keywords[];

private:
    QTextFormat *format( int id );
    void addFormat( int id, QTextFormat *f );
    void removeFormat( int id );

    void createFormats();

    QTextFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextFormat> formats;

};

#endif
