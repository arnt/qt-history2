#ifndef CINDENT_H
#define CINDENT_H

#include <qrichtext_p.h>
#include "dlldefs.h"

class EDITOR_EXPORT CIndent : public QTextIndent
{
public:
    CIndent();
    virtual ~CIndent() {}
    void indent( QTextDocument *doc, QTextParag *parag, int *oldIndent, int *newIndent );

    void setTabSize( int ts );
    void setIndentSize( int is );
    void setAutoIndent( bool ai ) { autoIndent = ai; reindent(); }
    void setKeepTabs( bool kt ) { keepTabs = kt; reindent(); }

private:
    void reindent();
    void indentLine( QTextParag *p, int &oldIndent, int &newIndent );
    void tabify( QString &s );

public:
    int tabSize, indentSize;
    bool autoIndent, keepTabs;

};

#endif
