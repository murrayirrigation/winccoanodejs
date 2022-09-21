#include "ConnectAnswerCallback.h"
#include "CallbackProcessor.h"
#include "manager/AnswerData.h"
#include "manager/AnswerValue.h"

ConnectAnswerCallback::ConnectAnswerCallback(Nan::Callback* cb, std::string dp) {
	this->cb = cb;
	this->dp = dp;
}

ConnectAnswerCallback::~ConnectAnswerCallback() {
}

void ConnectAnswerCallback::hotLinkCallBack(DpMsgAnswer& answer) { }

void ConnectAnswerCallback::hotLinkCallBack(DpHLGroup& dpGroup) {
	AnswerData* ad = new AnswerData;
	ad->values = new std::vector<AnswerValue*>;
	if (dpGroup.getNumberOfItems() > 0) {
		DpVCItem* item = dpGroup.getFirstItem();
		while (item) {
			ad->values->push_back(AnswerValue::ParseDpVCItem(item));
			item = dpGroup.getNextItem();
		}
	}
	ad->cb = cb;
	ad->resetCallback = false;
	ad->err = NULL;
	CallbackProcessor::SendAnswerData(ad);
}