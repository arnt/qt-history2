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

#ifndef ABSTRACTFORMWINDOW_H
#define ABSTRACTFORMWINDOW_H

#include "sdk_global.h"

#include <QWidget>

class AbstractFormEditor;
class AbstractFormWindowCursor;
class DomUI;
class QtUndoStack;

class QT_SDK_EXPORT AbstractFormWindow: public QWidget
{
    Q_OBJECT
public:
    enum EditMode
    {
        WidgetEditMode,
        ConnectionEditMode,
        TabOrderEditMode,
        BuddyEditMode
#ifdef DESIGNER_VIEW3D
        ,View3DEditMode
#endif
    };

    enum FeatureFlag
    {
        EditFeature = 0x01,
        GridFeature = 0x02,
        TabOrderFeature = 0x04,
        DefaultFeature = EditFeature | GridFeature
    };
    Q_DECLARE_FLAGS(Feature, FeatureFlag)

public:
    AbstractFormWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~AbstractFormWindow();

    virtual bool hasFeature(Feature f) const = 0;
    virtual Feature features() const = 0;

    virtual AbstractFormEditor *core() const;

    virtual AbstractFormWindowCursor *cursor() const = 0;

    virtual QString fileName() const = 0;

    QString author() const { return m_author; }
    QString comment() const { return m_comment; }
    void setAuthor(const QString &author) { m_author = author; }
    void setComment(const QString &comment) { m_comment = comment; }

    virtual QString contents() const = 0;
    virtual void setContents(QIODevice *dev) = 0;

    virtual QPoint grid() const = 0;

    virtual QWidget *mainContainer() const = 0;
    virtual void setMainContainer(QWidget *mainContainer) = 0;

    virtual bool isManaged(QWidget *widget) const = 0;

    virtual bool isDirty() const = 0;

    static AbstractFormWindow *findFormWindow(QWidget *w);

    virtual void setEditMode(EditMode mode) = 0;
    virtual EditMode editMode() const = 0;

    virtual QtUndoStack *commandHistory() const = 0;
    virtual void beginCommand(const QString &description) = 0;
    virtual void endCommand() = 0;

public slots:
    virtual void setFeatures(Feature f) = 0;
    virtual void setDirty(bool dirty) = 0;
    virtual void clearSelection(bool changePropertyDisplay = true) = 0;
    virtual void selectWidget(QWidget *w, bool select = true) = 0;
    virtual void setGrid(const QPoint &grid) = 0;
    virtual void setFileName(const QString &fileName) = 0;
    virtual void setContents(const QString &contents) = 0;

signals:
    void selectionChanged();
    void changed();
    void widgetManaged(QWidget *widget);
    void widgetUnmanaged(QWidget *widget);
    void aboutToUnmanageWidget(QWidget *widget);
    void activated(QWidget *widget);
    void featureChanged(Feature f);
    void widgetRemoved(QWidget *w);
    void widgetsChanged();

private:
    QString m_comment, m_author;
};

#endif // ABSTRACTFORMWINDOW_H
