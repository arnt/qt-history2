SAMPLE: DRGDRPS: OLE Drag-Drop Source

SUMMARY
=======

DRGDRPS is an OLE drag-drop Source that allows data of format CF_TEXT to be 
dragged from it. 

MORE INFORMATION
================

OLE2 provides drag-drop functionality that allows data to be dragged 
between applications and within applications.

DRGDRPS implements IDataObject, IDropSource and IEnumFormatEtc interfaces. 
It calls DoDragDrop when the user clicks on its main window to initiate 
a drag. It allows copying of data by dragging to a drag-drop target.

See the DRGDRPT sample in Software/Data Library for an example of an 
OLE drag-drop target. 

See the SIMPDND sample in the OLE2 SDK for a more complex drag-drop 
example.

