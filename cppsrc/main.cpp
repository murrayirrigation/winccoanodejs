/* cppsrc/main.cpp */
#include <nan.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include <list>
#include <mutex>
#include <v8.h>
#include "manager/ManagerManager.hxx"
#include "manager/ManagerResources.hxx"
#include "AnswerCallback.h"
#include "AlertConnectCallback.h"
#include "ConnectAnswerCallback.h"

using namespace Nan;

ManagerManager* manager = NULL;

void ResultCallback(const char *error, int result, Nan::Callback* callback) {
	Nan::HandleScope scope;
	if (!callback || callback->IsEmpty())
		return;
	v8::Local<v8::Value>* args = new v8::Local<v8::Value>[2];
	if (error) {
		args[0] = Nan::New(error).ToLocalChecked();
		args[1] = Nan::Null();
	}
	else {
		args[0] = Nan::Null();
		args[1] = Nan::New(result);
	}
	Nan::AsyncResource resource("WinCCOANodeJS:main");
	callback->Call(2, args, &resource);
	args[0] = Nan::Null();
	args[1] = Nan::Null();
	delete args;
}

NAN_METHOD(connect) {
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

    if (info.Length() < 1) {
        ThrowTypeError("Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString()) {
        ThrowTypeError("Argv is not a string");
        return;
    }

    v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);
    std::string cmdLineArgs = std::string(*v8String);
    std::vector<std::string> split;

    std::istringstream f(cmdLineArgs.c_str());
    std::string s;
    while (std::getline(f, s, ' ')) {
        split.push_back(s);
    }

    int argc = (int)split.size() + 1;
	char** argv = new char* [argc];

	if (info.Length() == 2 && info[1]->IsString()) {
		v8::String::Utf8Value v8NameString(info.GetIsolate(), info[0]);
		std::string nameString = std::string(*v8NameString);
		argv[0] = strdup(nameString.c_str());
	}
	else {
		argv[0] = strdup("node.exe");
	}
    for (int i = 1; i < argc; i++) {
		argv[i] = strdup(split[i - 1].c_str());
    }

	Resources::setExitOnError(false);
    ManagerResources::init(argc, argv);
    manager = new ManagerManager();
    bool connected = manager->connect();
    info.GetReturnValue().Set(connected ? Nan::True() : Nan::False());
	for (int i = 0; i < argc; i++) {
		free(argv[i]);
	}
    delete argv;
}

NAN_METHOD(disconnect) {
    manager->disconnect();
    delete manager;
}

NAN_METHOD(dpGet) {
    if (manager == NULL) {
        ThrowTypeError("Must call connect first");
        return;
    }

    if (info.Length() < 2) {
        ThrowTypeError("Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString()) {
        ThrowTypeError("dp is not a string");
        return;
    }

    if (!info[1]->IsFunction()) {
        ThrowTypeError("cb is not a function");
        return;
    }

	Nan::Callback* cb = new Nan::Callback(info[1].As<v8::Function>());

    v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);

    std::string dp = std::string(*v8String);

	AnswerCallback* aCallback = new AnswerCallback(cb);
	PVSSboolean success = manager->dpGet(dp.c_str(), aCallback);
	if (success == PVSS_FALSE)
		ResultCallback("dpGet command failure", NULL, cb);

	info.GetReturnValue().Set(success == PVSS_TRUE ? Nan::True() : Nan::False());
}

NAN_METHOD(dpSet) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 3) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("dp is not a string");
		return;
	}

	if (!info[2]->IsFunction()) {
		ThrowTypeError("cb is not a function");
		return;
	}

	Nan::Callback* cb = new Nan::Callback(info[2].As<v8::Function>());

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);

	std::string dp = std::string(*v8String);
	AnswerCallback* aCallback = new AnswerCallback(cb);
	PVSSboolean success;
	if (info[1]->IsBoolean()) {
		success = manager->dpSet(dp.c_str(), info[1].As<v8::Boolean>()->Value(), aCallback);
	}
	else if (info[1]->IsString()) {
		v8::String::Utf8Value value(info.GetIsolate(), info[1]);
		std::string dpValue = std::string(*value);
		success = manager->dpSet(dp.c_str(), dpValue.c_str(), aCallback);
	}
	else if (info[1]->IsInt32()) {
		success = manager->dpSet(dp.c_str(), (int)info[1].As<v8::Integer>()->Value(), aCallback);
	}
	else if (info[1]->IsNumber()) {
		success = manager->dpSet(dp.c_str(), info[1].As<v8::Number>()->Value(), aCallback);
	}
	else if (info[1]->IsDate()) {
		success = manager->dpSetDate(dp.c_str(), info[1].As<v8::Date>()->NumberValue(info.GetIsolate()->GetCurrentContext()).ToChecked(), aCallback);
	}
	else if (info[1]->IsArray()) {
		std::vector<std::string> vector;
		v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(info[1]);
		for (unsigned int i = 0; i < array->Length(); i++) {
			auto val = Nan::Get(array, i).ToLocalChecked();
			Nan::Utf8String value(val);
			vector.push_back(std::string(*value));
		}
		success = manager->dpSetStringArray(dp.c_str(), vector, aCallback);
	}
	else {
		ThrowTypeError("Set value is not a valid value for WinCC");
		return;
	}
	if (success == PVSS_FALSE)
		ResultCallback("dpSet command failure", NULL, cb);
	
	info.GetReturnValue().Set(success == PVSS_TRUE ? Nan::True() : Nan::False());
}

struct dpConnectRef {
	std::string dp;
	Nan::Callback* cb;
	ConnectAnswerCallback* aCallback;
};

struct dpConnectList {
	std::list<dpConnectRef*> callbacks;
	std::mutex mtx;
};

std::map<std::string, dpConnectList*> dpConnectCallbacks;
std::mutex dpMapMtx;
NAN_METHOD(dpConnect) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 2) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("dp is not a string");
		return;
	}

	if (!info[1]->IsFunction()) {
		ThrowTypeError("cb is not a function");
		return;
	}

	Nan::Callback* cb = new Nan::Callback(info[1].As<v8::Function>());

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);

	std::string dp = std::string(*v8String);

	ConnectAnswerCallback* aCallback = new ConnectAnswerCallback(cb, dp);
	PVSSboolean success = manager->dpConnect(dp.c_str(), aCallback);
	if (success == PVSS_FALSE)
		ResultCallback("dpConnect command failure", NULL, cb);
	dpConnectRef* dpconnectref = new dpConnectRef;
	dpconnectref->aCallback = aCallback;
	dpconnectref->cb = cb;
	dpconnectref->dp = dp;
	dpMapMtx.lock();
	std::map<std::string, dpConnectList*>::iterator itr = dpConnectCallbacks.find(dp);
	if (itr == dpConnectCallbacks.end())
		dpConnectCallbacks[dp] = new dpConnectList();
	auto dpCl = dpConnectCallbacks[dp];
	dpMapMtx.unlock();
	dpCl->mtx.lock();
	dpCl->callbacks.push_back(dpconnectref);
	dpCl->mtx.unlock();
	info.GetReturnValue().Set(success == PVSS_TRUE ? Nan::True() : Nan::False());
}

NAN_METHOD(dpDisconnect) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 2) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("dp is not a string");
		return;
	}

	if (!info[1]->IsFunction()) {
		ThrowTypeError("cb is not a function");
		return;
	}

	v8::Isolate* isolate = info.GetIsolate();

	v8::Local<v8::Function> cb = info[1].As<v8::Function>();

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);
	std::string dp = std::string(*v8String);
	dpMapMtx.lock();
	std::map<std::string, dpConnectList*>::iterator itr = dpConnectCallbacks.find(dp);
	if (itr == dpConnectCallbacks.end()) {
		info.GetReturnValue().Set(Nan::False());
		dpMapMtx.unlock();
		return;
	}
	dpConnectList* dpCL = itr->second;
	dpMapMtx.unlock();
	dpCL->mtx.lock();
	if (dpCL->callbacks.size() == 1) {
		try {
			//only callback in the list so no need to search
			dpConnectRef* dpcr = dpCL->callbacks.front();
			PVSSboolean success = manager->dpDisconnect(dp.c_str(), dpcr->aCallback);
			if (success == PVSS_FALSE)
				ResultCallback("dpDisconnect command failure", NULL, dpcr->cb);
			info.GetReturnValue().Set(success == PVSS_TRUE ? Nan::True() : Nan::False());
			delete dpcr->aCallback;
			dpcr->aCallback = NULL;
			dpcr->cb->Reset();
			delete dpcr->cb;
			dpCL->callbacks.clear();

			delete dpcr;
		}
		catch (...) {
		}
		dpCL->mtx.unlock();
		return;
	}
	std::list<dpConnectRef*>::iterator dpcrItr = dpCL->callbacks.begin();
	while(dpcrItr != dpCL->callbacks.end()) {
		v8::HandleScope handleScope(isolate);
		dpConnectRef* dpcr = *dpcrItr;
		Nan::Callback* pCB = dpcr->cb;
		v8::Local<v8::Function> referenceCB;
		try {
			referenceCB = pCB->GetFunction();
			if (dpcr->aCallback == NULL) {
				dpcrItr = dpCL->callbacks.erase(dpcrItr);
				continue;
			}
			else if (referenceCB == cb) {
				PVSSboolean success = manager->dpDisconnect(dp.c_str(), dpcr->aCallback);
				if (success == PVSS_FALSE)
					ResultCallback("dpDisconnect command failure", NULL, dpcr->cb);
				info.GetReturnValue().Set(success == PVSS_TRUE ? Nan::True() : Nan::False());
				delete dpcr->aCallback;
				dpcr->aCallback = NULL;
				dpcr->cb->Reset();
				delete dpcr->cb;
				dpcr->cb = NULL;
				dpcrItr = dpCL->callbacks.erase(dpcrItr);

				delete dpcr;
				dpCL->mtx.unlock();
				return;
			}
		}
		catch(...) {
			delete dpcr->aCallback;
			dpcr->aCallback = NULL;
			dpcr->cb->Reset();
			delete dpcr->cb;
			dpcr->cb = NULL;
			dpcrItr = dpCL->callbacks.erase(dpcrItr);

			delete dpcr;
		}
		if(dpCL->callbacks.size() > 0 && dpcrItr != dpCL->callbacks.end())
			++dpcrItr;
	}
	dpCL->mtx.unlock();
	info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(dpQuery) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 2) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("query is not a string");
		return;
	}

	if (!info[1]->IsFunction()) {
		ThrowTypeError("cb is not a function");
		return;
	}

	Nan::Callback* cb = new Nan::Callback(info[1].As<v8::Function>());

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);

	std::string query = std::string(*v8String);
	AnswerCallback* aCallback = new AnswerCallback(cb);
	PVSSboolean success = manager->dpQuery(query.c_str(), aCallback);
	if (success == PVSS_FALSE)
		ResultCallback("dpQuery command failure", NULL, cb);
	info.GetReturnValue().Set(success == PVSS_TRUE ? Nan::True() : Nan::False());
}

NAN_METHOD(dpElementType) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 2) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("DP is not a string");
		return;
	}

	if (!info[1]->IsFunction()) {
		ThrowTypeError("cb is not a function");
		return;
	}

	Nan::Callback* cb = new Nan::Callback(info[1].As<v8::Function>());
	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);

	std::string dp = std::string(*v8String);
	int ret;
	PVSSboolean success = manager->dpElementType(dp.c_str(), &ret);
	if (success)
		ResultCallback(NULL, ret, cb);
	else
		ResultCallback("Error getting dp element type", NULL, cb);
	cb->Reset();
}

NAN_METHOD(dpExists) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 1) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("DP is not a string");
		return;
	}

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);

	std::string dp = std::string(*v8String);
	info.GetReturnValue().Set(manager->dpExists(dp.c_str()) == PVSS_TRUE ? Nan::True() : Nan::False());
}

NAN_METHOD(getTypeName) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 2) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("DP is not a string");
		return;
	}

	if (!info[1]->IsString()) {
		ThrowTypeError("System is not a string");
		return;
	}

	v8::String::Utf8Value dpString(info.GetIsolate(), info[0]);
	std::string dp = std::string(*dpString);
	v8::String::Utf8Value sysString(info.GetIsolate(), info[1]);
	std::string sys = std::string(*sysString);
	CharString err;
	CharString type;

	if (manager->getTypeName(dp.c_str(), sys.c_str(), type, err) == PVSS_TRUE) {
		info.GetReturnValue().Set(Nan::New(type.c_str()).ToLocalChecked());
	}
	else {
		info.GetReturnValue().Set(Nan::Null());
	}
}

NAN_METHOD(getUserId) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 1) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("Username is not a string");
		return;
	}

	v8::String::Utf8Value usernameString(info.GetIsolate(), info[0]);
	std::string username = std::string(*usernameString);
	CharString err;

	uint16_t userid = manager->getUserId(username.c_str());
	info.GetReturnValue().Set(Nan::New(userid));
}

NAN_METHOD(checkPassword) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 2) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber()) {
		ThrowTypeError("UserID is not a number");
		return;
	}
	if (!info[1]->IsString()) {
		ThrowTypeError("password is not a string");
		return;
	}

	uint16_t userId = info[0].As<v8::Number>()->Uint32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
	v8::String::Utf8Value passwordString(info.GetIsolate(), info[1]);
	std::string password = std::string(*passwordString);
	CharString err;

	PVSSboolean ret = manager->checkPassword(userId, password.c_str());
	info.GetReturnValue().Set(ret == PVSS_TRUE ? Nan::True() : Nan::False());
}

struct alertConnectCallback {
	char** dpList;
	Nan::Callback* cb;
};

std::list<alertConnectCallback*> alertConnectCallbacks;

NAN_METHOD(alertConnect) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 2) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	for (int i = 0; i < info.Length() - 1; i++) {
		if (!info[i]->IsString()) {
			ThrowTypeError("Not all alert configs are strings");
			return;
		}
	}

	if (!info[info.Length() - 1]->IsFunction()) {
		ThrowTypeError("cb is not a function");
		return;
	}

	char** dpList = new char* [info.Length() - 1];
	for (int i = 0; i < info.Length() - 1; i++) {
		v8::String::Utf8Value dpString(info.GetIsolate(), info[i]);
		std::string dpListItem = std::string(*dpString);
		dpList[i] = new char[dpListItem.length() + 1];
		strncpy(dpList[i], dpListItem.c_str(), dpListItem.length() + 1);
	}

	Nan::Callback* cb = new Nan::Callback(info[info.Length() - 1].As<v8::Function>());
	alertConnectCallback* accb = new alertConnectCallback;
	accb->cb = cb;
	accb->dpList = dpList;
	alertConnectCallbacks.push_back(accb);
	AlertConnectCallback* alertConnectCB = new AlertConnectCallback(dpList, cb, info.Length() - 1);
	PVSSboolean success = manager->alertConnect(alertConnectCB);
	info.GetReturnValue().Set(success == PVSS_TRUE ? Nan::True() : Nan::False());
}

NAN_METHOD(alertSet) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 3) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("DP Element is not a string");
		return;
	}

	if (!info[1]->IsDate()) {
		ThrowTypeError("Alert time is not a Date");
		return;
	}

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);
	std::string dp = std::string(*v8String);
	double time = info[1].As<v8::Date>()->NumberValue(info.GetIsolate()->GetCurrentContext()).ToChecked();
	int value = info[2].As<v8::Integer>()->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
	PVSSboolean res = manager->alertSet(dp.c_str(), time, value);
	info.GetReturnValue().Set(res ? Nan::True() : Nan::False());
	return;
}

NAN_METHOD(dpGetDescription) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 1) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("DP Element is not a string");
		return;
	}

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);
	std::string dp = std::string(*v8String);

	char desc[1024];
	memset(desc, 0, 1024);

	PVSSboolean res = manager->dpGetDescription(dp.c_str(), desc);
	if(res)
		info.GetReturnValue().Set(Nan::New(desc).ToLocalChecked());
	else
		info.GetReturnValue().Set(Nan::Null());
	return;
}

NAN_METHOD(dpGetUnit) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 1) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("DP Element is not a string");
		return;
	}

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);
	std::string dp = std::string(*v8String);

	char unit[1024];
	memset(unit, 0, 1024);

	PVSSboolean res = manager->dpGetUnit(dp.c_str(), unit);
	if (res)
		info.GetReturnValue().Set(Nan::New(unit).ToLocalChecked());
	else
		info.GetReturnValue().Set(Nan::Null());
	return;
}

NAN_METHOD(dpGetFormat) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 1) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("DP Element is not a string");
		return;
	}

	v8::String::Utf8Value v8String(info.GetIsolate(), info[0]);
	std::string dp = std::string(*v8String);

	char format[1024];
	memset(format, 0, 1024);

	PVSSboolean res = manager->dpGetFormat(dp.c_str(), format);
	if (res)
		info.GetReturnValue().Set(Nan::New(format).ToLocalChecked());
	else
		info.GetReturnValue().Set(Nan::Null());
	return;
}

NAN_METHOD(dpNames) {
	if (manager == NULL) {
		ThrowTypeError("Must call connect first");
		return;
	}

	if (info.Length() < 1) {
		ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsString()) {
		ThrowTypeError("dpPattern is not a string");
		return;
	}

	if (info.Length() == 2) {
		if (!info[1]->IsString()) {
			ThrowTypeError("dpType is not a string");
			return;
		}
	}

	v8::String::Utf8Value v8DpPattern(info.GetIsolate(), info[0]);
	std::string dpPattern = std::string(*v8DpPattern);

	std::string dpType = "*";
	if (info.Length() == 2) {
		v8::String::Utf8Value v8DpType(info.GetIsolate(), info[1]);
		dpType = std::string(*v8DpType);
	}

	char** names;
	int count;
	PVSSboolean ret = manager->dpNames(dpPattern.c_str(), dpType.c_str(), names, count);
	if (ret) {
		auto arr = Nan::New<v8::Array>();
		for (int i = 0; i < count; i++) {
			Nan::Set(arr, i, Nan::New(names[i]).ToLocalChecked());
		}
		delete[] names;
		info.GetReturnValue().Set(arr);
	}
	else {
		info.GetReturnValue().Set(Nan::Null());
	}
	return;
}

NAN_MODULE_INIT(Init) {
	NAN_EXPORT(target, connect);
	NAN_EXPORT(target, disconnect);
	NAN_EXPORT(target, dpGet);
	NAN_EXPORT(target, dpSet);
	NAN_EXPORT(target, dpConnect);
	NAN_EXPORT(target, dpDisconnect);
	NAN_EXPORT(target, dpQuery);
	NAN_EXPORT(target, dpElementType);
	NAN_EXPORT(target, dpExists);
	NAN_EXPORT(target, getTypeName);
	NAN_EXPORT(target, getUserId);
	NAN_EXPORT(target, checkPassword);
	NAN_EXPORT(target, alertConnect);
	NAN_EXPORT(target, alertSet);
	NAN_EXPORT(target, dpGetDescription);
	NAN_EXPORT(target, dpGetUnit);
	NAN_EXPORT(target, dpGetFormat);
	NAN_EXPORT(target, dpNames);
}

NODE_MODULE(winccoanodejs, Init)