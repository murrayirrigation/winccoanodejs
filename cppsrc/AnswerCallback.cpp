#include <nan.h>
#include "AnswerCallback.h"
#include "CallbackProcessor.h"
#include "manager/AnswerValue.h"
#include "manager/AnswerData.h"

AnswerCallback::AnswerCallback(Nan::Callback* callback) {
	this->cb = callback;
}

AnswerCallback::~AnswerCallback() {
}

// Note: AnswerCallback will be destroyed at the end of this callback by Manager
void AnswerCallback::callBack(DpMsgAnswer& answer) {
	AnswerData* ad = new AnswerData;
	ad->values = new std::vector<AnswerValue*>;

	if (answer.getNrOfGroups() > 0) {
		AnswerGroup* answerGroup = answer.getFirstGroup();
		while (answerGroup) {
			unsigned int grNumItems = answerGroup->getNrOfItems();
			if (grNumItems > 0) {
				AnswerItem* item = answerGroup->getFirstItem();
				while (item) {
					ad->values->push_back(AnswerValue::ParseAnswerItem(item));
					item = answerGroup->getNextItem();
				}
			}
			answerGroup = answer.getNextGroup();
		}
	}

	ad->cb = cb;
	ad->resetCallback = true;
	ad->err = NULL;
	CallbackProcessor::SendAnswerData(ad);
}
