/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/


void ControlInfo::setControl( QAxWidget *activex )
{
    listInfo->clear();

    const QMetaObject *mo = activex->metaObject();
    Q3ListViewItem *item = new Q3ListViewItem(listInfo, "Class Info", QString::number(mo->classInfoCount()));
    int i;
    int count;
    for (i = mo->classInfoOffset(); i < mo->classInfoCount(); ++i) {
	const QMetaClassInfo info = mo->classInfo(i);
	(void)new Q3ListViewItem(item, info.name(), info.value());
    }
    item = new Q3ListViewItem(listInfo, "Signals", QString());
    count = 0;
    for (i = mo->memberOffset(); i < mo->memberCount(); ++i) {
	const QMetaMember member = mo->member(i);
        if (member.memberType() == QMetaMember::Signal) {
            ++count;
	    (void)new Q3ListViewItem(item, member.signature());
        }
    }
    item->setText(2, QString::number(count));

    item = new Q3ListViewItem(listInfo, "Slots", QString());
    count = 0;
    for (i = mo->memberOffset(); i < mo->memberCount(); ++i) {
	const QMetaMember member = mo->member(i);
        if (member.memberType() == QMetaMember::Slot) {
            ++count;
	    (void)new Q3ListViewItem(item, member.signature());
        }
    }
    item->setText(2, QString::number(count));

    item = new Q3ListViewItem(listInfo, "Properties", QString::number(mo->propertyCount()));    
    for (i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
	const QMetaProperty property = mo->property(i);
	(void)new Q3ListViewItem(item, property.name(), property.typeName());
    }
}
