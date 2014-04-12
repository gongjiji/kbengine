/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ENTITY_APP_H__
#define __ENTITY_APP_H__
// common include
#include "pyscript/script.hpp"
#include "pyscript/pyprofile.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "pyscript/pywatcher.hpp"
#include "helper/debug_helper.hpp"
#include "helper/script_loglevel.hpp"
#include "helper/profile.hpp"
#include "server/script_timers.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "server/globaldata_client.hpp"
#include "server/globaldata_server.hpp"
#include "server/profile_handler.hpp"
#include "server/callbackmgr.hpp"	
#include "entitydef/entitydef.hpp"
#include "entitydef/entities.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "network/message_handler.hpp"
#include "resmgr/resmgr.hpp"
#include "helper/console_helper.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include "server/serverapp.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

template<class E>
class EntityApp : public ServerApp
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1,
		TIMEOUT_ENTITYAPP_MAX
	};
public:
	EntityApp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~EntityApp();
	
	/** 
		��ش���ӿ� 
	*/
	virtual void handleTimeout(TimerHandle handle, void * arg);
	virtual void handleGameTick();

	/**
		ͨ��entityIDѰ�ҵ���Ӧ��ʵ�� 
	*/
	E* findEntity(ENTITY_ID entityID);

	/** 
		ͨ��entityID����һ��entity 
	*/
	virtual bool destroyEntity(ENTITY_ID entityID, bool callScript);

	/**
		��mailbox�����Ի�ȡһ��entity��ʵ��
		��Ϊ�������ϲ�һ���������entity��
	*/
	PyObject* tryGetEntityByMailbox(COMPONENT_ID componentID, ENTITY_ID eid);

	/**
		��mailbox�����Ի�ȡһ��channel��ʵ��
	*/
	Mercury::Channel* findChannelByMailbox(EntityMailbox& mailbox);

	KBEngine::script::Script& getScript(){ return script_; }
	PyObjectPtr getEntryScript(){ return entryScript_; }

	void registerScript(PyTypeObject*);
	int registerPyObjectToScript(const char* attrName, PyObject* pyObj);
	int unregisterPyObjectToScript(const char* attrName);

	bool installPyScript();
	virtual bool installPyModules();
	virtual void onInstallPyModules() {};
	virtual bool uninstallPyModules();
	bool uninstallPyScript();
	bool installEntityDef();
	
	virtual bool initializeWatcher();

	virtual void finalise();
	virtual bool inInitialize();
	virtual bool initialize();

	virtual void onSignalled(int sigNum);
	
	Entities<E>* pEntities()const{ return pEntities_; }
	ArraySize entitiesSize()const { return pEntities_->size(); }

	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }	

	/**
		����һ��entity 
	*/
	E* createEntityCommon(const char* entityType, PyObject* params,
		bool isInitializeScript = true, ENTITY_ID eid = 0, bool initProperty = true);

	virtual E* onCreateEntityCommon(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid);

	/** ����ӿ�
		�������һ��ENTITY_ID�εĻص�
	*/
	void onReqAllocEntityID(Mercury::Channel* pChannel, ENTITY_ID startID, ENTITY_ID endID);

	/** ����ӿ�
		dbmgr���ͳ�ʼ��Ϣ
		startID: ��ʼ����ENTITY_ID ����ʼλ��
		endID: ��ʼ����ENTITY_ID �ν���λ��
		startGlobalOrder: ȫ������˳�� �������ֲ�ͬ���
		startGroupOrder: ��������˳�� ����������baseapp�еڼ���������
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest);

	/** ����ӿ�
		dbmgr�㲥global���ݵĸı�
	*/
	void onBroadcastGlobalDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);


	/** ����ӿ�
		����ִ��һ��pythonָ��
	*/
	void onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		console����ʼprofile
	*/
	void startProfile(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/**
		��ȡapps����״̬, ���ڽű��л�ȡ��ֵ
	*/
	static PyObject* __py_getAppPublish(PyObject* self, PyObject* args);

	/**
		���ýű��������ǰ׺
	*/
	static PyObject* __py_setScriptLogType(PyObject* self, PyObject* args);

	/**
		���µ������еĽű�
	*/
	virtual void reloadScript(bool fullReload);
	virtual void onReloadScript(bool fullReload);

	/**
		ͨ�����·����ȡ��Դ��ȫ·��
	*/
	static PyObject* __py_getResFullPath(PyObject* self, PyObject* args);

	/**
		open�ļ�
	*/
	static PyObject* __py_kbeOpen(PyObject* self, PyObject* args);
protected:
	KBEngine::script::Script								script_;
	std::vector<PyTypeObject*>								scriptBaseTypes_;

	PyObjectPtr												entryScript_;

	EntityIDClient											idClient_;

	// �洢���е�entity������
	Entities<E>*											pEntities_;										

	TimerHandle												gameTimer_;

	// globalData
	GlobalDataClient*										pGlobalData_;									

	PY_CALLBACKMGR											pyCallbackMgr_;
};


template<class E>
EntityApp<E>::EntityApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
ServerApp(dispatcher, ninterface, componentType, componentID),
script_(),
scriptBaseTypes_(),
entryScript_(),
idClient_(),
pEntities_(NULL),
gameTimer_(),
pGlobalData_(NULL),
pyCallbackMgr_()
{
	ScriptTimers::initialize(*this);
	idClient_.pApp(this);

	// ��ʼ��mailboxģ���ȡentityʵ�庯����ַ
	EntityMailbox::setGetEntityFunc(std::tr1::bind(&EntityApp<E>::tryGetEntityByMailbox, this, 
		std::tr1::placeholders::_1, std::tr1::placeholders::_2));

	// ��ʼ��mailboxģ���ȡchannel������ַ
	EntityMailbox::setFindChannelFunc(std::tr1::bind(&EntityApp<E>::findChannelByMailbox, this, 
		std::tr1::placeholders::_1));
}

template<class E>
EntityApp<E>::~EntityApp()
{
}

template<class E>
bool EntityApp<E>::inInitialize()
{
	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	return installEntityDef();
}

template<class E>
bool EntityApp<E>::initialize()
{
	bool ret = ServerApp::initialize();
	if(ret)
	{
		gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
								reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	}
	return ret;
}

template<class E>
bool EntityApp<E>::initializeWatcher()
{
	WATCH_OBJECT("entitiesSize", this, &EntityApp<E>::entitiesSize);
	return ServerApp::initializeWatcher();
}

template<class E>
void EntityApp<E>::finalise(void)
{
	gameTimer_.cancel();

	WATCH_FINALIZE;
	
	pyCallbackMgr_.finalise();
	ScriptTimers::finalise(*this);
	pEntities_->finalise();
	
	uninstallPyScript();

	ServerApp::finalise();
}

template<class E>
bool EntityApp<E>::installEntityDef()
{
	if(!EntityDef::installScript(this->getScript().getModule()))
		return false;

	// ��ʼ��������չģ��
	// demo/res/scripts/
	if(!EntityDef::initialize(scriptBaseTypes_, componentType_)){
		return false;
	}

	return true;
}

template<class E>
int EntityApp<E>::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

template<class E>
int EntityApp<E>::unregisterPyObjectToScript(const char* attrName)
{ 
	return script_.unregisterToModule(attrName); 
}

template<class E>
bool EntityApp<E>::installPyScript()
{
	if(Resmgr::getSingleton().respaths().size() <= 0 || 
		Resmgr::getSingleton().getPyUserResPath().size() == 0 || 
		Resmgr::getSingleton().getPySysResPath().size() == 0)
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH is error!\n");
		return false;
	}

	std::wstring user_res_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().getPyUserResPath().c_str()));
	if(tbuf != NULL)
	{
		user_res_path += tbuf;
		free(tbuf);
	}
	else
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH error[char2wchar]!\n");
		return false;
	}

	std::wstring pyPaths = user_res_path + L"scripts/common;";
	pyPaths += user_res_path + L"scripts/data;";
	pyPaths += user_res_path + L"scripts/user_type;";

	switch(componentType_)
	{
	case BASEAPP_TYPE:
		pyPaths += user_res_path + L"scripts/server_common;";
		pyPaths += user_res_path + L"scripts/base;";
		break;
	case CELLAPP_TYPE:
		pyPaths += user_res_path + L"scripts/server_common;";
		pyPaths += user_res_path + L"scripts/cell;";
		break;
	case DBMGR_TYPE:
		pyPaths += user_res_path + L"scripts/server_common;";
		pyPaths += user_res_path + L"scripts/db;";
		break;
	default:
		pyPaths += user_res_path + L"scripts/client;";
		break;
	};
	
	std::string kbe_res_path = Resmgr::getSingleton().getPySysResPath();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = getScript().install(tbuf, pyPaths, "KBEngine", componentType_);
	free(tbuf);
	return ret;
}

template<class E>
void EntityApp<E>::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

template<class E>
bool EntityApp<E>::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

template<class E>
bool EntityApp<E>::installPyModules()
{
	Entities<E>::installScript(NULL);
	//Entity::installScript(g_script.getModule());

	pEntities_ = new Entities<E>();
	registerPyObjectToScript("entities", pEntities_);

	// ��װ���ģ��
	PyObject *entryScriptFileName = NULL;
	if(componentType() == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getBaseApp();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}
	else if(componentType() == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getCellApp();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}

	// ���pywatcher֧��
	if(!initializePyWatcher(&this->getScript()))
		return false;

	// ���globalData, globalBases֧��
	pGlobalData_ = new GlobalDataClient(DBMGR_TYPE, GlobalDataServer::GLOBAL_DATA);
	registerPyObjectToScript("globalData", pGlobalData_);

	// ע�ᴴ��entity�ķ�����py
	// ��ű�ע��app����״̬
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	publish,			__py_getAppPublish,		METH_VARARGS,	0);

	// ע�����ýű��������
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	scriptLogType,		__py_setScriptLogType,	METH_VARARGS,	0);
	
	// �����Դȫ·��
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	getResFullPath,		__py_getResFullPath,	METH_VARARGS,	0);

	// �ļ�����
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	open,				__py_kbeOpen,			METH_VARARGS,	0);

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_NORMAL", log4cxx::ScriptLevel::SCRIPT_INT))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_NORMAL.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_INFO", log4cxx::ScriptLevel::SCRIPT_INFO))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_INFO.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_ERR", log4cxx::ScriptLevel::SCRIPT_ERR))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_ERR.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_DBG", log4cxx::ScriptLevel::SCRIPT_DBG))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_DBG.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_WAR", log4cxx::ScriptLevel::SCRIPT_WAR))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_WAR.\n");
	}


	if(PyModule_AddIntConstant(this->getScript().getModule(), "NEXT_ONLY", KBE_NEXT_ONLY))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.NEXT_ONLY.\n");
	}
	
	if(entryScriptFileName != NULL)
	{
		entryScript_ = PyImport_Import(entryScriptFileName);
		SCRIPT_ERROR_CHECK();
		S_RELEASE(entryScriptFileName);

		if(entryScript_.get() == NULL)
		{
			return false;
		}
	}

	onInstallPyModules();
	return true;
}

template<class E>
bool EntityApp<E>::uninstallPyModules()
{
	S_RELEASE(pEntities_);
	unregisterPyObjectToScript("entities");

	unregisterPyObjectToScript("globalData");
	S_RELEASE(pGlobalData_); 

	Entities<E>::uninstallScript();
	//Entity::uninstallScript();
	EntityDef::uninstallScript();
	return true;
}

template<class E>
E* EntityApp<E>::createEntityCommon(const char* entityType, PyObject* params,
										 bool isInitializeScript, ENTITY_ID eid, bool initProperty)
{
	// ���ID�Ƿ��㹻, ���㷵��NULL
	if(eid <= 0 && idClient_.getSize() == 0)
	{
		PyErr_SetString(PyExc_SystemError, "EntityApp::createEntityCommon: is Failed. not enough entityIDs.");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "EntityApp::createEntityCommon: entity [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}
	else if(componentType_ == CELLAPP_TYPE ? !sm->hasCell() : !sm->hasBase())
	{
		PyErr_Format(PyExc_TypeError, "EntityApp::createEntityCommon: entity [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();

	// �ж��Ƿ�Ҫ����һ���µ�id
	ENTITY_ID id = eid;
	if(id <= 0)
		id = idClient_.alloc();
	
	E* entity = onCreateEntityCommon(obj, sm, id);

	if(initProperty)
		entity->initProperty();

	// ��entity����entities
	pEntities_->add(id, entity); 

	// ��ʼ���ű�
	if(isInitializeScript)
		entity->initializeEntity(params);

	SCRIPT_ERROR_CHECK();

	if(g_debugEntity)
	{
		INFO_MSG(boost::format("EntityApp::createEntityCommon: new %1% (%2%) refc=%3%.\n") % entityType % id % obj->ob_refcnt);
	}
	else
	{
		INFO_MSG(boost::format("EntityApp::createEntityCommon: new %1% (%2%)\n") % entityType % id);
	}

	return entity;
}

template<class E>
E* EntityApp<E>::onCreateEntityCommon(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid)
{
	// ִ��Entity�Ĺ��캯��
	return new(pyEntity) E(eid, sm);
}

template<class E>
void EntityApp<E>::onSignalled(int sigNum)
{
	this->ServerApp::onSignalled(sigNum);
	
	switch (sigNum)
	{
	case SIGQUIT:
		CRITICAL_MSG(boost::format("Received QUIT signal. This is likely caused by the "
					"%1%Mgr killing this %2% because it has been "
					"unresponsive for too long. Look at the callstack from "
					"the core dump to find the likely cause.\n") %
				COMPONENT_NAME_EX(componentType_) % 
				COMPONENT_NAME_EX(componentType_) );
		
		break;
	default: 
		break;
	}
}

template<class E>
PyObject* EntityApp<E>::tryGetEntityByMailbox(COMPONENT_ID componentID, ENTITY_ID eid)
{
	if(componentID != componentID_)
		return NULL;
	
	E* entity = pEntities_->find(eid);
	if(entity == NULL){
		ERROR_MSG(boost::format("EntityApp::tryGetEntityByMailbox: can't found entity:%1%.\n") % eid);
		return NULL;
	}

	return entity;
}

template<class E>
Mercury::Channel* EntityApp<E>::findChannelByMailbox(EntityMailbox& mailbox)
{
	// ������ID����0��������
	if(mailbox.getComponentID() > 0)
	{
		Components::ComponentInfos* cinfos = 
			Components::getSingleton().findComponent(mailbox.getComponentID());

		if(cinfos != NULL && cinfos->pChannel != NULL)
			return cinfos->pChannel; 
	}
	else
	{
		return Components::getSingleton().pNetworkInterface()->findChannel(mailbox.addr());
	}

	return NULL;
}

template<class E>
void EntityApp<E>::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

template<class E>
void EntityApp<E>::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("EntityApp::handleGameTick[%"PRTime"]:%u\n", t, time_);

	g_kbetime++;
	threadPool_.onMainThreadTick();
	handleTimers();
	getNetworkInterface().processAllChannelPackets(KBEngine::Mercury::MessageHandlers::pMainMessageHandlers);
}

template<class E>
bool EntityApp<E>::destroyEntity(ENTITY_ID entityID, bool callScript)
{
	PyObjectPtr entity = pEntities_->erase(entityID);
	if(entity != NULL)
	{
		static_cast<E*>(entity.get())->destroy(callScript);
		return true;
	}

	ERROR_MSG(boost::format("EntityApp::destroyEntity: not found %1%!\n") % entityID);
	return false;
}

template<class E>
E* EntityApp<E>::findEntity(ENTITY_ID entityID)
{
	AUTO_SCOPED_PROFILE("findEntity");
	return pEntities_->find(entityID);
}

template<class E>
void EntityApp<E>::onReqAllocEntityID(Mercury::Channel* pChannel, ENTITY_ID startID, ENTITY_ID endID)
{
	// INFO_MSG("EntityApp::onReqAllocEntityID: entityID alloc(%d-%d).\n", startID, endID);
	idClient_.onAddRange(startID, endID);
}

template<class E>
PyObject* EntityApp<E>::__py_getAppPublish(PyObject* self, PyObject* args)
{
	return PyLong_FromLong(g_appPublish);
}

template<class E>
PyObject* EntityApp<E>::__py_setScriptLogType(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	int type = -1;

	if(PyArg_ParseTuple(args, "i", &type) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args is error!");
		PyErr_PrintEx(0);
	}

	DebugHelper::getSingleton().setScriptMsgType(type);
	S_Return;
}

template<class E>
PyObject* EntityApp<E>::__py_getResFullPath(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getResFullPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;

	if(PyArg_ParseTuple(args, "s", &respath) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getResFullPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::string fullpath = Resmgr::getSingleton().matchRes(respath);
	return PyUnicode_FromString(fullpath.c_str());
}

template<class E>
PyObject* EntityApp<E>::__py_kbeOpen(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::open(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;
	char* fargs = NULL;

	if(PyArg_ParseTuple(args, "s|s", &respath, &fargs) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::open(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::string sfullpath = Resmgr::getSingleton().matchRes(respath);

	PyObject *ioMod = PyImport_ImportModule("io");
	PyObject *openedFile = PyObject_CallMethod(ioMod, const_cast<char*>("open"), const_cast<char*>("ss"), const_cast<char*>(sfullpath.c_str()), fargs);
	Py_DECREF(ioMod);
	return openedFile;
}

template<class E>
void EntityApp<E>::startProfile(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string profileName;
	int8 profileType;
	uint32 timelen;

	s >> profileName >> profileType >> timelen;

	switch(profileType)
	{
	case 0:	// pyprofile
		new PyProfileHandler(this->getNetworkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 1:	// cprofile
		new CProfileHandler(this->getNetworkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 2:	// eventprofile
		new EventProfileHandler(this->getNetworkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 3:	// mercuryprofile
		new MercuryProfileHandler(this->getNetworkInterface(), timelen, profileName, pChannel->addr());
		break;
	default:
		ERROR_MSG(boost::format("EntityApp::startProfile: type(%1%:%2%) not support!\n") % 
			profileType % profileName);
		break;
	};
}

template<class E>
void EntityApp<E>::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
						GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest)
{
	INFO_MSG(boost::format("EntityApp::onDbmgrInitCompleted: entityID alloc(%1%-%2%), startGlobalOrder=%3%, startGroupOrder=%4%, digest=%5%.\n") %
		startID % endID % startGlobalOrder % startGroupOrder % digest);

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;
	g_componentGlobalOrder = startGlobalOrder;
	g_componentGroupOrder = startGroupOrder;

	idClient_.onAddRange(startID, endID);
	g_kbetime = gametime;

	if(digest != EntityDef::md5().getDigestStr())
	{
		ERROR_MSG(boost::format("EntityApp::onDbmgrInitCompleted: digest not match. curr(%1%) != dbmgr(%2%)\n") %
			EntityDef::md5().getDigestStr() % digest);

		this->shutDown();
	}
}

template<class E>
void EntityApp<E>::onBroadcastGlobalDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string key, value;
	bool isDelete;
	
	s >> isDelete;
	s.readBlob(key);

	if(!isDelete)
	{
		s.readBlob(value);
	}

	PyObject * pyKey = script::Pickler::unpickle(key);
	if(pyKey == NULL)
	{
		ERROR_MSG("EntityApp::onBroadcastGlobalDataChanged: no has key!\n");
		return;
	}

	Py_INCREF(pyKey);

	if(isDelete)
	{
		if(pGlobalData_->del(pyKey))
		{
			// ֪ͨ�ű�
			SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onGlobalDataDel"), 
				const_cast<char*>("O"), pyKey);
		}
	}
	else
	{
		PyObject * pyValue = script::Pickler::unpickle(value);
		if(pyValue == NULL)
		{
			ERROR_MSG("EntityApp::onBroadcastGlobalDataChanged: no has value!\n");
			Py_DECREF(pyKey);
			return;
		}

		Py_INCREF(pyValue);

		if(pGlobalData_->write(pyKey, pyValue))
		{
			// ֪ͨ�ű�
			SCRIPT_OBJECT_CALL_ARGS2(getEntryScript().get(), const_cast<char*>("onGlobalData"), 
				const_cast<char*>("OO"), pyKey, pyValue);
		}

		Py_DECREF(pyValue);
	}

	Py_DECREF(pyKey);
}

template<class E>
void EntityApp<E>::onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string cmd;
	s.readBlob(cmd);

	PyObject* pycmd = PyUnicode_DecodeUTF8(cmd.data(), cmd.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(boost::format("EntityApp::onExecScriptCommand: size(%1%), command=%2%.\n") % 
		cmd.size() % cmd);

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);
	script_.run_simpleString(PyBytes_AsString(pycmd1), &retbuf);

	if(retbuf.size() == 0)
	{
		retbuf = "\r\n";
	}

	// ��������ظ��ͻ���
	Mercury::Bundle bundle;
	ConsoleInterface::ConsoleExecCommandCBMessageHandler msgHandler;
	bundle.newMessage(msgHandler);
	ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1::staticAddToBundle(bundle, retbuf);
	bundle.send(this->getNetworkInterface(), pChannel);

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

template<class E>
void EntityApp<E>::onReloadScript(bool fullReload)
{
	EntityMailbox::MAILBOXS::iterator iter =  EntityMailbox::mailboxs.begin();
	for(; iter != EntityMailbox::mailboxs.end(); iter++)
	{
		(*iter)->reload();
	}
}

template<class E>
void EntityApp<E>::reloadScript(bool fullReload)
{
	EntityDef::reload(fullReload);
	onReloadScript(fullReload);

	// ���нű����������
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										1);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

}
#endif
