// lib.h

#pragma once

#using <mscorlib.dll>
using namespace System;

class Worker;

// .NET class
namespace worker
{
    public __gc class netWorker
    {
    public:
	netWorker();
	~netWorker();

	__property void set_StatusString(String *string);
	__property String *get_StatusString();

    private:
	Worker *workerObject;
    };
}
