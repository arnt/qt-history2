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
    void functionAdd() { functionAdd( "public", "function" ); }

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

private:
    enum Attribute { Name, Specifier, Access, ReturnType, Type };
    struct FunctItem {
	int id;
	QString oldName;
	QString newName;
	QString oldRetTyp;
	QString retTyp;
	QString spec;
	QString access;
	QString type;
    };

    void changeItem( QListViewItem *item, Attribute a, const QString &nV );

    FormWindow *formWindow;
    QMap<QListViewItem*, int> functionIds;
    QStringList removedFunctions;
    QValueList<MetaDataBase::Function> itemList;
    QValueList<FunctItem> functList;
    int id;
};

#endif
