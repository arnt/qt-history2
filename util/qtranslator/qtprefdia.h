/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtprefdia.h#2 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTPREFDIA
#define QTPREFDIA

#include <qtabdialog.h>
#include <qstring.h>

class QTPreferences;

class QLineEdit;
class QListBox;
class QPushButton;
class QCheckBox;

/****************************************************************************
 *
 * Class: QTPrefDia
 *
 ****************************************************************************/

class QTPrefDia : public QTabDialog
{
    Q_OBJECT

public:
    QTPrefDia( QWidget *parent, const char *name, QTPreferences *prefs );

protected:
    enum ListEditAction {
        Add = 0,
        Change
    };

    void setupTabGeneral();
    void setupTabSources();
    void setupTabTranslation();

    void initTabGeneral();
    void initTabSources();
    void initTabTranslation();

    QTPreferences *preferences;

    QLineEdit *srcLineEdit, *extLineEdit, *dirLineEdit, *nameLineEdit;
    QPushButton *srcBrowse, *srcAdd, *srcDel,
        *extAdd, *extDel, *dirBrowse;
    QListBox *srcList, *extList;
    QCheckBox *foldersCheckBox;
    ListEditAction srcAction, extAction;

protected slots:
    void slotSrcLineEdit();
    void slotSrcBrowse();
    void slotSrcList( const QString &text );
    void slotSrcAdd();
    void slotSrcDel();
    void slotSrcListChanged( const QString & );
    void slotExtLineEdit();
    void slotExtList( const QString &text );
    void slotExtAdd();
    void slotExtDel();
    void slotExtListChanged( const QString & );

    void slotDirBrowse();

    void slotApplyButtonPressed();
    void slotHelpButtonPressed();
    
};

#endif
