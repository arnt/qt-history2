/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_H
#endif // QT_H

#if defined(Q_CC_BOR) && !defined(QT_QWINEXPORT)
#undef Q_TEMPLATEDLL
#endif

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN

#if defined(Q_DEFINED_QASCIIDICT) && defined(Q_DEFINED_QCONNECTION_LIST) && !defined(Q_EXPORTED_QASCIIDICT_TEMPLATES)
#define Q_EXPORTEE_QASCIIDICT_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QAsciiDictIterator<QConnectionList>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QAsciiDict<QConnectionList>;
#endif

#if defined(Q_DEFINED_QSTYLESHEET) && defined(Q_DEFINED_QDICT) && !defined(Q_EXPORTED_QSTYLESHEET_TEMPLATES)
#define Q_EXPORTED_QSTYLESHEET_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QDict<QStyleSheetItem>;
#endif

#if defined(Q_DEFINED_QLIBRARY) && defined(Q_DEFINED_QDICT) && !defined(Q_EXPORTED_QDICTLIBRARY_TEMPLATES)
#define Q_EXPORTED_QDICTLIBRARY_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QDict<QLibrary>; // for Qtopia
#endif

#if defined(Q_DEFINED_QGUARDEDPTR) && defined(Q_DEFINED_QOBJECT) && !defined(Q_EXPORTED_QGUARDEDPTROBJECT_TEMPLATES)
#define Q_EXPORTED_QGUARDEDPTROBJECT_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QGuardedPtr<QObject>;
#endif

// needed for Qtopia
#if defined(Q_DEFINED_QGUARDEDPTR) && defined(Q_DEFINED_QWIDGET) && !defined(Q_EXPORTED_QGUARDEDPTRQWIDGET_TEMPLATES)
#define Q_EXPORTED_QGUARDEDPTRQWIDGET_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QGuardedPtr<QWidget>;
#endif

#if defined(Q_DEFINED_QGUARDEDPTR) && defined(Q_DEFINED_QACCESSIBLE_OBJECT) && !defined(Q_EXPORTED_QACCESSIBLEOBJECT_TEMPLATES)
#define Q_EXPORTED_QACCESSIBLEOBJECT_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QGuardedPtr<QAccessibleObject>;
#endif

#if defined(Q_DEFINED_QINTDICT) && !defined(Q_EXPORTED_QINTDICT_TEMPLATES)
#define Q_EXPORTED_QINTDICT_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDict<int>;
#endif

#if defined(Q_DEFINED_QINTDICT) && defined(Q_DEFINED_QWIDGET) && !defined(Q_EXPORTED__TEMPLATES)
#define Q_EXPORTED__TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDictIterator<QWidget>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDict<QWidget>;
#endif

#if defined(Q_DEFINED_QMAP) && !defined(Q_EXPORTED_QMAPBASIC_TEMPLATES)
#define Q_EXPORTED_QMAPBASIC_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<int, int>; // for Qtopia
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<int, bool>; // for Qtopia
#endif

#if defined(Q_DEFINED_QMAP) && defined(Q_DEFINED_QSTRING) && !defined(Q_EXPORTED_QMAPQSTRING_TEMPLATES)
#define Q_EXPORTED_QMAPQSTRING_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<QString, QString>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<QString, int>; // for Qtopia
Q_TEMPLATE_EXTERN template class Q_EXPORT QMap<int, QString>; // for Qtopia
#endif

#if defined(Q_DEFINED_QMEMARRAY)  && !defined(Q_EXPORTED_QMEMARRAY_BASIC_TEMPLATES)   
#define Q_EXPORTED_QMEMARRAY_BASIC_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<int>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<bool>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<char>;
#endif

#if defined(Q_DEFINED_QMEMARRAY) && defined(Q_DEFINED_QPOINT)  && !defined(Q_EXPORTED_QMEMARAYPOINT_TEMPLATES)
#define Q_EXPORTED_QMEMARAYPOINT_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<QPoint>;
#endif

#if defined(Q_DEFINED_QPTRLIST)  && !defined(Q_EXPORTED_QPTRLIST_BASIC_TEMPLATES)
#define Q_EXPORTED_QPTRLIST_BASIC_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<char>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<char>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QWIDGET)  && !defined(Q_EXPORTED_QPTRLISTWIDGET_TEMPLATES)
#define Q_EXPORTED_QPTRLISTWIDGET_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QWidget>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QWidget>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QCONNECTION)  && !defined(Q_EXPORTED_QPTRLISTCONNECTION_TEMPLATES)
#define Q_EXPORTED_QPTRLISTCONNECTION_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QConnection>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QConnection>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QOBJECT)  && !defined(Q_EXPORTED_QPTRLISTOBJECT_TEMPLATES)
#define Q_EXPORTED_QPTRLISTOBJECT_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QObject>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QObject>;
#endif

#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QDOCKWINDOW)  && !defined(Q_EXPORTED_QPTRLISTDOCWINDOW_TEMPLATES)
#define Q_EXPORTED_QPTRLISTDOCWINDOW_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QDockWindow>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QDockWindow>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR)  && !defined(Q_EXPORTED_QPTRVECTOR_BASIC_TEMPLATES)
#define Q_EXPORTED_QPTRVECTOR_BASIC_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<int>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QSTYLESHEET)  && !defined(Q_EXPORTED_QPTRVECTORSTYLESHEETITEM_TEMPLATES)
#define Q_EXPORTED_QPTRVECTORSTYLESHEETITEM_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QStyleSheetItem>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QWIDGET)  && !defined(Q_EXPORTED_QPTRVECTORWIDGET_TEMPLATES)
#define Q_EXPORTED_QPTRVECTORWIDGET_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QWidget>;
#endif

#if defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QCONNECTION_LIST)  && !defined(Q_EXPORTED_QPTRVECTORCONNECTTIONLIST_TEMPLATES)
#define Q_EXPORTED_QPTRVECTORCONNECTTIONLIST_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QConnectionList>;
#endif

#if defined(Q_DEFINED_QVALUELIST)  && !defined(Q_EXPORTED_QVALUELIST_BASIC_TEMPLATES) 
#define Q_EXPORTED_QVALUELIST_BASIC_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueListIterator<bool>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<bool>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueListIterator<int>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<int>;
#endif

#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QRECT)  && !defined(Q_EXPORTED_QVALUELISTRECT_TEMPLATES)
#define Q_EXPORTED_QVALUELISTRECT_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueListIterator<QRect>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<QRect>;
#endif

#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QSTRING)  && !defined(Q_EXPORTED_QVALUELISTSTRING_TEMPLATES)
#define Q_EXPORTED_QVALUELISTSTRING_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueListIterator<QString>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<QString>;
#endif

// QStylesheet template exports
#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QPTRVECTOR) && defined(Q_DEFINED_QSTYLESHEET)  && !defined(Q_EXPORTED_QSTYLESHEETITEM1_TEMPLATES)
#define Q_EXPORTED_QSTYLESHEETITEM1_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList< QPtrVector< QStyleSheetItem> >;
#endif

#if defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QSTYLESHEET)  && !defined(Q_EXPORTED_QSTYLESHEETITEM2_TEMPLATES)
#define Q_EXPORTED_QSTYLESHEETITEM2_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QValueList<QStyleSheetItem::ListStyle>;
#endif

// qcanvas template exports
#if defined(Q_DEFINED_QPTRLIST)  && defined(Q_DEFINED_QCANVAS) && !defined(Q_EXPORTED_QCANVAS1_TEMPLATES)
#define Q_EXPORTED_QCANVAS1_TEMPLATES
QM_TEMPLATE_EXTERN_CANVAS template class QM_EXPORT_CANVAS QPtrListIterator< QCanvasItem >;
QM_TEMPLATE_EXTERN_CANVAS template class QM_EXPORT_CANVAS QPtrList< QCanvasItem >;
QM_TEMPLATE_EXTERN_CANVAS template class QM_EXPORT_CANVAS QPtrListIterator< QCanvasView >;
QM_TEMPLATE_EXTERN_CANVAS template class QM_EXPORT_CANVAS QPtrList< QCanvasView >;
#endif

#if defined(Q_DEFINED_QVALUELIST)  && defined(Q_DEFINED_QCANVAS) && !defined(Q_EXPORTED_QCANVAS2_TEMPLATES)
#define Q_EXPORTED_QCANVAS2_TEMPLATES
QM_TEMPLATE_EXTERN_CANVAS template class QM_EXPORT_CANVAS QValueListIterator< QCanvasItem* >;
QM_TEMPLATE_EXTERN_CANVAS template class QM_EXPORT_CANVAS QValueList< QCanvasItem* >;
#endif

// qtable template exports
#if defined(Q_DEFINED_QPTRLIST) && defined(Q_DEFINED_QTABLE_SELECTION) && !defined(Q_EXPORTED_QTABLESELECTION_TEMPLATES)
#define Q_EXPORTED_QTABLESELECTION_TEMPLATES
QM_TEMPLATE_EXTERN_TABLE template class QM_EXPORT_TABLE QPtrList<QTableSelection>;
#endif

#if defined(Q_DEFINED_QTABLE_ITEM) && defined(Q_DEFINED_QPTRVECTOR) && !defined(Q_EXPORTED_QTABLEITEM_TEMPLATES)
#define Q_EXPORTED_QTABLEITEM_TEMPLATES
QM_TEMPLATE_EXTERN_TABLE template class QM_EXPORT_TABLE QPtrVector<QTableItem>;
#endif

#if defined(Q_DEFINED_QTABLE) && defined(Q_DEFINED_QPTRVECTOR)
//Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrVector<QTable>;
#endif

// qsqlextension template exports
#if defined(Q_DEFINED_QSQLEXTENSION) && !defined(Q_EXPORTED_QSQLEXTENSION_TEMPLATES)
#define Q_EXPORTED_QSQLEXTENSION_TEMPLATES
QM_TEMPLATE_EXTERN_SQL template class QM_EXPORT_SQL QMap<int,QString>;
QM_TEMPLATE_EXTERN_SQL template class QM_EXPORT_SQL QMap<QString,Param>;
QM_TEMPLATE_EXTERN_SQL template class QM_EXPORT_SQL QVector<Holder>;
#endif


// ### Begin attempt to make 4.0 compile on MSVC 6.0
#if defined(Q_DEFINED_QSQLINDEX) && !defined(Q_EXPORTED_QSQLINDEX_TEMPLATES)
#define Q_EXPORTED_QSQLINDEX_TEMPLATES
QM_TEMPLATE_EXTERN_SQL template class QM_EXPORT_SQL QList<bool>;
#endif

#if defined(Q_DEFINED_QOBJECTLIST) && !defined(Q_EXPORTED_QOBJECTLISTCLEANUPHANDLER_TEMPLATES)
#define Q_EXPORTED_QOBJECTLISTCLEANUPHANDLER_TEMPLATES
Q_TEMPLATE_EXTERN template class Q_EXPORT QList<QObject*>;
#endif

// MOC_SKIP_END
#endif // template defined
