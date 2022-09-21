#pragma once

#include <Variable.hxx>
#include <AnswerGroup.hxx>
#include <DpMsgAnswer.hxx>
#include <v8.h>
#include <nan.h>

struct AnswerValue {
	VariableType type = VariableType::NO_VAR;
	bool bitVar = false;
	char* textVar = NULL;
	float floatVar = 0;
	int intVar = 0;
	unsigned int uintVar = 0;
	long long timeVar = 0; //JS timestamp from UTC format
	int dynlen = 0;
	AnswerValue** dyn = NULL;
	bool isNull = false;

	static AnswerValue* ParseAnswerItem(AnswerItem*);
	static AnswerValue* ParseDpVCItem(DpVCItem*);
	static void DeleteAnswerValueVector(std::vector<AnswerValue*>*);
	static void DeleteAnswerValue(AnswerValue*);
	static v8::Local<v8::Array> ToJSValuesArray(std::vector<AnswerValue*>*);

private:
	static AnswerValue** ProcessDynVar(DynVar*);
	static AnswerValue* ProcessVar(VariablePtr);
	static v8::Local<v8::Array> ToJSValuesArrayDyn(AnswerValue*);
};