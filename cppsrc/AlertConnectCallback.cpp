#include "AlertConnectCallback.h"
#include "AnswerCallback.h"
#include <LangTextVar.hxx>
#include <DpIdentifierVar.hxx>
#include <ExtTimeVar.hxx>
#include <CharVar.hxx>
#include <IntegerVar.hxx>
#include <FloatVar.hxx>
#include <AnyTypeVar.hxx>
#include "CallbackProcessor.h"
#include "./manager/AnswerValue.h"

void AlertConnectCallback::alertHotLinkCallBack(DpMsgAnswer& msg) {
}

void AlertConnectCallback::alertHotLinkCallBack(AlertAttrList& group) {
	AnswerData* ad = new AnswerData;
	ad->values = new std::vector<AnswerValue*>;

	DpVCItem* item = group.getFirstItem();
	int numItems = group.getNumberOfItems();
	while (item) {
		DpIdentifier dpId = item->getDpIdentifier();
		CharString dpName;
		dpId.convertToString(dpName);
		AnswerValue* dpNameAV = new AnswerValue;
		dpNameAV->textVar = (char*)dpName.c_str();
		dpNameAV->type = TEXT_VAR;
		ad->values->push_back(dpNameAV);
		ad->values->push_back(AnswerValue::ParseDpVCItem(item));
		item = group.getNextItem();
	}

	ad->cb = this->cb;
	ad->err = NULL;
	CallbackProcessor::SendAnswerData(ad);
}

AlertConnectCallback::AlertConnectCallback(char** dpList, Nan::Callback* cb, int dpCount) {
	this->dpList = dpList;
	this->cb = cb;
	this->dpCount = dpCount;
}

AlertConnectCallback::~AlertConnectCallback() {
}

Nan::Callback* AlertConnectCallback::getCB() {
	return this->cb;
}
char** AlertConnectCallback::getDpList() {
	return this->dpList;
}

int AlertConnectCallback::getDpCount() {
	return this->dpCount;
}

DpIdentList* AlertConnectCallback::getDpIdentList() {
	return this->dpIdentList;
}

void AlertConnectCallback::setDpIdentList(DpIdentList* dpIdentList) {
	this->dpIdentList = dpIdentList;
}