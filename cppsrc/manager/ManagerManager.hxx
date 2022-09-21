// Declaration of our ManagerManager-class
#ifndef  MANAGER_H
#define  MANAGER_H

#include   <Manager.hxx>        // include/Manager
#include   <DpIdentifier.hxx>   // include/Basics
#include "../AnswerCallback.h"
#include "../AlertConnectCallback.h"
#include "../ConnectAnswerCallback.h"
#include <queue>
#include <string>
#include <uv.h>
#include <mutex>

class ManagerManager : public Manager
{
  public:
    // Default constructor
    ManagerManager();
	~ManagerManager();

	void disconnect();
	bool connect();

	PVSSboolean dpSetManager(const char* dp, DpIdentifier*, const Variable&, AnswerCallback*);
	PVSSboolean dpSet(const char*, bool, AnswerCallback*);
	PVSSboolean dpSet(const char*, const char*, AnswerCallback*);
	PVSSboolean dpSet(const char*, int, AnswerCallback*);
	PVSSboolean dpSet(const char*, double, AnswerCallback*);
	PVSSboolean dpSetDate(const char*, double, AnswerCallback*);
	PVSSboolean dpSetStringArray(const char*, std::vector<std::string>, AnswerCallback*);

	PVSSboolean dpConnect(const char*, ConnectAnswerCallback*);
	PVSSboolean dpDisconnect(const char*, ConnectAnswerCallback*);

	PVSSboolean dpGet(const char*, AnswerCallback*);
	PVSSboolean dpQuery(const char*, AnswerCallback*);
	PVSSboolean dpElementType(const char*, int*);
	PVSSboolean dpExists(const char*);
	DpIdentifier* getDpIdentifier(const char*);
	PVSSboolean getTypeName(const char*, const char*, CharString&, CharString&);
	PVSSboolean checkPassword(PVSSuserIdType id, const char* password);
	PVSSuserIdType getUserId(const char* username);

	PVSSboolean alertConnect(AlertConnectCallback*);
	PVSSboolean alertSet(const char* dpe, double time, int value);
	PVSSboolean dpGetDescription(const char* dpe, char* outDescription);
	PVSSboolean dpGetUnit(const char* dpe, char* outUnit);
	PVSSboolean dpGetFormat(const char* dpe, char* outFormat);
	PVSSboolean dpNames(const char* dpPattern, const char* dpType, char**& names, int& count);

	PVSSboolean getDoExit() {
		return doExit;
	}

	friend void runDispatch(ManagerManager*);

  private:
    static PVSSboolean doExit;
	std::mutex managerAccessMtx;
	std::thread* dispatchThread;

    // callback from signal handler
    virtual void signalHandler(int sig);
};

#endif
