ActiveQtEXE
------------


This is a prototype of an ActiveX framework for generating
Qt-based ActiveX controls.
It is implemented as an out of process COM server containing
a QApplication object that is common for all the instances
of the ActiveX object.
This makes it possible for one host to contain several Qt
ActiveX widgets without getting into conflicts over the
QApplication object.

It is also possible to implement several different ActiveX
controls in the same server, and not all of them need to
be Qt-based.

The project includes a framework to generate new Qt ActiveX
controls, and focus has been on keeping the process as
simple as possible.
Just let the ActiveX control class inherit from your Qt widget,
and you're up and running.
If you are inheriting QActiveXBase (contained in the project) in
your Qt widget, there is nothing else to do, if not, you will
need to implement a couple of support functions yourself.

Have fun with it, and feel free to comment on it, or modify the
code any way you'd like.

Keep in mind that this is a prototype, and there will be work
done to get it better.

---
Erik H. Bakke
