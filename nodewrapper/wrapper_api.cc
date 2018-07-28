#include "wrapper_api.h"
#include "misc.h"
#include "../texthook/host.h"
#include "../texthook/textthread.h"

#include <iostream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <queue>

class mutex
{
private:
	CRITICAL_SECTION _criticalSection;
public:
	mutex() { InitializeCriticalSection(&_criticalSection); }
	~mutex() { DeleteCriticalSection(&_criticalSection); }
	inline void lock() { EnterCriticalSection(&_criticalSection); }
	inline void unlock() { LeaveCriticalSection(&_criticalSection); }

	class scoped_lock
	{
	public:
		inline explicit scoped_lock(mutex & sp) : _sl(sp) { _sl.lock(); }
		inline ~scoped_lock() { _sl.unlock(); }
	private:
		scoped_lock(scoped_lock const &);
		scoped_lock & operator=(scoped_lock const &);
		mutex& _sl;
	};
};

std::wstring TextThreadToString(TextThread *thread)
{
	ThreadParameter tp = thread->GetThreadParameter();

	std::wstringstream wss;
	wss << thread->Number() << L":"
		<< tp.pid << L":"
		<< std::hex << std::uppercase << tp.hook << L":"
		<< std::hex << std::uppercase << tp.retn << L":" 
		<< std::hex << std::uppercase << tp.spl << L":"
		<< Host::GetHookName(tp.pid, tp.hook) << L":"
		<< GenerateHCodeWstring(Host::GetHookParam(tp.pid, tp.hook), tp.pid);

	return wss.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////

NAN_METHOD(NodeWrapper::Start)
{
	Nan::HandleScope scope;
	Host::Start();
}

NAN_METHOD(NodeWrapper::Open)
{
	Nan::HandleScope scope;
	Host::Open();
}

//////////////////////////////////////////////////////////////////////////////////////////////

Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onProcessAttachFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onProcessDetachFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onThreadCreateFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onThreadRemoveFunction;
Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent _onThreadOutputFunction;


std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> _converter;

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

mutex _queueMutex;
std::queue<CallbackInfo *> _callbackInfos;

void _callbackHandler(uv_async_t *handle)
{
	Nan::HandleScope scope;
	CallbackInfo *info;

	mutex::scoped_lock sl(_queueMutex);
	while (!_callbackInfos.empty()) {
		info = _callbackInfos.front();
		_notifyCallback(info);
		delete info;
		_callbackInfos.pop();
	}
}

void _notifyCallback(CallbackInfo *info)
{
	switch (info->type)
	{
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
		//std::cout << "in c++: thread created..." << std::endl;

		std::wstring wsTs = TextThreadToString(info->tt);
		std::string sTs = _converter.to_bytes(wsTs);
		v8::Local<v8::Value> argv[] = { Nan::New(sTs).ToLocalChecked() };
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onThreadCreateFunction), 1, argv);
	}
		break;
	case CallbackType::THREAD_REMOVE: {
		std::wstring wsTs = TextThreadToString(info->tt);
		std::string sTs = _converter.to_bytes(wsTs);
		v8::Local<v8::Value> argv[] = { Nan::New(sTs).ToLocalChecked() };
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onThreadRemoveFunction), 1, argv);
	}
		break;
	case CallbackType::THREAD_OUTPUT: {
		std::wstring wsOut = TextThreadToString(info->tt);
		std::string sTsOut = _converter.to_bytes(wsOut);
		std::string sOutput = _converter.to_bytes(info->output);

		v8::Local<v8::Value> argv[] = {
			Nan::New(sTsOut).ToLocalChecked(),
			Nan::New(sOutput).ToLocalChecked()
		};
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_onThreadOutputFunction), 2, argv);
	}
		break;
	default: {
		//std::cout << "c++: error callback not handled" << std::endl;
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

	Host::RegisterProcessAttachCallback([&](DWORD pid) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::PROCESS_ATTACH;
		newInfo->pid = pid;
		_callbackInfos.push(newInfo);
		uv_async_send(&_async);
	});
}

NAN_METHOD(NodeWrapper::OnProcessDetach)
{
	Nan::HandleScope scope;

	if (!info[0]->IsFunction()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}

	_onProcessDetachFunction = Nan::Persistent<v8::Function>(info[0].As<v8::Function>());

	Host::RegisterProcessDetachCallback([&](DWORD pid) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::PROCESS_DETACH;
		newInfo->pid = pid;
		_callbackInfos.push(newInfo);
		uv_async_send(&_async);
	});
}

NAN_METHOD(NodeWrapper::OnThreadCreate)
{
	Nan::HandleScope scope;

	if (!info[0]->IsFunction() || !info[1]->IsFunction()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}

	_onThreadCreateFunction = Nan::Persistent<v8::Function>(info[0].As<v8::Function>());
	_onThreadOutputFunction = Nan::Persistent<v8::Function>(info[1].As<v8::Function>());

	Host::RegisterThreadCreateCallback([&](TextThread* thread) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::THREAD_CREATE;
		newInfo->tt = thread;
		_callbackInfos.push(newInfo);

		thread->RegisterOutputCallBack([&](TextThread* threadOut, std::wstring output) {
			//std::cout << "in c++: thread output..." << std::endl;
			CallbackInfo *newInfo = new CallbackInfo;
			newInfo->type = CallbackType::THREAD_OUTPUT;
			newInfo->tt = threadOut;
			newInfo->output = output;
			_callbackInfos.push(newInfo);
			uv_async_send(&_async);
			return output;
		});

		uv_async_send(&_async);
	});
}

NAN_METHOD(NodeWrapper::OnThreadRemove)
{
	Nan::HandleScope scope;

	if (!info[0]->IsFunction()) {
		info.GetIsolate()->ThrowException(v8::Exception::TypeError(Nan::New("Wrong arguments").ToLocalChecked()));
	}

	_onThreadRemoveFunction = Nan::Persistent<v8::Function>(info[0].As<v8::Function>());

	Host::RegisterThreadRemoveCallback([&](TextThread* thread) {
		CallbackInfo *newInfo = new CallbackInfo;
		newInfo->type = CallbackType::THREAD_REMOVE;
		newInfo->tt = thread;
		_callbackInfos.push(newInfo);
		uv_async_send(&_async);
	});
}

////////////////////////////////////////////////////////////////////////////////////////////////

NAN_METHOD(NodeWrapper::InjectProcess)
{
	Nan::HandleScope scope;

	uv_async_init(uv_default_loop(), &_async, _callbackHandler);
	//uv_async_init(uv_default_loop(), &_async_output, _callbackHandler);

	int pid = info[0]->IntegerValue();
	Host::InjectProcess(pid);
}

NAN_METHOD(NodeWrapper::DetachProcess)
{
	Nan::HandleScope scope;

	int pid = info[0]->IntegerValue();
	Host::DetachProcess(pid);
}