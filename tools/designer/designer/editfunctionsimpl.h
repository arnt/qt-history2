/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef EDITFUNCTIONSIMPL_H
#define EDITFUNCTIONSIMPL_H

#include "editfunctions.h"
#include "hierarchyview.h"
#include "metadatabase.h"
#include <qmap.h>

class FormWindow;
class QListViewItem;

class EditFunctions : public EditFunctionsBase
{
    Q_OBJECT

public:
    EditFunctions( QWidget *parent, FormWindow *fw, bool showOnlySlots = FALSE );

    void setCurrentFunction( const QString &function );
    void functionAdd( const QString &access = QString::null,
		      const QString &type = QString::null  );
    void functionAdd() { functionAdd( "public" ); }

signals:
    void itemRenamed(const QString &);

protected slots:
    void okClicked();
    void functionRemove();
    void currentItemChanged( QListViewItem * );
    void currentTextChanged( const QString &txt );
    void currentSpecifierChanged( const QString &s );
    void currentAccessChanged( const QString &a );
    void currentReturnTypeChanged( const QString &type );
    void currentTypeChanged( const QString &type );
    void displaySlots( bool justSlots );
    void emitItemRenamed( QListViewItem *, int, const QString & );

private:
    enum Attribute { Name, Specifier, Access, ReturnType, Type };
    struct FunctItem {
	int id;
	QString oldName;
	QString newName;
	QString oldRetTyp;
	QString retTyp;
	QString spec;
	QString oldSpec;
	QString access;
	QString oldAccess;
	QString type;
	QString oldType;

	Q_DUMMY_COMPARISON_OPERATOR( FunctItem )
    };

    void changeItem( QListViewItem *item, Attribute a, const QString &nV );

    FormWindow *formWindow;
    QMap<QListViewItem*, int> functionIds;
    QStringList removedFunctions;
    QList<MetaDataBase::Function> itemList;
    QList<FunctItem> functList;
    int id;
    QString lastType;
};

#endif
