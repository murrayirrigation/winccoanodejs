#include <stdlib.h>
#include <Variable.hxx>
#include <BitVar.hxx>
#include <TextVar.hxx>
#include <DynVar.hxx>
#include <IntegerVar.hxx>
#include <FloatVar.hxx>
#include <TimeVar.hxx>
#include <CharVar.hxx>
#include <ExtTimeVar.hxx>
#include <LangTextVar.hxx>
#include <UIntegerVar.hxx>
#include <AnyTypeVar.hxx>
#include <DpIdentifierVar.hxx>
#include <DpVCItem.hxx>
#include <ErrHdl.hxx>
#include <ErrClass.hxx>
#include <v8.h>
#include <nan.h>
#include "AnswerValue.h"
#include "AnswerData.h"

AnswerValue* AnswerValue::ParseAnswerItem(AnswerItem* item) {
	return ProcessVar(item->getValuePtr());
}

AnswerValue* AnswerValue::ParseDpVCItem(DpVCItem* item)
{
	return ProcessVar(item->getValuePtr());
}

AnswerValue** AnswerValue::ProcessDynVar(DynVar* dynVar) {
	auto dynlen = dynVar->getArrayLength();
	auto ret = new AnswerValue *[dynlen];
	auto var = dynVar->getFirstVar();
	int i = 0;
	while (var) {
		ret[i] = ProcessVar(var->getTarget());
		var = dynVar->getAt(++i);
	}
	return ret;
}

AnswerValue* AnswerValue::ProcessVar(VariablePtr var) {
	AnswerValue* av = new AnswerValue;
	DynVar* dyn = NULL;
	TimeVar* time;
	ExtTimeVar* exttime;
	CharString dp;
	if (var == NULL) {
		av->isNull = true;
		return av;
	}

	av->type = var->isA();
	switch (var->isA()) {
	case VariableType::BIT_VAR:
		av->bitVar = ((BitVar*)var)->getValue() == PVSS_TRUE;
		break;
	case VariableType::TEXT_VAR:
		av->textVar = strdup(((TextVar*)var)->getValue());
		break;
	case VariableType::CHAR_VAR:
		av->uintVar = ((CharVar*)var)->getValue();
		break;
	case VariableType::LANGTEXT_VAR:
		av->textVar = strdup(((LangTextVar*)var)->getValue().getText());
		break;
	case VariableType::INTEGER_VAR:
		av->intVar = ((IntegerVar*)var)->getValue();
		break;
	case VariableType::FLOAT_VAR:
		av->floatVar = ((FloatVar*)var)->getValue();
		break;
	case VariableType::TIME_VAR:
		time = (TimeVar*)var;
		av->timeVar = time->getSeconds() * 1000 + (long)time->getMilliSeconds();
		break;
	case VariableType::EXTTIME_VAR:
		exttime = (ExtTimeVar*)var;
		av->timeVar = exttime->getSeconds() * 1000 + (long)exttime->getMilliSeconds();
		break;
	case VariableType::UINTEGER_VAR:
		av->uintVar = ((UIntegerVar*)var)->getValue();
		break;
	case VariableType::DPIDENTIFIER_VAR:
		av->type = VariableType::TEXT_VAR;
		((DpIdentifierVar*)var)->getValue().convertToString(dp);
		av->textVar = strdup(dp.c_str());
		break;
	case VariableType::DYNANYTYPE_VAR:
	case VariableType::DYNDYNANYTYPE_VAR:
	case VariableType::DYNDYNBIT_VAR:
	case VariableType::DYNDYNINTEGER_VAR:
	case VariableType::DYNDYNFLOAT_VAR:
	case VariableType::DYNDYNTEXT_VAR:
	case VariableType::DYNDYNTIME_VAR:
	case VariableType::DYNBIT_VAR:
	case VariableType::DYNINTEGER_VAR:
	case VariableType::DYNFLOAT_VAR:
	case VariableType::DYNTEXT_VAR:
	case VariableType::DYNTIME_VAR:
		dyn = (DynVar*)var;
		av->dyn = ProcessDynVar(dyn);
		av->dynlen = dyn->getArrayLength();
		break;
	default:
		break;
	}
	return av;
}

v8::Local<v8::Array> AnswerValue::ToJSValuesArrayDyn(AnswerValue* item) {
	Nan::EscapableHandleScope scope;
	v8::Local<v8::Array> arr = Nan::New<v8::Array>(item->dynlen);
	for (int i = 0; i < item->dynlen; i++) {
		AnswerValue* var = item->dyn[i];
		if (var->isNull) {
			Nan::Set(arr, i, Nan::Null());
		}
		else {
			switch (var->type) {
			case VariableType::BIT_VAR:
				Nan::Set(arr, i, Nan::New<v8::Boolean>(var->bitVar));
				break;
			case VariableType::TEXT_VAR:
			case VariableType::LANGTEXT_VAR:
				Nan::Set(arr, i, Nan::New(var->textVar).ToLocalChecked());
				break;
			case VariableType::INTEGER_VAR:
				Nan::Set(arr, i, Nan::New<v8::Integer>(var->intVar));
				break;
			case VariableType::FLOAT_VAR:
				Nan::Set(arr, i, Nan::New<v8::Number>(var->floatVar));
				break;
			case VariableType::TIME_VAR:
			case VariableType::EXTTIME_VAR:
				Nan::Set(arr, i, Nan::New<v8::Date>(var->timeVar).ToLocalChecked());
				break;
			case VariableType::UINTEGER_VAR:
			case VariableType::CHAR_VAR:
				Nan::Set(arr, i, Nan::New<v8::Uint32>(var->uintVar));
				break;
			case VariableType::DYNANYTYPE_VAR:
			case VariableType::DYNDYNANYTYPE_VAR:
			case VariableType::DYNDYNBIT_VAR:
			case VariableType::DYNDYNINTEGER_VAR:
			case VariableType::DYNDYNFLOAT_VAR:
			case VariableType::DYNDYNTEXT_VAR:
			case VariableType::DYNDYNTIME_VAR:
			case VariableType::DYNBIT_VAR:
			case VariableType::DYNINTEGER_VAR:
			case VariableType::DYNFLOAT_VAR:
			case VariableType::DYNTEXT_VAR:
			case VariableType::DYNTIME_VAR:
				Nan::Set(arr, i, ToJSValuesArrayDyn(var));
				break;
			}
		}
	}
	return scope.Escape(arr);
}

v8::Local<v8::Array> AnswerValue::ToJSValuesArray(std::vector<AnswerValue*>* values) {
	Nan::EscapableHandleScope scope;
	size_t valuesSize = values->size();
	v8::Local<v8::Array> arr = Nan::New<v8::Array>(valuesSize);
	for (unsigned int i = 0; i < valuesSize; i++) {
		if ((*values)[i]->isNull) {
			Nan::Set(arr, i, Nan::Null());
		}
		else {
			switch ((*values)[i]->type) {
			case VariableType::BIT_VAR:
				Nan::Set(arr, i, Nan::New((*values)[i]->bitVar));
				break;
			case VariableType::TEXT_VAR:
			case VariableType::LANGTEXT_VAR:
				Nan::Set(arr, i, Nan::New((*values)[i]->textVar).ToLocalChecked());
				break;
			case VariableType::INTEGER_VAR:
				Nan::Set(arr, i, Nan::New<v8::Integer>((*values)[i]->intVar));
				break;
			case VariableType::FLOAT_VAR:
				Nan::Set(arr, i, Nan::New<v8::Number>((*values)[i]->floatVar));
				break;
			case VariableType::TIME_VAR:
			case VariableType::EXTTIME_VAR:
				Nan::Set(arr, i, Nan::New<v8::Date>((*values)[i]->timeVar).ToLocalChecked());
				break;
			case VariableType::UINTEGER_VAR:
			case VariableType::CHAR_VAR:
				Nan::Set(arr, i, Nan::New<v8::Uint32>((*values)[i]->uintVar));
				break;
			case VariableType::DYNANYTYPE_VAR:
			case VariableType::DYNDYNANYTYPE_VAR:
			case VariableType::DYNDYNBIT_VAR:
			case VariableType::DYNDYNINTEGER_VAR:
			case VariableType::DYNDYNFLOAT_VAR:
			case VariableType::DYNDYNTEXT_VAR:
			case VariableType::DYNDYNTIME_VAR:
			case VariableType::DYNBIT_VAR:
			case VariableType::DYNINTEGER_VAR:
			case VariableType::DYNFLOAT_VAR:
			case VariableType::DYNTEXT_VAR:
			case VariableType::DYNTIME_VAR:
				Nan::Set(arr, i, ToJSValuesArrayDyn((*values)[i]));
				break;
			default:
				Nan::Set(arr, i, Nan::Null());
				break;
			}
		}
	}
	return scope.Escape(arr);
}

void AnswerValue::DeleteAnswerValueVector(std::vector<AnswerValue*>* values) {
	while (values->size())
	{
		AnswerValue* av = values->back();
		values->pop_back();
		DeleteAnswerValue(av);
	}
}

void AnswerValue::DeleteAnswerValue(AnswerValue* av) {
	if (av->dyn != NULL) {
		for (int i = 0; i < av->dynlen; i++) {
			DeleteAnswerValue(av->dyn[i]);
		}
		delete[] av->dyn;
	}
	if (av->textVar != NULL) {
		free(av->textVar);
	}
	delete av;
}
