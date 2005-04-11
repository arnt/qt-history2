#ifndef RICHTEXTEDITOR_H
#define RICHTEXTEDITOR_H

#include <QtGui/QTextEdit>
#include <QtGui/QDialog>
#include "shared_global.h"

class QToolBar;

class QT_SHARED_EXPORT RichTextEditor : public QTextEdit
{
    Q_OBJECT
public:
    RichTextEditor(QWidget *parent = 0);
    void setDefaultFont(const QFont &font);

    QToolBar *createToolBar(QWidget *parent = 0);

public slots:
    void setFontBold(bool b);
    void setFontPointSize(double);
    void setText(const QString &text);
    QString text(Qt::TextFormat format) const;

signals:
    void textChanged();

private:
    Qt::TextFormat detectFormat() const;
};

class QT_SHARED_EXPORT RichTextEditorDialog : public QDialog
{
    Q_OBJECT
public:
    RichTextEditorDialog(QWidget *parent = 0);
    RichTextEditor *editor();

private:
    RichTextEditor *m_editor;
};

#endif // RITCHTEXTEDITOR_H
