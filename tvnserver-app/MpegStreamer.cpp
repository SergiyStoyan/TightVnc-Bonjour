//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "MpegStreamer.h"
#include <tchar.h>

void MpegStreamer::MpegStreamerConfigReloadListener::onConfigReload(ServerConfig *serverConfig)
{
	//MpegStreamer::serverConfig = serverConfig;
}

void MpegStreamer::MpegStreamerConfigReloadListener::onTvnServerShutdown()
{
	MpegStreamer::StopAll();
}

MpegStreamer::MpegStreamerConfigReloadListener MpegStreamer::mpegStreamerConfigReloadListener = MpegStreamerConfigReloadListener();
bool MpegStreamer::initialized = false;
LogWriter* MpegStreamer::log;
MpegStreamer::MpegStreamerList MpegStreamer::mpegStreamerList = MpegStreamerList();
LocalMutex MpegStreamer::lock;
//ServerConfig* MpegStreamer::serverConfig;
HANDLE MpegStreamer::anti_zombie_job;

void MpegStreamer::Initialize(LogWriter *log, TvnServer *tvnServer, Configurator *configurator)
{
	AutoLock l(&lock);

	MpegStreamer::log = log;
	if (initialized)
	{
		MpegStreamer::log->interror(_T("MpegStreamer: Is already initialized"));
		return;
	}

	tvnServer->addListener(&mpegStreamerConfigReloadListener);
	configurator->addListener(&mpegStreamerConfigReloadListener);
	mpegStreamerConfigReloadListener.onConfigReload(configurator->getServerConfig());

	//this is an anti-zombie mechanism that must kill the child processes even if the app crashed
	anti_zombie_job = CreateJobObject(NULL, NULL); // GLOBAL
	if (!anti_zombie_job)
	{
		MpegStreamer::log->interror(_T("MpegStreamer: CreateJobObject failed"));
		return;
	}
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (!SetInformationJobObject(anti_zombie_job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
	{
		MpegStreamer::log->interror(_T("MpegStreamer: SetInformationJobObject failed"));
		return;
	}

	initialized = true;
}

MpegStreamer* MpegStreamer::get(ULONG ip)
{
	//ULONG ip = SocketAddressIPv4::resolve(host, 0).getSockAddr().sin_addr.S_un.S_addr;
	//ULONG ip = address.getSockAddr().sin_addr.S_un.S_addr;
	//USHORT port = address.getSockAddr().sin_port;
	
	AutoLock l(&lock);
	for (MpegStreamerList::iterator i = mpegStreamerList.begin(); i != mpegStreamerList.end(); i++)
	{
		MpegStreamer* ms = (*i);
		if (ms->address.getSockAddr().sin_addr.S_un.S_addr != ip)
			continue;
		//if (sss->address.getSockAddr().sin_port != port)
		//	continue;
		return ms;
	}
	return NULL;
}

MpegStreamer::MpegStreamer(ULONG ip, USHORT port)
{ 
	this->address = SocketAddressIPv4::resolve(ip, port);
}

void MpegStreamer::Start(ULONG ip)
{
	if (!initialized)
	{
		log->interror(_T("MpegStreamer: Is not initialized"));
		return;
	}
	
	ServerConfig *config = Configurator::getInstance()->getServerConfig();
	if (!config->isMpegStreamerEnabled())
		return;

	if (config->getMpegStreamerDelayMss() > 0)
		Sleep(config->getMpegStreamerDelayMss());

	AutoLock l(&lock);

	MpegStreamer* ms;
	for (ms = MpegStreamer::get(ip); ms; ms = MpegStreamer::get(ip))
		ms->destroy();

	ms = new MpegStreamer(ip, config->getMpegStreamerDestinationPort());
	try
	{
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&ms->processInformation, sizeof(ms->processInformation));
		StringStorage ss;
		//ms->address.toString2(&ss);
		//if(1)//window title
		//	ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i title=\"%s\" -f mpegts udp://%s"), config->getMpegStreamerFramerate(), config->getMpegStreamerCapturedWindowTitle(), ss.getString());
		//else if(2)//region in the virtual desktop
		//	ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -offset_x %d -offset_y %d -video_size %dx%d -show_region 1 -i desktop -f mpegts udp://%s"), config->getMpegStreamerFramerate(), offset_x, offset_y, video_size_x, video_size_y, ss.getString());
		//
		ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i desktop  -f mpegts udp://%s"), config->getMpegStreamerFramerate(), ss.getString());
		DWORD dwCreationFlags = 0;
		if(config->isMpegStreamerWindowHidden())
			dwCreationFlags = dwCreationFlags | CREATE_NO_WINDOW;
		if (!CreateProcess(NULL, (LPTSTR)ms->commandLine.getString(), NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &si, &ms->processInformation))
			throw SystemException();
	}
	catch (SystemException &e)
	{
		log->interror(_T("MpegStreamer: Could not CreateProcess. Command line:\r\n%s\r\nError: %s"), ms->commandLine.getString(), e.getMessage());
		return;
	}
	if (!AssignProcessToJobObject(anti_zombie_job, ms->processInformation.hProcess))
	{
		log->interror(_T("MpegStreamer: AssignProcessToJobObject failed. Error: %d"), GetLastError());
		return;
	}

	if (MpegStreamer::get(ms->address.getSockAddr().sin_addr.S_un.S_addr))
	{
		StringStorage ss;
		ms->address.toString(&ss);
		log->interror(_T("MpegStreamer: while adding a stream: a stream to this destination aready exists: %s"), ss.getString());
		throw Exception(ss.getString());
	}
	mpegStreamerList.push_back(ms);

	StringStorage ss;
	ms->address.toString2(&ss);
	log->message(_T("MpegStreamer: Started for: %s"), ss.getString());
}

void MpegStreamer::Stop(ULONG ip)
{
	MpegStreamer* sss = MpegStreamer::get(ip);
	if (!sss)
	{
		SocketAddressIPv4 s = SocketAddressIPv4::resolve(ip, 0);
		StringStorage ss;
		s.toString(&ss);
		log->interror(_T("MpegStreamer: No service exists for address: %s"), ss.getString());
		return;
	}
	sss->destroy();
}

void MpegStreamer::destroy()
{
	AutoLock l(&lock);

	DWORD ec;
	if (GetExitCodeProcess(processInformation.hProcess, &ec) && ec == STILL_ACTIVE)
	{
		try
		{
			if (!TerminateProcess(processInformation.hProcess, 0))
				throw SystemException();
		}
		catch (SystemException &e)
		{
			log->error(_T("MpegStreamer: Could not terminate process for %s\r\nError: %s"), commandLine.getString(), e.getMessage());
			//!!!return;//!!! it must be removed from the list. Otherwise it may duplicate in the list.
		}
	}
	DWORD state = WaitForSingleObject(processInformation.hProcess, 1000);
	if (state != WAIT_OBJECT_0)
		log->interror(_T("MpegStreamer: !!!ZOMBIE PROCESS RUNNING!!!: %s"), commandLine.getString());

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
	mpegStreamerList.remove(this);

	StringStorage ss;
	address.toString2(&ss);
	log->message(_T("MpegStreamer: Stopped for address: %s"), ss.getString());

	delete(this);//!!!ATTENTION: can be applied only to objects created by 'new'!!!
	//!!!this object must be forgotten after this line!!!
}

void MpegStreamer::StopAll()
{
	AutoLock l(&lock);
	MpegStreamerList list = mpegStreamerList;//copy to another list to iterate while removing objects from the base one
	for (MpegStreamerList::iterator i = list.begin(); i != list.end(); i++)
	{
		MpegStreamer* sss = (*i);
		sss->destroy();
	}
	log->message(_T("MpegStreamer: Stopped all."));
}

ULONG MpegStreamer::GetIp(SocketIPv4* s)
{
	SocketAddressIPv4 sa;
	s->getPeerAddr(&sa);
	return sa.getSockAddr().sin_addr.S_un.S_addr;
}