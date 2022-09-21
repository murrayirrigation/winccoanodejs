#ifndef __AnswerCallback_H_
#define __AnswerCallback_H_

#include <WaitForAnswer.hxx>
#include <Variable.hxx>
#include <AnswerGroup.hxx>
#include <DpMsgAnswer.hxx>
#include <nan.h>
#include <v8.h>
#include <vector>

class AnswerCallback : public WaitForAnswer {
public:
	AnswerCallback(Nan::Callback*);
	~AnswerCallback();

	virtual void callBack(DpMsgAnswer&);

private:
	Nan::Callback* cb;
};

#endif // !__AnswerCallback_H_