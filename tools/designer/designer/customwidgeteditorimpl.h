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

#ifndef CUSTOMWIDGETEDITORIMPL_H
#define CUSTOMWIDGETEDITORIMPL_H

#include "customwidgeteditor.h"
#include "metadatabase.h"

#include <qmap.h>

class QTimer;
class QListBoxItem;
class MainWindow;

class CustomWidgetEditor : public CustomWidgetEditorBase
{
    Q_OBJECT

public:
    CustomWidgetEditor( QWidget *parent , MainWindow *mw);

protected slots:
    void addWidgetClicked();
    void classNameChanged( const QString & );
    void currentWidgetChanged( QListBoxItem * );
    void deleteWidgetClicked();
    void headerFileChanged( const QString & );
    void heightChanged( int );
    void includePolicyChanged( int );
    void pixmapChoosen();
    void widthChanged( int );
    void chooseHeader();
    void checkWidgetName();
    void closeClicked();
    void currentSignalChanged( QListBoxItem *i );
    void addSignal();
    void removeSignal();
    void signalNameChanged( const QString &s );
    void slotAccessChanged( const QString & );
    void slotNameChanged( const QString & );
    void addSlot();
    void removeSlot();
    void currentSlotChanged( QListViewItem * );
    void propertyTypeChanged( const QString & );
    void propertyNameChanged( const QString & );
    void addProperty();
    void removeProperty();
    void currentPropertyChanged( QListViewItem * );
    void saveDescription();
    void loadDescription();
    void horDataChanged( int );
    void verDataChanged( int );
    void widgetIsContainer( bool );

private:
    MetaDataBase::CustomWidget *findWidget( QListBoxItem *i );
    void setupDefinition();
    void setupSignals();
    void setupSlots();
    void setupProperties();
    void updateCustomWidgetSizes();

private:
    QMap<QListBoxItem*, MetaDataBase::CustomWidget*> customWidgets;
    QString oldName;
    QTimer *checkTimer;
    QListBoxItem *oldItem;
    MainWindow *mainWindow;
    QObjectList cwLst;

};

#endif
