Instructions
============

To build the package you need to have Visual C++ with the ATL installed.
A Qt 3.0.2 (or higher) DLL should be somewhere in the path (the plugin 
for the Qt Designer integration requires Qt to be a shared shared library).

Unzip the archive and run the "build.bat" batch file. This will simply run

qmake
nmake
regcontrols

regcontrols just registers the example ActiveX servers.


To remove the installation and unregister the servers run

build clean


Documentation, including a class API reference and example documentation
is present in doc/html.


Container
=========

The container module of the framework enables Qt applications to host
any COM object and wraps a QMetaObject around OLE automation. An
abstract base class implements this, and is pulled into a QObject and a
QWidget class via multiple inheritance:

QComBase    - implements meta object generation and provides a common API
              for initializing and using of the COM object
QComObject  - a QObject around a COM object ( : public QObject, public QComBase )
QAxWidget   - a QWidget around an ActiveX ( : public QWidget, public QComBase )

Instantiating any of those object wrappers with a suitable COM object
(any COM object that implements IDispatch is suitable) creates a meta
object on-the-fly, with the consequence that

- all COM properties become available as Qt properties
- all COM methods/property-setters become available as Qt slots
- all COM events/property-notifications become available as Qt signals

This includes data type conversion, e.g. QString -> OLESTR and such.
Some datatypes are not supported, but the framework gives access to the
COM object interfaces so that developers can work with the object via
COM as they would always do.

The container framework comes as a DLL that can be linked against, but
it also implements the plugin interfaces supported by Qt Designer. Both
the examples "testcon" and "webbrowser" have been created in the
Designer using that plugin.


To test the container part of the framework, run examples/testcon and
examples/webbrowser. The webbrowser example utilizes the Microsoft Web
Browser Control (the core component of Internet explorer) for a www
browser. The implementation of functionality not available via simple
signal slot connections is in the mainwindow.ui.h file.
The "testcon" example is basically the "ActiveX Control Test Container"
that ships with Visual Studio written with Qt, and utilizing the
QActiveX widget. You can instantiate any of the ActiveX'es registered on
your system in an MDI environment and call slots, change properties and
monitor events. If an ActiveX control doesn't work with the "testcon"
example, try if it works in the Microsoft test container before writing a
bug report ;-)



Control
=======

The control module is a static library that implements WinMain, thus
replacing qtmain.lib when the project is supposed to become an ActiveX 
server. All ActiveX servers written with Qt run out-of-process to avoid 
QApplication conflicts.

The library implements OLE automation based on the QMetaObject and
supports multiple ActiveX'es in a single server executable. As the type
information for OLE has to be provided statically as a type library, an
additional precompiler dubbed "idc" parses the QObject class declaration
and generates IDL source code that is turned into a type library by the
Microsoft IDL compiler.

Implementing an ActiveX with Qt turns

- all Qt properties into COM properties
- all Qt slots into COM methods
- all Qt signals into COM events


The control side of the framework comes with four examples which can be
run in the Microsoft test container, but of course as well in the Qt
testcon example. All controls registered start with a "Q", so you can
easily jump to them in the ActiveX listbox.
The examples "simple", "tetrix", "multiple" and "wrapper" demonstrate
different ways about how to write ActiveX'es with Qt. The documentation
includes HTML pages that demonstrate the ActiveX controls.
