SAMPLE: DRGDRPT: OLE Drag-Drop Target

OLE2 provides drag-drop functionality that allows data to be dragged 
between applications and within applications.

DRGDRPT is an OLE drag-drop target that allows data of format CF_TEXT 
to be dropped on it. DRGDRPT implements the IDropTarget interface. It 
registers its main window as a drop target using RegisterDragDrop.

See the DRGDRPS sample in Software/Data Library for an example 
of an OLE drag-drop source.

See the SIMPDND sample in the OLE2 SDK for a more complex 
drag-drop example.
