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

#include <q3mainwindow.h>
#include <qintdict.h>
#include <qptrlist.h>
#include "categoryinterface.h"

class QToolBox;
class QStyle;
class QWidgetStack;

class Frame : public Q3MainWindow
{
    Q_OBJECT

public:
    Frame( QWidget *parent=0, const char *name=0 );
    void setCategories( const QPtrList<CategoryInterface> &l );

    static void updateTranslators();

    QWidgetStack *widgetStack() const { return stack; }

private slots:
    void setStyle( const QString& );

protected:
    bool event( QEvent *e );

private:
    QWidget *createCategoryPage( CategoryInterface *c );

private:
    QToolBox *toolBox;
    QWidgetStack *stack;
    QIntDict<QWidget> categoryPages;
    QPtrList<CategoryInterface> categories;

};
