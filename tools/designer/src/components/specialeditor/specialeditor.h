/*
    specialeditor.h
*/

#ifndef SPECIALEDITOR_H
#define SPECIALEDITOR_H

#include <extension.h>
#include <QObject>

class QWidget;

class ISpecialEditor
{
public:
    virtual ~ISpecialEditor() {}

    virtual QWidget *createEditor(QWidget *parent) = 0;
    virtual void applyChanges() = 0;
    virtual void revertChanges() = 0;
};

Q_DECLARE_EXTENSION_INTERFACE(ISpecialEditor, "http://trolltech.com/Qt/IDE/SpecialEditor")
#endif
