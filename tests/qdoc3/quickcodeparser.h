/*
  quickcodeparser.h
*/

#ifndef QUICKCODEPARSER_H
#define QUICKCODEPARSER_H

#include "cppcodeparser.h"

class QuickCodeParser : public CppCodeParser
{
public:
    QuickCodeParser();
    ~QuickCodeParser();

    virtual void convertTree( const Tree *tree );

protected:
    virtual StringSet topicCommands();
    virtual Node *processTopicCommand( const QString& command,
				       const QString& arg, const Doc& doc );
};

#endif
