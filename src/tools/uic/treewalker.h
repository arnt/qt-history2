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

#ifndef TREEWALKER_H
#define TREEWALKER_H

class DomUI;
class DomLayoutDefault;
class DomLayoutFunction;
class DomTabStops;
class DomLayout;
class DomLayoutItem;
class DomWidget;
class DomSpacer;
class DomColor;
class DomColorGroup;
class DomPalette;
class DomFont;
class DomPoint;
class DomRect;
class DomSizePolicy;
class DomSize;
class DomDate;
class DomTime;
class DomDateTime;
class DomProperty;
class DomCustomWidgets;
class DomCustomWidget;
class DomAction;
class DomActionGroup;
class DomActionRef;
class DomImages;
class DomImage;
class DomItem;
class DomIncludes;
class DomInclude;
class DomString;
class DomResourcePixmap;
class DomResources;
class DomResource;

struct TreeWalker
{
    inline virtual ~TreeWalker() {}

    virtual void accept(DomUI *ui);
    virtual void accept(DomLayoutDefault *layoutDefault);
    virtual void accept(DomLayoutFunction *layoutFunction);
    virtual void accept(DomTabStops *tabStops);
    virtual void accept(DomCustomWidgets *customWidgets);
    virtual void accept(DomCustomWidget *customWidget);
    virtual void accept(DomLayout *layout);
    virtual void accept(DomLayoutItem *layoutItem);
    virtual void accept(DomWidget *widget);
    virtual void accept(DomSpacer *spacer);
    virtual void accept(DomColor *color);
    virtual void accept(DomColorGroup *colorGroup);
    virtual void accept(DomPalette *palette);
    virtual void accept(DomFont *font);
    virtual void accept(DomPoint *point);
    virtual void accept(DomRect *rect);
    virtual void accept(DomSizePolicy *sizePolicy);
    virtual void accept(DomSize *size);
    virtual void accept(DomDate *date);
    virtual void accept(DomTime *time);
    virtual void accept(DomDateTime *dateTime);
    virtual void accept(DomProperty *property);
    virtual void accept(DomImages *images);
    virtual void accept(DomImage *image);
    virtual void accept(DomIncludes *includes);
    virtual void accept(DomInclude *incl);
    virtual void accept(DomAction *action);
    virtual void accept(DomActionGroup *actionGroup);
    virtual void accept(DomActionRef *actionRef);

};


#endif // TREEWALKER_H
