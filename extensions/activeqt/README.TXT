Instructions
============

To build the package you need to have Visual C++ with the ATL installed,
and a Qt 3.0.4 (or higher) installation in QTDIR.

Unzip the archive and run the "build.bat" batch file. To remove the 
installation and unregister the servers run "build clean".

Documentation, including a class API reference and example documentation,
starts at doc/html/index.html.


Container
=========

The container module of the framework enables Qt applications to host
any COM object and wraps a QMetaObject around OLE automation. An
abstract base class implements this, and is pulled into a QObject and a
QWidget class via multiple inheritance:

QAxBase     - implements meta object generation and provides a common API
              for initializing and using of the COM object
QAxObject   - a QObject around a COM object ( : public QObject, public QAxBase )
QAxWidget   - a QWidget around an ActiveX ( : public QWidget, public QAxBase )

Instantiating any of those object wrappers with a suitable COM object
(any COM object that implements IDispatch is suitable) creates a meta
object on-the-fly, with the consequence that

- all COM properties become available as Qt properties
- all COM methods/property-setters become available as Qt slots
- all COM events/property-notifications become available as Qt signals

This includes data type conversion, e.g. QString -> OLESTR.
Some datatypes are not supported, but the framework gives access to the
COM object interfaces so that developers can work with the object via
COM as they would always do.

The container framework is a shared or static library (depending on the 
configuration of Qt); the shared version also implements the widget plugin 
interface supported by Qt Designer.
Both the examples "testcon" and "webbrowser" have been created in the
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
bug report.



Server
=======

The server module is a static library that implements WinMain, thus
replacing qtmain.lib when the project is supposed to become an ActiveX 
server. ActiveX servers written with Qt can be in process or out of
process, and provide any number of ActiveX controls.

The library implements OLE automation based on the QMetaObject and
supports multiple ActiveX'es in a single server executable. As the type
information for OLE has to be provided statically as a type library, the
build system is extended to generate and link this type library into the
executable. As the Win32 API required for that step is not supported on
Windows 95/98/ME it is not possible to build ActiveX servers with Qt on
those Windows versions. The resulting binary however will work on
all Windows versions.

Implementing an ActiveX control with Qt turns

- all Qt properties into COM properties
- all Qt slots into COM methods
- all Qt signals into COM events


The control side of the framework comes with five examples which can be
run in the Microsoft test container, but of course as well in the Qt
testcon example.
The examples "simple", "tetrix", "multiple" and "wrapper" and "opengl" 
demonstrate different ways of writing ActiveX'es with Qt. The 
documentation includes HTML pages that demonstrate the ActiveX controls.
