#include "nodewrapper/wrapper_api.h"

using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::String;
using Nan::GetFunction;
using Nan::New;
using Nan::Set;

NAN_MODULE_INIT(InitAll) {
	Set(target, New<String>("start").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::Start)).ToLocalChecked());
	Set(target, New<String>("onProcessAttach").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::OnProcessAttach)).ToLocalChecked());
	Set(target, New<String>("onProcessDetach").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::OnProcessDetach)).ToLocalChecked());
	Set(target, New<String>("onThreadCreate").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::OnThreadCreate)).ToLocalChecked());
	Set(target, New<String>("onThreadRemove").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::OnThreadRemove)).ToLocalChecked());
	Set(target, New<String>("injectProcess").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::InjectProcess)).ToLocalChecked());
	Set(target, New<String>("detachProcess").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::DetachProcess)).ToLocalChecked());
	Set(target, New<String>("insertHook").ToLocalChecked(),
		GetFunction(New<FunctionTemplate>(NodeWrapper::InsertHook)).ToLocalChecked());
}

NODE_MODULE(addon, InitAll)
