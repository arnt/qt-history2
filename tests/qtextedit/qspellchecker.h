#ifndef QSPELLCHECKER_H
#define QSPELLCHECKER_H

#include "qrichtext_p.h"
#include "../../../qpim/words/qdawg.h"

class QSpellChecker : public QTextSyntaxHighlighter
{
public:
    QSpellChecker( QTextDocument *d );
    virtual ~QSpellChecker() {}
    void highlighte( QTextParag *string, int start, bool invalidate = TRUE );

private:
    QTextFormat *format( int ) { return 0; }
    enum State {
	Acceptable,
	Intermediate, 
	Invalid
    };
    State findWordState(const QDawg::Node* n, const QString& s, int index = 0 ) const;
    bool checkWord( const QString &word, int i, QTextParag *string );
    
    QTextFormat *invalidFormat;
    QDawg *dawg;
    
};

#endif
