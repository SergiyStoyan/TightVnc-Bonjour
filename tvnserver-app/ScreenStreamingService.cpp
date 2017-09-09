//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "ScreenStreamingService.h"
#include <tchar.h>

void ScreenStreamingService::ScreenStreamingServiceConfigReloadListener::onConfigReload(ServerConfig *serverConfig)
{
	ScreenStreamingService::serverConfig = serverConfig;
}

void ScreenStreamingService::ScreenStreamingServiceConfigReloadListener::onTvnServerShutdown()
{
	ScreenStreamingService::StopAll();
}

ScreenStreamingService::ScreenStreamingServiceConfigReloadListener ScreenStreamingService::screenStreamingServiceConfigReloadListener = ScreenStreamingServiceConfigReloadListener();
bool ScreenStreamingService::initialized = false;
LogWriter* ScreenStreamingService::log;
ScreenStreamingService::ScreenStreamingServiceList ScreenStreamingService::screenStreamingServiceList = ScreenStreamingServiceList();
LocalMutex ScreenStreamingService::lock;
ServerConfig* ScreenStreamingService::serverConfig;

void ScreenStreamingService::Initialize(LogWriter *log, TvnServer *tvnServer, Configurator *configurator)
{
	AutoLock l(&lock);

	ScreenStreamingService::log = log;
	if (initialized)
	{
		ScreenStreamingService::log->interror(_T("ScreenStreamingService: Is already initialized"));
		return;
	}

	tvnServer->addListener(&screenStreamingServiceConfigReloadListener);
	configurator->addListener(&screenStreamingServiceConfigReloadListener);
	screenStreamingServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());

	initialized = true;
}

ScreenStreamingService* ScreenStreamingService::get(ULONG ip)
{
	//ULONG ip = SocketAddressIPv4::resolve(host, 0).getSockAddr().sin_addr.S_un.S_addr;
	//ULONG ip = address.getSockAddr().sin_addr.S_un.S_addr;
	//USHORT port = address.getSockAddr().sin_port;
	
	AutoLock l(&lock);
	for (ScreenStreamingServiceList::iterator i = screenStreamingServiceList.begin(); i != screenStreamingServiceList.end(); i++)
	{
		ScreenStreamingService* sss = (*i);
		if (sss->address.getSockAddr().sin_addr.S_un.S_addr != ip)
			continue;
		//if (sss->address.getSockAddr().sin_port != port)
		//	continue;
		return sss;
	}
	return NULL;
}

ScreenStreamingService::ScreenStreamingService(ULONG ip, USHORT port)
{ 
	this->address = SocketAddressIPv4::resolve(ip, port);
}

void ScreenStreamingService::Start(ULONG ip)
{
	if (!initialized)
	{
		log->interror(_T("ScreenStreamingService: Is not initialized"));
		return;
	}

	//ServerConfig *sc = Configurator::getInstance()->getServerConfig();
	if (!ScreenStreamingService::serverConfig->isScreenStreamingEnabled())
		return;

	if (ScreenStreamingService::serverConfig->getScreenStreamingDelayMss() > 0)
		Sleep(ScreenStreamingService::serverConfig->getScreenStreamingDelayMss());

	AutoLock l(&lock);

	ScreenStreamingService* sss;
	for (sss = ScreenStreamingService::get(ip); sss; sss = ScreenStreamingService::get(ip))
		sss->destroy();

	sss = new ScreenStreamingService(ip, ScreenStreamingService::serverConfig->getScreenStreamingDestinationPort());
	try
	{
		//sss->process = new Process(_T("ffmpeg.exe"), _T("-f gdigrab -framerate %d -i desktop -f mpegts udp://%s", ScreenStreamingService::serverConfig->getScreenStreamingFramerate(), ss.getString()));
		//sss->process->start();	

		STARTUPINFO si;
		::ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		::ZeroMemory(&sss->processInformation, sizeof(sss->processInformation));
		StringStorage ss;
		sss->address.toString2(&ss);
		sss->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i desktop -f mpegts udp://%s"), ScreenStreamingService::serverConfig->getScreenStreamingFramerate(), ss.getString());
		//sss->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i desktop -f mpegts udp://%s 2>_1.txt"), ScreenStreamingService::serverConfig->getScreenStreamingFramerate(), ss.getString());
		if (!CreateProcess(NULL, (LPTSTR)sss->commandLine.getString(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &sss->processInformation))
			throw SystemException();
	}
	catch (SystemException &e)
	{
		log->error(_T("ScreenStreamingService: Could not CreateProcess. Command line:\r\n%s\r\nError: %s"), sss->commandLine.getString(), e.getMessage());
		//log->error(_T("ScreenStreamingService: Could not CreateProcess. Command line:\r\n%s %s\r\nError: %s"), sss->process->getFilename(), sss->process->getArguments(), e.getMessage());
		return;
	}

	if (ScreenStreamingService::get(sss->address.getSockAddr().sin_addr.S_un.S_addr))
	{
		StringStorage ss;
		sss->address.toString(&ss);
		ss.format(_T("ScreenStreamingService: while adding a stream: a stream to this destination aready exists: %s"), ss.getString());
		throw Exception(ss.getString());
	}
	screenStreamingServiceList.push_back(sss);

	StringStorage ss;
	sss->address.toString2(&ss);
	log->message(_T("ScreenStreamingService: Started for: %s"), ss.getString());
}

void ScreenStreamingService::Stop(ULONG ip)
{
	ScreenStreamingService* sss = ScreenStreamingService::get(ip);
	if (!sss)
	{
		SocketAddressIPv4 s = SocketAddressIPv4::resolve(ip, 0);
		StringStorage ss;
		s.toString(&ss);
		log->interror(_T("ScreenStreamingService: No service exists for address: %s"), ss.getString());
		return;
	}
	sss->destroy();
}

void ScreenStreamingService::destroy()
{
	AutoLock l(&lock);

	DWORD ec;
	if (GetExitCodeProcess(processInformation.hProcess, &ec) && (ec == STILL_ACTIVE))
	{
		try
		{
			//process->kill();
			if (!TerminateProcess(processInformation.hProcess, 0))
				throw SystemException();
		}
		catch (SystemException &e)
		{
			//log->error(_T("ScreenStreamingService: Could not terminate process:\r\n%s %s\r\nError: %s"), process->getFilename(), process->getArguments(), e.getMessage());
			log->error(_T("ScreenStreamingService: Could not terminate process for %s\r\nError: %s"), commandLine.getString(), e.getMessage());
			//!!!return;//!!! it is expected to be removed from the list. Otherwise it may duplicate in the list.
		}
	}
	DWORD state = WaitForSingleObject(processInformation.hProcess, 1000);
	if (state != WAIT_OBJECT_0)
		log->error(_T("ScreenStreamingService: ZOMBIE PROCESSES RUNNING: %s"), commandLine.getString());
	/*DWORD ec;
	if (!GetExitCodeProcess(processInformation.hProcess, &ec))
		log->error(_T("ScreenStreamingService: GetExitCodeProcess: %d"), GetLastError());
	else if (ec == STILL_ACTIVE) 
			log->error(_T("ScreenStreamingService: ZOMBIE PROCESSES RUNNING: %s"), commandLine.getString());*/

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
	screenStreamingServiceList.remove(this);

	StringStorage ss;
	address.toString2(&ss);
	log->message(_T("ScreenStreamingService: Stopped for address: %s"), ss.getString());

	delete(this);//!!!ATTENTION: can be applied only to objects created by 'new'!!!
	//!!!this object must be forgotten after this line!!!
}

void ScreenStreamingService::StopAll()
{
	AutoLock l(&lock);
	for (ScreenStreamingServiceList::iterator i = screenStreamingServiceList.begin(); i != screenStreamingServiceList.end(); i++)
	{
		ScreenStreamingService* sss = (*i);
		sss->destroy();
	}
	log->message(_T("ScreenStreamingService: Stopped all."));
}

ULONG ScreenStreamingService::GetIp(SocketIPv4* s)
{
	SocketAddressIPv4 sa;
	s->getPeerAddr(&sa);
	return sa.getSockAddr().sin_addr.S_un.S_addr;
}