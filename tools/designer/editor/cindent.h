#ifndef CINDENT_H
#define CINDENT_H

#include <qrichtext_p.h>
#include <qvaluestack.h>
#include "dlldefs.h"

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class EDITOR_EXPORT QValueStack<QChar>;
// MOC_SKIP_END
#endif

class EDITOR_EXPORT CIndent : public QTextIndent
{
public:
    CIndent();
    virtual ~CIndent() {}
    void indent( QTextDocument *doc, QTextParag *parag, int *oldIndent, int *newIndent );

    static void tabify( QString &s );
    static void untabify( QString &s );

private:
    int indentation( const QString &s ) const;
    void simplifyLine( QString &s ) const;
    void indentLine( QTextParag *p, int &oldIndent, int &newIndent ) const;
    bool isBlockStart( const QString &s ) const;
    void findBlockStart( QTextParag **p, QString &s, int &i, const QChar &c, const QChar &opposite );
    void indentBlock( QTextParag **p, int &currIndent );
    void indentParenBlock( QTextParag **p, int &currIndent, int &i );

private:
    QTextParag *parag, *startParag;
    int extraIndent;
    bool caseDone;
    QValueStack<QChar> curClose;

};

#endif
