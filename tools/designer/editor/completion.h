 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef COMPLETION_H
#define COMPLETION_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qmap.h>
#include "dlldefs.h"

class QTextDocument;
class Editor;
class QVBox;
class QListBox;
class QLabel;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class EDITOR_EXPORT QMap<QChar, QStringList>;
// MOC_SKIP_ENDI
#endif

class EDITOR_EXPORT EditorCompletion : public QObject
{
    Q_OBJECT

public:
    EditorCompletion( Editor *e );

    virtual void addCompletionEntry( const QString &s, QTextDocument *doc );
    virtual QStringList completionList( const QString &s, QTextDocument *doc ) const;
    virtual void updateCompletionMap( QTextDocument *doc );

    bool eventFilter( QObject *o, QEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual bool doCompletion();
    virtual bool doObjectCompletion();
    virtual bool doObjectCompletion( const QString &object );
    virtual bool doArgumentHint( bool useIndex );

    virtual void addEditor( Editor *e );
    virtual QStringList functionParameters( const QString &func, QChar & );

    virtual void setContext( QObjectList *toplevels, QObject *this_ );

protected:
    virtual bool continueComplete();
    virtual void showCompletion( const QStringList &lst );

protected:
    QVBox *completionPopup;
    QListBox *completionListBox;
    QLabel *functionLabel;
    int completionOffset;
    Editor *curEditor;
    QString searchString;
    QStringList cList;
    QMap<QChar, QStringList> completionMap;

};

#endif
