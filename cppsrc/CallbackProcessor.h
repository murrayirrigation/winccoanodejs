#pragma once
#include <mutex>
#include "manager/AnswerData.h"

class CallbackProcessor
{
public:
	CallbackProcessor();
	~CallbackProcessor();
	static void SendAnswerData(AnswerData*);
	friend void processCallbacks(uv_async_t*);

protected:
	void addCallback(AnswerData*);
	AnswerData* nextCallback();
	unsigned int callbackCount();
	uv_async_t* processorHandle;

private:
	std::queue<AnswerData*> callbacks;
	std::mutex callbackQueueMtx;
};

