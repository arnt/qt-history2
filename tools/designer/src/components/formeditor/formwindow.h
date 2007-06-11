/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include "formeditor_global.h"
#include <formwindowbase_p.h>

// Qt
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSet>

class QDesignerDnDItemInterface;
class DomConnections;

class QWidget;
class QLabel;
class QTimer;
class QAction;
class QMenu;
class QUndoStack;
class QRubberBand;

namespace qdesigner_internal {

class FormEditor;
class FormWindowCursor;
class WidgetEditorTool;
class FormWindowWidgetStack;
class FormWindowManager;
class FormWindowDnDItem;
class SetPropertyCommand;
class BreakLayoutCommand;


class QT_FORMEDITOR_EXPORT FormWindow: public FormWindowBase
{
    Q_OBJECT

public:
    explicit FormWindow(FormEditor *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~FormWindow();

    virtual QDesignerFormEditorInterface *core() const;

    virtual QDesignerFormWindowCursorInterface *cursor() const;

    virtual int toolCount() const;
    virtual int currentTool() const;
    virtual void setCurrentTool(int index);
    virtual QDesignerFormWindowToolInterface *tool(int index) const;
    virtual void registerTool(QDesignerFormWindowToolInterface *tool);

    virtual QString author() const;
    virtual void setAuthor(const QString &author);

    virtual QString comment() const;
    virtual void setComment(const QString &comment);

    virtual void layoutDefault(int *margin, int *spacing);
    virtual void setLayoutDefault(int margin, int spacing);

    virtual void layoutFunction(QString *margin, QString *spacing);
    virtual void setLayoutFunction(const QString &margin, const QString &spacing);

    virtual QString pixmapFunction() const;
    virtual void setPixmapFunction(const QString &pixmapFunction);

    virtual QString exportMacro() const;
    virtual void setExportMacro(const QString &exportMacro);

    virtual QStringList includeHints() const;
    virtual void setIncludeHints(const QStringList &includeHints);

    virtual QString fileName() const;
    virtual void setFileName(const QString &fileName);

    virtual QString contents() const;
    virtual void setContents(const QString &contents);
    virtual void setContents(QIODevice *dev);

    virtual QDir absoluteDir() const;

    virtual void simplifySelection(QWidgetList *sel) const;

    virtual void ensureUniqueObjectName(QObject *object);

    virtual QWidget *mainContainer() const;
    void setMainContainer(QWidget *mainContainer);
    bool isMainContainer(const QWidget *w) const;

    QWidget *currentWidget() const;

    virtual QSize sizeHint() const;

    bool hasInsertedChildren(QWidget *w) const;

    QList<QWidget *> selectedWidgets() const;
    void clearSelection(bool changePropertyDisplay=true);
    bool isWidgetSelected(QWidget *w) const;
    void selectWidget(QWidget *w, bool select=true);

    void selectWidgets();
    void repaintSelection();
    void updateSelection(QWidget *w);
    void updateChildSelections(QWidget *w);
    void raiseChildSelections(QWidget *w);
    void raiseSelection(QWidget *w);
    void hideSelection(QWidget *w);

    inline const QList<QWidget *>& widgets() const { return m_widgets; }
    inline int widgetCount() const { return m_widgets.count(); }
    inline QWidget *widgetAt(int index) const { return m_widgets.at(index); }

    QList<QWidget *> widgets(QWidget *widget) const;

    QWidget *createWidget(DomUI *ui, const QRect &rect, QWidget *target);

    bool isManaged(QWidget *w) const;

    void manageWidget(QWidget *w);
    void unmanageWidget(QWidget *w);

    inline QUndoStack *commandHistory() const
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

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

    QStringList resourceFiles() const;
    void addResourceFile(const QString &path);
    void removeResourceFile(const QString &path);

    void resizeWidget(QWidget *widget, const QRect &geometry);

    bool dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list, QWidget *target,
                        const QPoint &global_mouse_pos);

    virtual QWidget *findContainer(QWidget *w, bool excludeLayout) const;
    // for WidgetSelection only.
    QWidget *designerWidget(QWidget *w) const;

    //  Initialize and return a popup menu for a managed widget
    QMenu *initializePopupMenu(QWidget *managedWidget);

    virtual void paste(PasteMode pasteMode);
    virtual QEditorFormBuilder *createFormBuilder();

    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void contextMenuRequested(QMenu *menu, QWidget *widget);

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

    void editContents();

protected:
    virtual QMenu *createPopupMenu(QWidget *w);
    virtual void resizeEvent(QResizeEvent *e);

    void insertWidget(QWidget *w, const QRect &rect, QWidget *target, bool already_in_form = false);

private slots:
    void selectionChangedTimerDone();
    void updateDirty();
    void checkSelection();
    void checkSelectionNow();

private:
    void init();
    void initializeCoreTools();

    int getValue(const QRect &rect, int key, bool size) const;
    int calcValue(int val, bool forward, bool snap, int snapOffset) const;
    QRect applyValue(const QRect &rect, int val, int key, bool size) const;

    bool frameNeeded(QWidget *w) const;

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
    
    bool setCurrentWidget(QWidget *currentWidget);
    bool trySelectWidget(QWidget *w, bool select);


    BreakLayoutCommand *breakLayoutCommand(QWidget *w);

    void setCursorToAll(const QCursor &c, QWidget *start);

    QPoint mapToForm(const QWidget *w, const QPoint &pos) const;
    bool canBeBuddy(QWidget *w) const;

    QWidget *findTargetContainer(QWidget *widget) const;

    bool isPageOfContainerWidget(const QWidget *widget) const;
    void clearMainContainer();

    static int widgetDepth(const QWidget *w);
    static bool isChildOf(const QWidget *c, const QWidget *p);

    void editWidgets();

    void updateWidgets();

    void handleArrowKeyEvent(int key, Qt::KeyboardModifiers modifiers);

private:
    QWidget *containerForPaste() const;

    FormEditor *m_core;
    FormWindowCursor *m_cursor;
    QWidget *m_mainContainer;
    QWidget *m_currentWidget;

    bool m_blockSelectionChanged;
    bool m_drawRubber;

    QPoint m_rectAnchor;
    QRect m_currRect;

    QWidgetList m_widgets;
    QSet<QWidget*> m_insertedWidgets;

    class Selection;
    Selection *m_selection;

    QPoint m_startPos;
   
    QUndoStack *m_commandHistory;

    QString m_fileName;

    typedef QPair<QPalette ,bool> PaletteAndFill;
    typedef QMap<QWidget*, PaletteAndFill> WidgetPaletteMap;
    WidgetPaletteMap m_palettesBeforeHighlight;

    QRubberBand *m_rubberBand;

    QTimer *m_selectionChangedTimer;
    QTimer *m_checkSelectionTimer;
    QTimer *m_geometryChangedTimer;

    int m_dirty;
    int m_lastIndex;

    FormWindowWidgetStack *m_widgetStack;
    WidgetEditorTool *m_widgetEditor;

    QStringList m_resourceFiles;

    QString m_comment;
    QString m_author;
    QString m_pixmapFunction;
    int m_defaultMargin, m_defaultSpacing;
    QString m_marginFunction, m_spacingFunction;
    QString m_exportMacro;
    QStringList m_includeHints;

    QList<SetPropertyCommand*> m_moveSelection;
    int m_lastUndoIndex;
    bool m_dblClicked;

private:
    friend class WidgetEditorTool;
};

}  // namespace qdesigner_internal

#endif // FORMWINDOW_H
