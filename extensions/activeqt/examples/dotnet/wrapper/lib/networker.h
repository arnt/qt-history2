// lib.h

#pragma once

#using <mscorlib.dll>
using namespace System;

class Worker;
class Dispatcher;

// .NET class
public __gc class netWorker
{
public:
    netWorker();
    ~netWorker();

    __property String *get_StatusString();
    __property void set_StatusString(String *string);

    __event void statusStringChanged(String *args);

private:
    Worker *workerObject;
};
