/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qmainwindow.h>
#include <qhash.h>
#include <qlist.h>
#include "categoryinterface.h"

class QToolBox;
class QStyle;
class QWidgetStack;

class Frame : public QMainWindow
{
    Q_OBJECT

public:
    Frame( QWidget *parent=0, const char *name=0 );
    void setCategories( const QList<CategoryInterface *> &l );

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
    QHash<int, QWidget *> categoryPages;
    QList<CategoryInterface *> categories;

};
