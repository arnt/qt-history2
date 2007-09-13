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

/*
    javacodemarker.h
*/

#ifndef JAVACODEMARKER_H
#define JAVACODEMARKER_H

#include "codemarker.h"

QT_BEGIN_NAMESPACE

class JavaCodeMarker : public CodeMarker
{
public:
    JavaCodeMarker();
    ~JavaCodeMarker();

    bool recognizeCode( const QString& code );
    bool recognizeExtension( const QString& ext );
    bool recognizeLanguage( const QString& lang );
    QString plainName(const Node *node);
    QString plainFullName(const Node *node, const Node *relative);
    QString markedUpCode( const QString& code, const Node *relative,
        		  const QString& dirPath );
    QString markedUpSynopsis( const Node *node, const Node *relative,
        		      SynopsisStyle style );
    QString markedUpName( const Node *node );
    QString markedUpFullName( const Node *node, const Node *relative );
    QString markedUpEnumValue(const QString &enumValue, const Node *relative);
    QString markedUpIncludes( const QStringList& includes );
    QList<Section> sections(const InnerNode *innerNode, SynopsisStyle style, Status status);
    QString functionBeginRegExp( const QString& funcName );
    QString functionEndRegExp( const QString& funcName );
    const Node *resolveTarget( const QString& target, const Tree *tree, const Node *relative );
};

QT_END_NAMESPACE

#endif
