#include "../../src/kernel/qrichtext_p.h"

class Indent_CPP : public QTextIndent
{
public:
    Indent_CPP();
    void indent( QTextDocument *doc, QTextParag *parag, int *oldIndent, int *newIndent );

    static void tabify( QString &s );
    static void untabify( QString &s );

};

