#ifndef QCPPSYNTAXHIGHLIGHTER_H
#define QCPPSYNTAXHIGHLIGHTER_H

#include "qrichtext_p.h"

class QCppSyntaxHighlighter : public QTextSyntaxHighlighter
{
public:
    enum CppIds {
	Comment = 1,
	Number,
	String,
	Type,
	Keyword,
	PreProcessor
    };

    QCppSyntaxHighlighter( QTextDocument *d );
    virtual ~QCppSyntaxHighlighter() {}
    void highlighte( QTextParag *string, int start, bool invalidate = TRUE );

private:
    QTextFormat *format( int id );
    void addFormat( int id, QTextFormat *f );
    void removeFormat( int id );

    void createFormats();

    QTextFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextFormat> formats;

};

class QCppIndent : public QTextIndent
{
public:
    QCppIndent( QTextDocument *d );
    void indent( QTextParag *parag, int *oldIndent, int *newIndent );

};

#endif
