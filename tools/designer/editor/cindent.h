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

};

#endif
