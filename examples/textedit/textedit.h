/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <qmainwindow.h>
#include <qmap.h>

class QAction;
class QComboBox;
class QTabWidget;
class QTextEdit;
class QTextCharFormat;
class QMenu;

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    TextEdit( QWidget *parent = 0, const char *name = 0 );

private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    void load( const QString &f );
    QTextEdit *currentEditor() const;

private slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void filePrint();
    void fileClose();
    void fileExit();

    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily( const QString &f );
    void textSize( const QString &p );
    void textStyle( int s );
    void textColor();
    void textAlign( QAction *a );

    void editorChanged();

    void currentCharFormatChanged(const QTextCharFormat &format);

private:
    void fontChanged( const QFont &f );
    void colorChanged( const QColor &c );
    void alignmentChanged( Qt::Alignment a );

    QTextEdit *createNewEditor(const QString &title = QString::null);

    QAction *actionTextBold,
	*actionTextUnderline,
	*actionTextItalic,
	*actionTextColor,
	*actionAlignLeft,
	*actionAlignCenter,
	*actionAlignRight,
	*actionAlignJustify,
	*actionInsertTableRow,
	*actionInsertTableColumn,
	*actionDeleteTableRow,
	*actionDeleteTableColumn;

    QMenu *editMenu;
    QToolBar *editToolBar;

    QComboBox *comboStyle,
	*comboFont,
	*comboSize;
    QTabWidget *tabWidget;
    QMap<QTextEdit*, QString> filenames;

};


#endif
