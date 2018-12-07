#ifndef WRAPPER_API_H
#define WRAPPER_API_H

#include "../include/v8.h"
#include <nan.h>

namespace NodeWrapper
{
	NAN_METHOD(Start);
	NAN_METHOD(OnProcessAttach);
	NAN_METHOD(OnProcessDetach);
	NAN_METHOD(OnThreadCreate);
	NAN_METHOD(OnThreadRemove);
	NAN_METHOD(InjectProcess);
	NAN_METHOD(DetachProcess);
	NAN_METHOD(InsertHook);
}

#endif