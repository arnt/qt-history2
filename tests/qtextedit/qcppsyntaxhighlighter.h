#ifndef QCPPSYNTAXHIGHLIGHTER_H
#define QCPPSYNTAXHIGHLIGHTER_H

#include "qtexteditintern_p.h"

class QCppSyntaxHighlighter : public QTextEditSyntaxHighlighter
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

    QCppSyntaxHighlighter( QTextEditDocument *d );
    virtual ~QCppSyntaxHighlighter() {}
    void highlighte( QTextEditParag *string, int start, bool invalidate = TRUE );

private:
    QTextEditFormat *format( int id );
    void addFormat( int id, QTextEditFormat *f );
    void removeFormat( int id );

    void createFormats();

    QTextEditFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextEditFormat> formats;

};

class QCppIndent : public QTextEditIndent
{
public:
    QCppIndent( QTextEditDocument *d );
    void indent( QTextEditParag *parag, int *oldIndent, int *newIndent );

};

#endif
