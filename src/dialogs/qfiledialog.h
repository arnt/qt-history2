/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledialog.h#10 $
**
** Definition of QFileDialog class
**
** Created : 950428
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFILEDLG_H
#define QFILEDLG_H

#include "qdir.h"
#include "qdialog.h"

class QListBox;
class QLineEdit;
class QComboBox;
class QLabel;
class QPushButton;


class QFileDialog : public QDialog
{
    Q_OBJECT
public:
    QFileDialog( const char *dirName, const char *filter = 0,
		 QWidget *parent=0, const char *name=0, bool modal=FALSE );
    QFileDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE );
   ~QFileDialog();

    QString	selectedFile()	const;

    const char *dirPath() const;
    void	setDir( const char * );
    const QDir *dir() const;
    void	setDir( const QDir & );

    void	rereadDir();

    static QString getOpenFileName( const char *dir = 0, const char *filter= 0,
				    QWidget *parent = 0, const char *name = 0);
    static QString getSaveFileName( const char *dir = 0, const char *filter= 0,
				    QWidget *parent = 0, const char *name = 0);

signals:
    void	fileHighlighted( const char * );
    void	fileSelected( const char * );
    void	dirEntered( const char * );

private slots:
    void	fileSelected( int );
    void	fileHighlighted( int );
    void	dirSelected( int );
    void	pathSelected( int );

    void	okClicked();
    void	filterClicked();
    void	cancelClicked();

protected:
    void	resizeEvent( QResizeEvent * );

private:
    void	init();
    void	updatePathBox( const char * );

    QDir	d;
    QString	fileName;

    QListBox   *files;
    QListBox   *dirs;
    QLineEdit  *filterEdit;
    QLineEdit  *nameEdit;
    QComboBox  *pathBox;
    QLabel     *filterL;
    QLabel     *nameL;
    QLabel     *dirL;
    QLabel     *fileL;
    QPushButton *okB;
    QPushButton *filterB;
    QPushButton *cancelB;

private:	// Disabled copy constructor and operator=
    QFileDialog( const QFileDialog & ) {}
    QFileDialog &operator=( const QFileDialog & ) { return *this; }
};


#endif // QFILEDLG_H
