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

#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include "formeditor_global.h"
#include "formeditor.h"

// sdk
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractformwindow.h>

// Qt
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSet>

#include <QtGui/QWidget>
#include <QtGui/QPixmap>

class DomConnections;
class Connection;

class QLabel;
class QTimer;
class QAction;
class QMenu;
class QtUndoStack;
class QRubberBand;
class BreakLayoutCommand;

namespace qdesigner { namespace components { namespace formeditor {

class FormEditor;
class FormWindowCursor;
class WidgetSelection;
class WidgetEditorTool;
class FormWindowWidgetStack;
class FormWindowManager;
class FormWindowDnDItem;


// ### fake - remove when actions are implemented
class QT_FORMEDITOR_EXPORT WidgetToActionMap
{
public:
    void add(QWidget *w, const QString &action_name)
        { m_map[w].append(action_name); }
    QStringList actions(QWidget *w)
        { return m_map.value(w, QStringList()); }
private:
    typedef QMap<QWidget*, QStringList> Map;
    Map m_map;
};

// ### fake - remove when actions are implemented
class QT_FORMEDITOR_EXPORT ActionListElt
{
public:
    QString name;
    QString objectName;
    QString menu;
    QString icon;
    QString iconText;
    QString shortcut;
};
typedef QList<ActionListElt> ActionList;

class QT_FORMEDITOR_EXPORT FormWindow: public QDesignerFormWindowInterface
{
    Q_OBJECT
public:
    enum HighlightMode
    {
        Restore,
        Highlight
    };

public:
    FormWindow(FormEditor *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~FormWindow();

    virtual QDesignerFormEditorInterface *core() const;

    virtual QDesignerFormWindowCursorInterface *cursor() const;

    virtual int toolCount() const;
    virtual int currentTool() const;
    virtual void setCurrentTool(int index);
    virtual QDesignerFormWindowToolInterface *tool(int index) const;
    virtual void registerTool(QDesignerFormWindowToolInterface *tool);

    virtual bool hasFeature(Feature f) const;
    virtual Feature features() const;
    virtual void setFeatures(Feature f);

    virtual QString author() const;
    virtual QString comment() const;
    virtual void setAuthor(const QString &author);
    virtual void setComment(const QString &comment);

    virtual QString fileName() const;
    virtual void setFileName(const QString &fileName);

    virtual QString contents() const;
    virtual void setContents(const QString &contents);
    virtual void setContents(QIODevice *dev);

    virtual QPoint grid() const { return m_grid; }
    virtual void setGrid(const QPoint &grid) { m_grid = grid; }

    virtual void simplifySelection(QList<QWidget*> *sel) const;

    virtual QWidget *mainContainer() const;
    void setMainContainer(QWidget *mainContainer);
    bool isMainContainer(const QWidget *w) const;

    QWidget *currentWidget() const;
    void setCurrentWidget(QWidget *currentWidget);

    virtual QSize sizeHint() const
    { return QSize(400, 300); }  /// ### remove me

    bool hasInsertedChildren(QWidget *w) const;

    QList<QWidget *> selectedWidgets() const;
    void clearSelection(bool changePropertyDisplay=true);
    bool isWidgetSelected(QWidget *w) const;
    void selectWidget(QWidget *w, bool select=true);

    void selectWidgets();
    void repaintSelection();
    void repaintSelection(QWidget *w);
    void updateSelection(QWidget *w);
    void updateChildSelections(QWidget *w);
    void raiseChildSelections(QWidget *w);
    void raiseSelection(QWidget *w);
    void hideSelection(QWidget *w);

    inline QList<QWidget *> widgets() const { return m_widgets; }
    inline int widgetCount() const { return m_widgets.count(); }
    inline QWidget *widgetAt(int index) const { return m_widgets.at(index); }

    QList<QWidget *> widgets(QWidget *widget) const;

    QWidget *createWidget(DomUI *ui, const QRect &rect, QWidget *target);
    void deleteWidgets(const QList<QWidget*> &widget_list);

    bool isManaged(QWidget *w) const;

    void manageWidget(QWidget *w);
    void unmanageWidget(QWidget *w);

    inline QtUndoStack *commandHistory() const
    { return m_commandHistory; }

    void beginCommand(const QString &description);
    void endCommand();

    bool blockSelectionChanged(bool blocked);
    void emitSelectionChanged();

    bool unify(QObject *w, QString &s, bool changeIt);

    bool isDirty() const;
    void setDirty(bool dirty);

    static FormWindow *findFormWindow(QWidget *w);

    virtual QWidget *containerAt(const QPoint &pos);
    virtual QWidget *widgetAt(const QPoint &pos);
    virtual void highlightWidget(QWidget *w, const QPoint &pos,
                                    HighlightMode mode = Highlight);

    void updateOrderIndicators();

    WidgetToActionMap &widgetToActionMap() { return m_widget_to_action_map; }
    ActionList &actionList() { return m_action_list; }

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

    QStringList resourceFiles() const;
    void addResourceFile(const QString &path);
    void removeResourceFile(const QString &path);

    void resizeWidget(QWidget *widget, const QRect &geometry);

    void dropWidgets(QList<QDesignerDnDItemInterface*> &item_list, QWidget *target,
                        const QPoint &global_mouse_pos);

signals:
    void showContextMenu(QWidget *w, const QPoint &pos);

public slots:
    void deleteWidgets();
    void raiseWidgets();
    void lowerWidgets();
    void copy();
    void cut();
    void paste();
    void selectAll();

    void layoutHorizontal();
    void layoutVertical();
    void layoutGrid();
    void layoutHorizontalSplit();
    void layoutVerticalSplit();
    void layoutHorizontalContainer(QWidget *w);
    void layoutVerticalContainer(QWidget *w);
    void layoutGridContainer(QWidget *w);
    void breakLayout(QWidget *w);

    void breakLayout();
    void editContents();

    QString relativePath(const QString &abs_path) const;
    QString absolutePath(const QString &rel_path) const;

protected:
    virtual QMenu *createPopupMenu(QWidget *w);
    virtual void resizeEvent(QResizeEvent *e);

    void insertWidget(QWidget *w, const QRect &rect, QWidget *target);

private slots:
    void selectionChangedTimerDone();
    void updateDirty();
    void checkSelection();
    void checkSelectionNow();

private:
    void init();
    void initializeCoreTools();

    QPoint gridPoint(const QPoint &p) const;

    enum RectType { Insert, Rubber };

    void startRectDraw(const QPoint &global, QWidget *, RectType t);
    void continueRectDraw(const QPoint &global, QWidget *, RectType t);
    void endRectDraw();

    QWidget *containerAt(const QPoint &pos, QWidget *notParentOf);

    void checkPreviewGeometry(QRect &r);

    void finishContextMenu(QWidget *w, QWidget *menuParent, QContextMenuEvent *e);

    bool handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e);
    bool handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
    bool handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);
    bool handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);

    bool isCentralWidget(QWidget *w) const;
    QWidget *designerWidget(QWidget *w) const;

    BreakLayoutCommand *breakLayoutCommand(QWidget *w);

    void setCursorToAll(const QCursor &c, QWidget *start);
    void restoreCursors(QWidget *start, FormWindow *fw);

    QPoint mapToForm(const QWidget *w, const QPoint &pos) const;
    bool canBeBuddy(QWidget *w) const;

    QWidget *findContainer(QWidget *w, bool excludeLayout) const;
    QWidget *findTargetContainer(QWidget *widget) const;

    static int widgetDepth(QWidget *w);
    static bool isChildOf(QWidget *c, const QWidget *p);

    void editWidgets();

    void updateWidgets();
    void bfs(QWidget *widget);

private:
    Feature m_feature;
    FormEditor *m_core;
    FormWindowCursor *m_cursor;
    QWidget *m_mainContainer;
    QWidget *m_currentWidget;
    QPoint m_grid;

    uint m_blockSelectionChanged: 1;
    uint drawRubber: 1;
    uint oldRectValid: 1;
    uint hadOwnPalette: 1;
    uint pad[28];

    QPoint rectAnchor;
    QRect currRect;

    QList<QWidget*> m_widgets;
    QSet<QWidget*> m_insertedWidgets;

    QList<WidgetSelection *> selections;
    QHash<QWidget *, WidgetSelection *> usedSelections;

    QPoint startPos;
    QPoint currentPos;

    QRect widgetGeom;
    QPoint oldPressPos;
    QPoint origPressPos;
    QWidget *startWidget;
    QWidget *endWidget;

    QWidget *targetContainer;
    QPalette restorePalette;

    QtUndoStack *m_commandHistory;

    QString m_fileName;
    QString pixLoader;

    QList<QWidget*> orderedWidgets;
    QList<QWidget*> stackedWidgets;

    QMap<QWidget*, QPalette> palettesBeforeHighlight;

    QRubberBand *m_rubberBand;

    QTimer *m_selectionChangedTimer;
    QTimer *m_checkSelectionTimer;
    QTimer *m_geometryChangedTimer;

    int m_dirty;
    int m_lastIndex;

    WidgetToActionMap m_widget_to_action_map;
    ActionList m_action_list;

    QString m_comment;
    QString m_author;

    FormWindowWidgetStack *m_widgetStack;
    WidgetEditorTool *m_widgetEditor;

    QStringList m_resourceFiles;

private:
//    friend class FormWindowManager;
    friend class WidgetHandle;
    friend class WidgetSelection;
    friend class QDesignerWidget;
    friend class WidgetEditorTool;
};

} } } // namespace qdesigner::components::formeditor

#endif // FORMWINDOW_H
