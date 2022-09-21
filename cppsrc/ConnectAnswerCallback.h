#ifndef __CONNECT_ANSWER_CALLBACK_H_
#define __CONNECT_ANSWER_CALLBACK_H_

#include <HotLinkWaitForAnswer.hxx>
#include <v8.h>
#include <nan.h>
#include <vector>
#include "manager/AnswerData.h"

struct AnswerData;

class ConnectAnswerCallback : public HotLinkWaitForAnswer {
public:
	ConnectAnswerCallback(Nan::Callback*, std::string dp);
	~ConnectAnswerCallback();
	virtual void hotLinkCallBack(DpMsgAnswer&);
	virtual void hotLinkCallBack(DpHLGroup&);
	const char* getDp() {
		return this->dp.c_str();
	}

private:
	Nan::Callback* cb;
	std::string dp;
};

#endif //__CONNECT_ANSWER_CALLBACK_H_