#include "ManagerManager.hxx"
#include "ManagerResources.hxx"
#include <HotLinkWaitForAnswer.hxx>   // include/Manager
#include <StartDpInitSysMsg.hxx>      // include/Messages
#include <DpMsgAnswer.hxx>            // include/Messages
#include <DpMsgHotLink.hxx>           // include/Messages
#include <DpHLGroup.hxx>              // include/Basics
#include <DpVCItem.hxx>               // include/Basics
#include <ErrHdl.hxx>                 // include/Basics
#include <ErrClass.hxx>               // include/Basics
#include <DpIdentifier.hxx>
#include <DpIdentifierVar.hxx>
#include <BitVar.hxx>
#include <TextVar.hxx>
#include <IntegerVar.hxx>
#include <FloatVar.hxx>
#include <TimeVar.hxx>
#include <DynVar.hxx>
#include <LangTextVar.hxx>
#include <signal.h>
#include <thread>
#include <uv.h>
#include <ctime>
#include <mutex>

PVSSboolean ManagerManager::doExit = PVSS_FALSE;

void runDispatch(ManagerManager* manager) {
	// Check for messages and exit dispatch
	long sec = 0;
	long usec = 0;
	while (manager->doExit != PVSS_TRUE) {
		{
			std::lock_guard<std::mutex> lock(manager->managerAccessMtx);
			manager->dispatch(sec, usec);
		}
		// Sleep before we check again
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

ManagerManager::ManagerManager()
    : Manager(ManagerIdentifier(API_MAN, Resources::getManNum())) {
}

void ManagerManager::signalHandler(int sig) {
    if ((sig == SIGINT) || (sig == SIGTERM))
        ManagerManager::doExit = PVSS_TRUE;
    else
        Manager::signalHandler(sig);
}

void ManagerManager::disconnect() {
	doExit = PVSS_TRUE;
	dispatchThread->join();
	delete dispatchThread;
}

bool ManagerManager::connect() {
    long sec, usec;
    // First connect to Data manager.
    // We want Typecontainer and Identification so we can resolve names
    // This call succeeds or the manager will exit
    PVSSboolean success = connectToData(StartDpInitSysMsg::TYPE_CONTAINER | StartDpInitSysMsg::DP_IDENTIFICATION, 0, PVSS_TRUE);
    if (success != PVSS_TRUE)
        return false;

    // While we are in STATE_INIT we are initialized by the Data manager
    while (getManagerState() == STATE_INIT)
    {
        // Wait max. 1 second in select to receive next message from data.
        // It won't take that long...
        sec = 1;
        usec = 0;
        dispatch(sec, usec);
    }

    // We are now in STATE_ADJUST and can connect to Event manager
    // This call will succeed or the manager will exit
    success = connectToEvent(PVSS_TRUE);
    if (success != PVSS_TRUE)
        return false;

	dispatchThread = new std::thread(runDispatch, this);
    return true;
}

ManagerManager::~ManagerManager() {
	std::cerr << "Error: destructor should not be called for ManagerManager" << std::endl;
}

DpIdentifier* ManagerManager::getDpIdentifier(const char* dp) {
	PVSSboolean ret;
	DpIdentifier* id = new DpIdentifier;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::getId(dp, *id);
	}
	if (ret == PVSS_FALSE) {
		CharString err = CharString("Could not get DpIdentifier for ") + CharString(dp);
		ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
			ErrClass::ERR_CONTROL,
			ErrClass::UNEXPECTEDSTATE,  // fits all
			"ManagerManager",              // our file name
			"getDpIdentifier",                      // our function name
			err);
		return NULL;
	}
	return id;
}

PVSSboolean ManagerManager::dpGet(const char* dp, AnswerCallback* callback) {
	PVSSboolean ret;
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::dpGet(*id, callback);
	}
	if (ret == PVSS_FALSE) {
        CharString err = CharString("dpGet failed for ") + CharString(dp);
        ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
            ErrClass::ERR_CONTROL,
            ErrClass::UNEXPECTEDSTATE,  // fits all
            "ManagerManager",              // our file name
            "dpGet",                      // our function name
            err);
    }
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpSetManager(const char* dp, DpIdentifier* id, const Variable& value, AnswerCallback* cb) {
	TimeVar time;
	PVSSboolean ret;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::dpSetTimed(time, *id, value, cb);
	}
	if (ret == PVSS_FALSE) {
		CharString err = CharString("dpSet failed for ") + CharString(dp);
		ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
			ErrClass::ERR_CONTROL,
			ErrClass::UNEXPECTEDSTATE,  // fits all
			"ManagerManager",              // our file name
			"dpSetManager",                      // our function name
			err);
	}
	return ret;
}

PVSSboolean ManagerManager::dpSet(const char* dp, bool value, AnswerCallback* cb) {
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	const BitVar bitVar(value ? PVSS_TRUE : PVSS_FALSE);
	PVSSboolean ret = dpSetManager(dp, id, bitVar, cb);
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpSet(const char* dp, const char* value, AnswerCallback* cb) {
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	DpElementType type;
	PVSSboolean ret = Manager::getElementType(*id, type);
	if (!ret) {
		const TextVar textVar(value, strlen(value));
		ret = dpSetManager(dp, id, textVar, cb);
	}
	else {
		if (type == DpElementType::DPELEMENT_DPID) {
			DpIdentifier* dpVal = getDpIdentifier(value);
			if (dpVal == NULL)
				return PVSS_FALSE;
			const DpIdentifierVar dpVar(*dpVal);
			ret = dpSetManager(dp, id, dpVar, cb);
			delete dpVal;
		}
		else {
			const TextVar textVar(value, strlen(value));
			ret = dpSetManager(dp, id, textVar, cb);
		}
	}
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpSet(const char* dp, int value, AnswerCallback* cb) {
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	const IntegerVar intVar(value);
	PVSSboolean ret = dpSetManager(dp, id, intVar, cb);
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpSet(const char* dp, double value, AnswerCallback* cb) {
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	const FloatVar floatVar(value);
	PVSSboolean ret = dpSetManager(dp, id, floatVar, cb);
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpSetDate(const char* dp, double value, AnswerCallback* cb) {
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	time_t ts = (unsigned long)(value / 1000);
	PVSSshort milli = ((long long)value) % 1000;
	PVSSTime t(ts, milli);
	const TimeVar timeVar(t);
	PVSSboolean ret = dpSetManager(dp, id, timeVar, cb);
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpSetStringArray(const char* dp, std::vector<std::string> array, AnswerCallback* cb) {
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	DynVar dynVar(TEXT_VAR);
	for (std::string s : array) {
		dynVar.append(new TextVar(s.c_str(), strlen(s.c_str())));
	}
	PVSSboolean ret = dpSetManager(dp, id, dynVar, cb);
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpConnect(const char* dp, ConnectAnswerCallback* cb) {
	PVSSboolean ret;
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::dpConnect(*id, cb, PVSS_FALSE);
	}
	if (ret == PVSS_FALSE) {
		CharString err = CharString("dpConnect failed for ") + CharString(dp);
		ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
			ErrClass::ERR_CONTROL,
			ErrClass::UNEXPECTEDSTATE,  // fits all
			"ManagerManager",              // our file name
			"dpConnect",                      // our function name
			err);
	}
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpDisconnect(const char* dp, ConnectAnswerCallback* cb) {
	PVSSboolean ret;
	DpIdentifier* id = getDpIdentifier(dp);
	if (id == NULL)
		return PVSS_FALSE;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::dpDisconnect(*id, cb);
	}
	if (ret == PVSS_FALSE) {
		CharString err = CharString("dpDisconnect failed for ") + CharString(dp);
		ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
			ErrClass::ERR_CONTROL,
			ErrClass::UNEXPECTEDSTATE,  // fits all
			"ManagerManager",              // our file name
			"dpDisconnect",                      // our function name
			err);
	}
	delete id;
	return ret;
}

PVSSboolean ManagerManager::dpQuery(const char* query, AnswerCallback* cb) {
	PVSSboolean ret;
	CharString csQuery(query, strlen(query));
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::dpQuery(csQuery, cb);
	}
	if (ret == PVSS_FALSE) {
		CharString err = CharString("dpQuery failed for ") + csQuery;
		ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
			ErrClass::ERR_CONTROL,
			ErrClass::UNEXPECTEDSTATE,  // fits all
			"ManagerManager",
			"dpQuery",
			err);
	}
	return ret;
}

PVSSboolean ManagerManager::dpElementType(const char* dp, int* type) {
	PVSSboolean ret;
	DpIdentifier id;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::getId(dp, id);
	}
	*type = 0;
	if (ret == PVSS_FALSE)
		return PVSS_FALSE;
	DpElementType dpType;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::getElementType(id, dpType);
	}
	if (ret == PVSS_FALSE)
		return PVSS_FALSE;
	*type = (int)dpType;
	return PVSS_TRUE;
}

PVSSboolean ManagerManager::dpExists(const char* dp) {
	DpIdentifier id;
	std::lock_guard<std::mutex> lock(managerAccessMtx);
	return Manager::getId(dp, id);
}

PVSSboolean ManagerManager::getTypeName(const char* dp, const char* system, CharString& type, CharString& err) {
	PVSSboolean ret;
	DpIdentifier id;
	CharString fullDp = system;
	fullDp = fullDp + ":" + dp;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::getId(fullDp, id);
	}
	if (ret == PVSS_FALSE) {
		err = "DP does not exist";
		return PVSS_FALSE;
	}

	SystemNumType sysNum;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::getSystemId(system, sysNum);
	}
	if (ret == PVSS_FALSE) {
		err = "System does not exist";
		return PVSS_FALSE;
	}
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::getTypeName(id.getDpType(), type, sysNum);
	}
	if (ret == PVSS_FALSE) {
		err = "Could not get type name";
		return PVSS_FALSE;
	}

	return PVSS_TRUE;
}

PVSSboolean ManagerManager::checkPassword(PVSSuserIdType id, const char* password) {
	std::lock_guard<std::mutex> lock(managerAccessMtx);
	return Manager::checkPassword(id, password);
}

PVSSuserIdType ManagerManager::getUserId(const char* username) {
	std::lock_guard<std::mutex> lock(managerAccessMtx);
	return Manager::getUserId(username);
}

PVSSboolean ManagerManager::alertConnect(AlertConnectCallback* cb) {
	DpIdentList* dpList = new DpIdentList();
	for (int i = 0; i < cb->getDpCount(); i++) {
		DpIdentifier* dpId = new DpIdentifier();
		dpId->convertFromString(cb->getDpList()[i]);
		dpList->append(*dpId);
	}
	PVSSboolean ret;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::alertConnect(*dpList, cb, PVSS_FALSE);
	}
	cb->setDpIdentList(dpList);
	return ret;
}

PVSSboolean ManagerManager::alertSet(const char* dpe, double time, int value) {
	PVSSboolean ret;
	AlertList list;
	DpIdentifier dpId;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		ret = Manager::getId(dpe, dpId);
	}
	if (!ret)
		return PVSS_FALSE;
	PVSSTime pTime(time / 1000);
	AlertTime alertTime(pTime);
	AlertIdentifier alId(dpId, alertTime);
	IntegerVar *intValue = new IntegerVar(value);
	list.insertAttribute(alId, intValue);

	std::lock_guard<std::mutex> lock(managerAccessMtx);
	return Manager::alertSet(list);
}

PVSSboolean ManagerManager::dpGetDescription(const char* dpe, char* outDescription) {
	PVSSboolean res;
	DpIdentifier dpId;
	LangTextVar ltv;
	DpIdentificationResult ret;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		res = Manager::getId(dpe, dpId);
		if (!res)
			return PVSS_FALSE;
		ret = Manager::dpGetComment(dpId, ltv);
	}
	if (ret == DpIdentOK) {
		auto text = ltv.getValue().getText();
		strncpy(outDescription, text.c_str(), text.len());
		return PVSS_TRUE;
	}
	else {
		return PVSS_FALSE;
	}
}

PVSSboolean ManagerManager::dpGetUnit(const char* dpe, char* outUnit) {
	PVSSboolean res;
	DpIdentifier dpId;
	LangTextVar ltv;
	DpIdentificationResult ret;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		res = Manager::getId(dpe, dpId);
		if (!res)
			return PVSS_FALSE;
		ret = Manager::dpGetUnit(dpId, ltv);
	}
	if (ret == DpIdentOK) {
		auto text = ltv.getValue().getText();
		strncpy(outUnit, text.c_str(), text.len());
		return PVSS_TRUE;
	}
	else {
		return PVSS_FALSE;
	}
}

PVSSboolean ManagerManager::dpGetFormat(const char* dpe, char* outFormat) {
	PVSSboolean res;
	DpIdentifier dpId;
	LangTextVar ltv;
	DpIdentificationResult ret;
	{
		std::lock_guard<std::mutex> lock(managerAccessMtx);
		res = Manager::getId(dpe, dpId);
		if (!res)
			return PVSS_FALSE;
		ret = Manager::dpGetFormat(dpId, ltv);
	}
	if (ret == DpIdentOK) {
		auto text = ltv.getValue().getText();
		strncpy(outFormat, text.c_str(), text.len());
		return PVSS_TRUE;
	}
	else {
		return PVSS_FALSE;
	}
}

PVSSboolean ManagerManager::dpNames(const char* dpPattern, const char* dpType, char**& names, int& count) {
	PVSSboolean res;
	CharString csPattern(dpPattern);
	CharString csDpType(dpType);

	int index = csPattern.indexOf(":");
	SystemNumType sysNum = DpIdentification::getDefaultSystem();
	if (index != -1) {
		//possible system reference. Could be a dp reference though. Check to see if it matches a system name
		
		CharString csSystem = csPattern.substring(0, index);
		res = Manager::getSystemId(csSystem, sysNum);
		if(!res)
			sysNum = DpIdentification::getDefaultSystem();
	}

	DpIdentifier* dpIdArr;
	PVSSlong howMany;
	DpTypeId dpTypeId = 0;

	if (csDpType.cmp("*") != 0) {
		res = Manager::getTypeId(csDpType, dpTypeId, sysNum);
		if (!res)
			dpTypeId = 0;
	}

	res = Manager::getIdSet(dpPattern, dpIdArr, howMany, dpTypeId);
	if (!res)
		return res;

	names = new char* [howMany];
	for (int i = 0; i < howMany; i++) {
		Manager::getLIName(dpIdArr[i], names[i]);
	}

	count = howMany;
	return PVSS_TRUE;
}