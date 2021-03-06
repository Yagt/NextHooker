#include "wrapper_api.h"
#include "misc.h"
#include "../vnrhook/host/host.h"
#include "../vnrhook/host/textthread.h"

#include <iostream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <queue>
#include <windows.h>

//////////////////////////////////////////////////////////////////////////////////////////////

Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onProcessAttachFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onProcessDetachFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onThreadCreateFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onThreadRemoveFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onThreadOutputFunction;


std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> _converter;

//////////////////////////////////////////////////////////////////////////////////////////////

v8::Local<v8::Object> TextThreadToV8Object(TextThread *thread)
{
	ThreadParam tp = thread->tp;

	v8::Local<v8::Object> obj = Nan::New<v8::Object>();

	obj->Set(Nan::New("num").ToLocalChecked(), Nan::New(static_cast<double>(thread->handle)));
	obj->Set(Nan::New("pid").ToLocalChecked(), Nan::New(static_cast<uint32_t>(tp.processId)));
	obj->Set(Nan::New("hook").ToLocalChecked(), Nan::New(static_cast<double>(tp.addr)));
	obj->Set(Nan::New("retn").ToLocalChecked(), Nan::New(static_cast<double>(tp.ctx)));
	obj->Set(Nan::New("spl").ToLocalChecked(), Nan::New(static_cast<double>(tp.ctx2)));
	obj->Set(Nan::New("name").ToLocalChecked(), Nan::New(_converter.to_bytes(
		thread->name))
		.ToLocalChecked());
	obj->Set(Nan::New("hcode").ToLocalChecked(), Nan::New(_converter.to_bytes(
		GenerateHCodeWstring(Host::GetHookParam(tp), tp.processId)))
		.ToLocalChecked());

	return obj;
}

v8::Local<v8::Object> RemovedTextThreadToV8Object(TextThread *thread)
{
	ThreadParam tp = thread->tp;

	v8::Local<v8::Object> obj = Nan::New<v8::Object>();

	obj->Set(Nan::New("num").ToLocalChecked(), Nan::New(static_cast<double>(thread->handle)));
	obj->Set(Nan::New("pid").ToLocalChecked(), Nan::New(static_cast<uint32_t>(tp.processId)));
	obj->Set(Nan::New("hook").ToLocalChecked(), Nan::New(static_cast<double>(tp.addr)));
	obj->Set(Nan::New("retn").ToLocalChecked(), Nan::New(static_cast<double>(tp.ctx)));
	obj->Set(Nan::New("spl").ToLocalChecked(), Nan::New(static_cast<double>(tp.ctx2)));

	return obj;
}

//////////////////////////////////////////////////////////////////////////////////////////////

enum CallbackType
{
	PROCESS_ATTACH,
	PROCESS_DETACH,
	THREAD_CREATE,
	THREAD_REMOVE,
	THREAD_OUTPUT
};

struct CallbackInfo
{
	CallbackType type;
	DWORD pid;
	TextThread *tt;
	std::wstring output;
};

void _notifyCallback(CallbackInfo *info);

uv_async_t _async;

std::mutex _queueMutex;
std::queue<CallbackInfo *> _callbackInfos;

void _callbackHandler(uv_async_t *handle)
{
	Nan::HandleScope scope;
	CallbackInfo *info;

	_queueMutex.lock();
	while (!_callbackInfos.empty()) {
		info = _callbackInfos.front();
		_notifyCallback(info);
		delete info;
		info = nullptr;
		_callbackInfos.pop();
	}
	_queueMutex.unlock();
}

void _notifyCallback(CallbackInfo *info)
{
	switch (info->type) {
	case CallbackType::PROCESS_ATTACH: {
		v8::Local<v8::Value> argv[] = { Nan::New((uint32_t)info->pid) };
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onProcessAttachFunction), 1, argv);
	}
		break;
	case CallbackType::PROCESS_DETACH: {
		v8::Local<v8::Value> argv[] = { Nan::New((uint32_t)info->pid) };
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onProcessDetachFunction), 1, argv);
	}
		break;
	case CallbackType::THREAD_CREATE: {
		v8::Local<v8::Value> argv[] = { TextThreadToV8Object(info->tt) };
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onThreadCreateFunction), 1, argv);
	}
		break;
	case CallbackType::THREAD_REMOVE: {
		v8::Local<v8::Value> argv[] = { RemovedTextThreadToV8Object(info->tt) };
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onThreadRemoveFunction), 1, argv);
	}
		break;
	case CallbackType::THREAD_OUTPUT: {
		std::string sOutput = _converter.to_bytes(info->output);

		v8::Local<v8::Value> argv[] = {
			TextThreadToV8Object(info->tt),
			Nan::New(sOutput).ToLocalChecked()
		};
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onThreadOutputFunction), 2, argv);
	}
		break;
	default: {
	}
		break;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////

NAN_METHOD(NodeWrapper::OnProcessAttach)
{
	Nan::HandleScope scope;

	if (!info[0]->IsFunction()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}

	_onProcessAttachFunction = Nan::Persistent<v8::Function>(info[0].As<v8::Function>());
}

NAN_METHOD(NodeWrapper::OnProcessDetach)
{
	Nan::HandleScope scope;

	if (!info[0]->IsFunction()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}

	_onProcessDetachFunction = Nan::Persistent<v8::Function>(info[0].As<v8::Function>());
}

NAN_METHOD(NodeWrapper::OnThreadCreate)
{
	Nan::HandleScope scope;

	if (!info[0]->IsFunction() || !info[1]->IsFunction()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}

	_onThreadCreateFunction = Nan::Persistent<v8::Function>(info[0].As<v8::Function>());
	_onThreadOutputFunction = Nan::Persistent<v8::Function>(info[1].As<v8::Function>());
}

NAN_METHOD(NodeWrapper::OnThreadRemove)
{
	Nan::HandleScope scope;

	if (!info[0]->IsFunction()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}

	_onThreadRemoveFunction = Nan::Persistent<v8::Function>(info[0].As<v8::Function>());
}

////////////////////////////////////////////////////////////////////////////////////////////////

std::mutex _callbackMutex;

NAN_METHOD(NodeWrapper::Start)
{
	Nan::HandleScope scope;
	Host::Start(
	[&](DWORD pid) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::PROCESS_ATTACH;
		newInfo->pid = pid;
		_callbackMutex.lock();
		_callbackInfos.push(newInfo);
		_callbackMutex.unlock();
		uv_async_send(&_async);
	}, 
	[&](DWORD pid) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::PROCESS_DETACH;
		newInfo->pid = pid;
		_callbackMutex.lock();
		_callbackInfos.push(newInfo);
		_callbackMutex.unlock();
		uv_async_send(&_async);
	}, 
	[&](TextThread* thread) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::THREAD_CREATE;
		newInfo->tt = thread;
		_callbackMutex.lock();
		_callbackInfos.push(newInfo);
		_callbackMutex.unlock();
		uv_async_send(&_async);
	}, 
	[&](TextThread* thread) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::THREAD_REMOVE;
		newInfo->tt = thread;
		_callbackMutex.lock();
		_callbackInfos.push(newInfo);
		_callbackMutex.unlock();
		uv_async_send(&_async);
	},
	[&](TextThread* threadOut, std::wstring &output) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::THREAD_OUTPUT;
		newInfo->tt = threadOut;
		newInfo->output = output;
		_callbackMutex.lock();
		_callbackInfos.push(newInfo);
		_callbackMutex.unlock();
		uv_async_send(&_async);
		return true;
	});
}

////////////////////////////////////////////////////////////////////////////////////////////////

NAN_METHOD(NodeWrapper::InjectProcess)
{
	Nan::HandleScope scope;

	uv_async_init(uv_default_loop(), &_async, _callbackHandler);

	int pid = info[0]->IntegerValue();
	Host::InjectProcess(pid);
}

NAN_METHOD(NodeWrapper::DetachProcess)
{
	Nan::HandleScope scope;

	int pid = info[0]->IntegerValue();
	Host::DetachProcess(pid);
}

NAN_METHOD(NodeWrapper::InsertHook)
{
	Nan::HandleScope scope;
	if (!info[0]->IsUint32() || !info[1]->IsString()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}
	int pid = info[0]->IntegerValue();
	v8::String::Utf8Value v8s(info[1]);
	std::string sHookCode(*v8s);
	std::wstring wsHookCode(_converter.from_bytes(sHookCode));
	HookParam toInsert = ParseHCodeWstring(wsHookCode);
	if (toInsert.type == 0 && toInsert.length_offset == 0)
	{
		info.GetIsolate()->ThrowException(v8::Exception::Error(Nan::New("invalid /H code").ToLocalChecked()));
	}
	Host::InsertHook(pid, toInsert);
}
