/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FORMFILE_H
#define FORMFILE_H

#include <qobject.h>
#include "timestamp.h"
#include "metadatabase.h"

class Project;
class FormWindow;
class SourceEditor;

class FormFile : public QObject
{
    Q_OBJECT
    friend class SourceEditor;

public:
    enum Who {
	WFormWindow = 1,
	WFormCode = 2,
	WAnyOrAll = WFormWindow | WFormCode
    };

    enum UihState { None, Ok, Deleted };

    FormFile( const QString &fn, bool temp, Project *p, const char *name = 0 );
    ~FormFile();

    void setFormWindow( FormWindow *f );
    void setEditor( SourceEditor *e );
    void setFileName( const QString &fn );
    void setCode( const QString &c );
    void setModified( bool m, int who = WAnyOrAll );
    void setCodeEdited( bool b );

    FormWindow *formWindow() const;
    SourceEditor *editor() const;
    QString fileName() const;
    QString absFileName() const;

    bool supportsCodeFile() const { return !codeExtension().isEmpty(); }
    QString codeComment() const;
    QString codeFile() const;
    QString code();
    bool isCodeEdited() const;

    bool loadCode();
    void load();
    bool save( bool withMsgBox = TRUE, bool ignoreModified = FALSE );
    bool saveAs( bool ignoreModified = FALSE );
    bool close();
    bool closeEvent();
    bool isModified( int who = WAnyOrAll );
    bool hasFormCode() const;
    void createFormCode();
    void syncCode();
    void checkTimeStamp();
    bool isUihFileUpToDate();
    void addFunctionCode( MetaDataBase::Function function );
    void removeFunctionCode( MetaDataBase::Function function );
    void functionNameChanged( const QString &oldName, const QString &newName );
    void functionRetTypeChanged( const QString &fuName, const QString &oldType, const QString &newType );

    void showFormWindow();
    SourceEditor *showEditor( bool askForUih = TRUE );

    static QString createUnnamedFileName();
    QString formName() const;

    bool isFake() const { return fake; }

    void parseCode( const QString &txt, bool allowModify );

    void addConnection( const QString &sender, const QString &signal,
			const QString &receiver, const QString &slot );
    void removeConnection( const QString &sender, const QString &signal,
			   const QString &receiver, const QString &slot );

    bool hasTempFileName() const { return fileNameTemp; }
    void setCodeFileState( UihState );
    int codeFileState() const;
    bool setupUihFile( bool askForUih = TRUE );

    Project *project() const { return pro; }

signals:
    void somethingChanged( FormFile* );

private slots:
    void formWindowChangedSomehow();
    void notifyFormWindowChange();

private:
    bool isFormWindowModified() const;
    bool isCodeModified() const;
    void setFormWindowModified( bool m );
    void setCodeModified( bool m );
    QString codeExtension() const;
    bool checkFileName( bool allowBreak );

private:
    QString filename;
    bool fileNameTemp;
    Project *pro;
    FormWindow *fw;
    SourceEditor *ed;
    QString cod;
    TimeStamp timeStamp;
    bool codeEdited;
    QString cachedFormName;
    bool fake;
    bool pkg;
    bool cm;
    UihState codeFileStat;

};

#endif
