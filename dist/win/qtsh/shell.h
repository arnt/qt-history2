#ifndef SHELL_H
#define SHELL_H

#include <qstring.h>

class Shell
{
public:
    enum Error {
	NoCommand,
	RmTooFewArguments,
	MvTooFewArguments,
	MvTooMuchArguments,
	MvFileDoesNotExist,
	CpTooFewArguments,
	CpTooMuchArguments,
	CpFileDoesNotExist,
	UnknownCommand
    };
    
    enum Warning {
	RmCannotOpenFile,
	RmFileDoesNotExist
    };
    
    Shell();
    
private:
    void parse();
    void parseMove();
    void parseCopy();
    void parseRemove();
    void removeFiles( const QString &f );
    void removeFile( const QString &f );

    void error( Error e, const QString &msg = QString::null );    
    void warning( Warning w, const QString &msg = QString::null );
    
};

#endif
