#ifndef QCPPSYNTAXHIGHLIGHTER_H
#define QCPPSYNTAXHIGHLIGHTER_H

#include "qrichtext_p.h"

class QCppSyntaxHighlighter : public QTextPreProcessor
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

    QCppSyntaxHighlighter();
    virtual ~QCppSyntaxHighlighter() {}
    void process( QTextDocument *doc, QTextParag *string, int start, bool invalidate = TRUE );

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
    QCppIndent();
    void indent( QTextDocument *doc, QTextParag *parag, int *oldIndent, int *newIndent );

};

#endif
