#include <uv.h>
#include <nan.h>
#include <v8.h>
#include "CallbackProcessor.h"

CallbackProcessor* processor = NULL;

CallbackProcessor::CallbackProcessor()
{
	if (processor == NULL) {
		processorHandle = new uv_async_t;
		processorHandle->data = (void*)this;
		uv_async_init(uv_default_loop(), processorHandle, processCallbacks);
	}
}

CallbackProcessor::~CallbackProcessor()
{
	uv_close((uv_handle_t*)processorHandle, NULL);
	delete processorHandle;
}

void CallbackProcessor::SendAnswerData(AnswerData*data)
{
	if (processor == NULL)
		processor = new CallbackProcessor();
	processor->addCallback(data);
}


void CallbackProcessor::addCallback(AnswerData* callback) {
	std::lock_guard<std::mutex> lock(callbackQueueMtx);
	callbacks.push(callback);
	uv_async_send(processorHandle);
}

unsigned int CallbackProcessor::callbackCount() {
	std::lock_guard<std::mutex> lock(callbackQueueMtx);
	return callbacks.size();
}

AnswerData* CallbackProcessor::nextCallback() {
	std::lock_guard<std::mutex> lock(callbackQueueMtx);
	AnswerData* ret = callbacks.front();
	callbacks.pop();
	return ret;
}

void processCallbacks(uv_async_t* req) {
	CallbackProcessor* processor = (CallbackProcessor*) req->data;
	while (processor->callbackCount() > 0) {
		AnswerData* ad = processor->nextCallback();
		if (ad->cb != NULL) {
			try {
				Nan::HandleScope scope;
				v8::Local<v8::Value> args[] = { Nan::Null(), Nan::Null() };
				if (ad->err != NULL) {
					args[0] = Nan::New(ad->err->textVar).ToLocalChecked();
				}

				if (ad->values && ad->values->size() > 0) {
					args[1] = AnswerValue::ToJSValuesArray(ad->values);
				}
				Nan::AsyncResource resource("WinCCOANodeJS:CallbackProcessor");
				ad->cb->Call(2, args, &resource);
				if (ad->resetCallback)
					ad->cb->Reset();
			}
			catch (std::exception& err) {
				//ErrHdl::error(ErrClass::PRIO_WARNING,      // It is a severe error
				//	ErrClass::ERR_CONTROL,
				//	ErrClass::UNEXPECTEDSTATE,  // fits all
				//	"ManagerManager",              // our file name
				//	"processCallbacks",                      // our function name
				//	err.what());
			}
			catch (...) {
				//ErrHdl::error(ErrClass::PRIO_WARNING,      // It is a severe error
				//	ErrClass::ERR_CONTROL,
				//	ErrClass::UNEXPECTEDSTATE,  // fits all
				//	"ManagerManager",              // our file name
				//	"processCallbacks",                      // our function name
				//	"Error processing callback");
			}
		}
		//clean up
		if (ad->values != NULL) {
			AnswerValue::DeleteAnswerValueVector(ad->values);
			delete ad->values;
		}
		if (ad->err != NULL)
			delete ad->err;
		ad->cb = NULL;
		delete ad;
	}
}
