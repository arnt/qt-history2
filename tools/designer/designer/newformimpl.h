/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef NEWFORMIMPL_H
#define NEWFORMIMPL_H

#include "newform.h"
#include <qiconview.h>

class Project;

class NewItem : public QIconViewItem
{
public:
    enum Type {
	ProjectType,
	Form,
	CustomForm,
	SourceFileType,
	SourceTemplateType
    };

    NewItem( QIconView *view, const QString &text )
	: QIconViewItem( view, text ) {}
    virtual void insert( Project *pro ) = 0;
    virtual void setProject( Project * ) {}

};

class ProjectItem : public NewItem
{
public:
    ProjectItem( QIconView *view, const QString &text );
    void insert( Project *pro );
    int rtti() const { return (int)ProjectType; }

    void setLanguage( const QString &l ) { lang = l; }
    QString language() const { return lang; }

private:
    QString lang;

};

class FormItem : public NewItem
{
public:
    enum FormType {
	Widget,
	Dialog,
	Wizard,
	MainWindow
    };

    FormItem( QIconView *view, const QString &text );
    void insert( Project *pro );
    int rtti() const { return (int)Form; }

    void setFormType( FormType ft ) { fType = ft; }
    FormType formType() const { return fType; }

private:
    FormType fType;

};

class CustomFormItem : public NewItem
{
public:
    CustomFormItem( QIconView *view, const QString &text );
    void insert( Project *pro );
    int rtti() const { return (int)CustomForm; }

    void setTemplateFile( const QString &tf ) { templFile = tf; }
    QString templateFileName() const { return templFile; }

private:
    QString templFile;

};

class SourceFileItem : public NewItem
{
public:
    SourceFileItem( QIconView *view, const QString &text );
    void insert( Project *pro );
    int rtti() const { return (int)SourceFileType; }

    void setExtension( const QString &e ) { ext = e; }
    QString extension() const { return ext; }
    void setLanguage( const QString &l ) { lang = l; }
    void setProject( Project *pro );

private:
    QString ext;
    QString lang;
    bool visible;

};

class SourceTemplateItem : public NewItem
{
public:
    SourceTemplateItem( QIconView *view, const QString &text );
    void insert( Project *pro );
    int rtti() const { return (int)SourceTemplateType; }

    void setTemplate( const QString &t ) { templ = t; }
    QString tenplate() const { return templ; }
    void setLanguage( const QString &l ) { lang = l; }
    void setProject( Project *pro );

private:
    QString templ;
    QString lang;
    bool visible;

};

class NewForm : public NewFormBase
{
    Q_OBJECT

public:
    NewForm( QIconView *templateView, const QString &templatePath );
    NewForm( QWidget *parent, const QStringList& projects,
	     const QString& currentProject, const QString &templatePath );
    void insertTemplates( QIconView*, const QString& );
    void accept();
    QList<QIconViewItem*> allViewItems();

protected slots:
    void projectChanged( const QString &project );
    void itemChanged( QIconViewItem *item );

private:
    QList<QIconViewItem*> allItems;

};

#endif
