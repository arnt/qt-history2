#include "qspellchecker.h"

#include <qfile.h>

QSpellChecker::QSpellChecker()
    : QTextSyntaxHighlighter()
{
    dawg = new QDawg;
    QFile dawgfile("udw.dawg");
    bool dump = FALSE;

    if ( !dawgfile.exists() ) {
	QString fn = "/usr/dict/words";
	QFile in(fn);
	in.open(IO_ReadOnly);
	dawg->createFromWords(&in);
	dawgfile.open(IO_WriteOnly);
	dawg->write(&dawgfile);
	dawgfile.close();
    } else {
	dawgfile.open(IO_ReadOnly);
	dawg->read(&dawgfile);
    }
    if ( dump )
	dawg->dump();

}

bool QSpellChecker::checkWord( const QString &word, int i, QTextParag *string )
{
    QTextFormat f;
    switch ( findWordState( dawg->root(), word ) ) {
    case Acceptable:
	f.setColor( Qt::black );
	string->setFormat( i - word.length(), word.length(), &f, TRUE, QTextFormat::Color );
	break;
    case Intermediate:
	f.setColor( Qt::blue );
	string->setFormat( i - word.length(), word.length(), &f, TRUE, QTextFormat::Color );
	break;
    case Invalid:
	f.setColor( Qt::red );
	string->setFormat( i - word.length(), word.length(), &f, TRUE, QTextFormat::Color );
	return FALSE;
	break;
    }
    return TRUE;
}

void QSpellChecker::highlighte( QTextDocument *doc, QTextParag *string, int, bool )
{
    QString s = string->string()->toString();
    QChar c;
    QString lastWord;

    for ( int i = 0; i < (int)s.length(); ++i ) {
	c = s[ i ];
	if ( c.isPunct() || c.isSpace() || c.isMark() || c.isNumber() || c.isDigit() || c.isNull() ) {
	    if ( lastWord.isEmpty() ) {
		lastWord = QString::null;
		continue;
	    }
	    if ( !checkWord( lastWord, i, string ) )
		checkWord( lastWord.lower(), i, string );
	    lastWord = QString::null;
	} else {
	    lastWord += c;
	}
    }

    string->setFirstHighlighte( FALSE );
    string->setEndState( 0 );
}

QSpellChecker::State QSpellChecker::findWordState(const QDawg::Node* n, const QString& s, int index ) const
{
    if ( s[index] == ' ' )
	return findWordState(dawg->root(), s, index+1);
    if ( n ) {
	if ( s[index] == n->letter() ) {
	    if ( index == (int)s.length()-1 ) {
		return n->isWord() ? Acceptable : Intermediate;
	    }
	    return findWordState(n->jump(), s, index+1);
	} else {
	    return findWordState(n->next(), s, index);
	}
    }
    return index == 0 || s[index-1] == ' ' ? Acceptable : Invalid;
}
