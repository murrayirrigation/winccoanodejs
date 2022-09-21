#pragma once
#include <nan.h>
#include "AnswerValue.h"

struct AnswerData {
	Nan::Callback* cb;
	std::vector<AnswerValue*> *values;
	AnswerValue* err = NULL;
	boolean resetCallback;
};
