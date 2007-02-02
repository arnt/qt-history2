/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef CSSHIGHLIGHTER_H
#define CSSHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "shared_global_p.h"

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT CssHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    CssHighlighter(QTextDocument *document);

protected:
    void highlightBlock(const QString&);
    void highlight(const QString&, int, int, int/*State*/);

private:
    enum State { Selector, Property, Value, Pseudo, Pseudo1, Pseudo2, Quote, 
                 MaybeComment, Comment, MaybeCommentEnd };
};

} // namespace qdesigner_internal

#endif // CSSHIGHLIGHTER_H
