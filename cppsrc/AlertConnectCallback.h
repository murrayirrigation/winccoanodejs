#ifndef __ALERTCONNECT_CALLBACK__
#define __ALERTCONNECT_CALLBACK__

#include <AlertHotLinkWaitForAnswer.hxx>
#include <DpIdentList.hxx>
#include <nan.h>

class AlertConnectCallback : public AlertHotLinkWaitForAnswer {
public:
	AlertConnectCallback(char**, Nan::Callback*, int);
	~AlertConnectCallback();
	virtual void alertHotLinkCallBack(DpMsgAnswer& answer);
	Nan::Callback* getCB();
	char** getDpList();
	int getDpCount();
	DpIdentList* getDpIdentList();
	void setDpIdentList(DpIdentList*);
private:
	char** dpList;
	Nan::Callback* cb;
	int dpCount;
	DpIdentList* dpIdentList;
protected:
	virtual void alertHotLinkCallBack(AlertAttrList& group);
};

#endif //__ALERTCONNECT_CALLBACK__