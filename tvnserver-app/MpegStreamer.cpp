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
	AutoLock l(&lock);
	for (MpegStreamerList::iterator i = mpegStreamerList.begin(); i != mpegStreamerList.end(); i++)
	{
		MpegStreamer* ms = (*i);
		if (ms->address.getSockAddr().sin_addr.S_un.S_addr != ip)
			continue;
		return ms;
	}
	return NULL;
}

MpegStreamer::MpegStreamer(ULONG ip, USHORT port)
{ 
	address = SocketAddressIPv4::resolve(ip, port);
	readChildProcessOutputThread = NULL;
	childProcessStdErrRead = NULL;
	childProcessStdErrWrite = NULL;
}

MpegStreamer::~MpegStreamer()
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

	if (processInformation.hProcess)
	{
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
	}

	if (childProcessStdErrWrite)
		CloseHandle(childProcessStdErrWrite);
	if (childProcessStdErrRead)
		CloseHandle(childProcessStdErrRead);
	if (readChildProcessOutputThread)
		TerminateThread(readChildProcessOutputThread, 0);

	mpegStreamerList.remove(this);

	StringStorage ss;
	address.toString2(&ss);
	log->message(_T("MpegStreamer: Stopped for address: %s"), ss.getString());
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
		delete(ms);

	ms = new MpegStreamer(ip, config->getMpegStreamerDestinationPort());
	try
	{
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		//if (config->isMpegStreamerWindowHidden())//!!!BUG!!! - stdout could not be separated from strerr
		//	ms->redirect_process_output2log(&si);
		ZeroMemory(&ms->processInformation, sizeof(ms->processInformation));
		StringStorage ss;
		ms->address.toString2(&ss);
		switch (config->getMpegStreamerCaptureMode())
		{
		case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_DISPLAY:
		{
			//getting dimensions of the monitors to cut off it from the virtual desktop
			StringStorage dn;
			config->getMpegStreamerCapturedDisplayDeviceName(&dn);
			LONG x, y, w, h;
			if (!get_display_virtual_area(dn, &x, &y, &w, &h))
			{
				log->error(_T("MpegStreamer: Could not get desktop dimensions for '%s'. The entire virtual desktop will be streamed."), dn.getString());
				ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i desktop  -f mpegts udp://%s"), config->getMpegStreamerFramerate(), ss.getString());
			}
			else
				ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -offset_x %d -offset_y %d -video_size %dx%d -show_region 1 -i desktop -f mpegts udp://%s"), config->getMpegStreamerFramerate(), x, y, w, h, ss.getString());
			break;
		}
		case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_AREA:
		{
			LONG x, y, w, h;
			config->getMpegStreamerCapturedArea(&x, &y, &w, &h);
			ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -offset_x %d -offset_y %d -video_size %dx%d -show_region 1 -i desktop -f mpegts udp://%s"), config->getMpegStreamerFramerate(), x, y, w, h, ss.getString());
			break;
		}
		case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_WINDOW:
		{
			StringStorage wt;
			config->getMpegStreamerCapturedWindowTitle(&wt);
			ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i title=\"%s\" -f mpegts udp://%s"), config->getMpegStreamerFramerate(), wt.getString(), ss.getString());
			break;
		}
		default:
			throw new Exception(_T("Unexpected option"));
		}
		//ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i desktop1 -f mpegts udp://%s"), config->getMpegStreamerFramerate(), ss.getString());
		log->message(_T("MpegStreamer: Launching:\r\n%s"), ms->commandLine.getString());
		DWORD dwCreationFlags = 0;
		if(config->isMpegStreamerWindowHidden())
			dwCreationFlags = dwCreationFlags | CREATE_NO_WINDOW;
		if (!CreateProcess(NULL, (LPTSTR)ms->commandLine.getString(), NULL, NULL, TRUE, dwCreationFlags, NULL, NULL, &si, &ms->processInformation))
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

BOOL MpegStreamer::get_display_virtual_area(const StringStorage display_name, LONG* x, LONG* y, LONG* width, LONG* height)
{
	wcsncpy(display_device_name, display_name.getString(), sizeof(display_device_name) / sizeof(WCHAR));
	EnumDisplayMonitors(NULL, NULL, MpegStreamer::MonitorEnumProc, 0);
	if (!display_info.cbSize)
		return FALSE;
	*x = display_info.rcMonitor.left;
	*y = display_info.rcMonitor.top;
	*width = display_info.rcMonitor.right - display_info.rcMonitor.left;
	*height = display_info.rcMonitor.bottom - display_info.rcMonitor.top;
	return TRUE;
}
MONITORINFOEX MpegStreamer::display_info = MONITORINFOEX();
WCHAR MpegStreamer::display_device_name[CCHDEVICENAME];
BOOL CALLBACK MpegStreamer::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	display_info.cbSize = sizeof(MONITORINFOEX);
	if (!GetMonitorInfo(hMonitor, &display_info))
		return TRUE;// continue enumerating
	if (!wcscmp(display_info.szDevice, display_device_name))
		return FALSE;//stop enumerating
	display_info.cbSize = 0;
	return TRUE;// continue enumerating
}

BOOL MpegStreamer::redirect_process_output2log(STARTUPINFO* si)
{
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (CreatePipe(&childProcessStdErrRead, &childProcessStdErrWrite, &saAttr, 0))
	{
		if (SetHandleInformation(childProcessStdErrRead, HANDLE_FLAG_INHERIT, 0))
		{
			si->hStdError = childProcessStdErrWrite;
			/*HANDLE r, w;
			if(CreatePipe(&r, &w, &saAttr, 0) && SetHandleInformation(r, HANDLE_FLAG_INHERIT, 0))
				si->hStdOutput = w;*/
			si->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
			si->dwFlags |= STARTF_USESTDHANDLES;

			readChildProcessOutputThread = CreateThread(0, 0, readChildProcessOutput, this, 0, 0);
			if (readChildProcessOutputThread)
				return TRUE;
			else
				log->interror(_T("MpegStreamer: Could not create readErrorOutputFromChildProcessThread"));
		}
		else
			log->interror(_T("MpegStreamer: Could not SetHandleInformation. Error: %d"), GetLastError());
	}
	else
		log->interror(_T("MpegStreamer: Could not CreatePipe. Error: %d"), GetLastError());

	CloseHandle(childProcessStdErrRead);
	CloseHandle(childProcessStdErrWrite);
	si->dwFlags = 0;
	si->hStdError = NULL;
	si->hStdOutput = NULL;

	return FALSE;
}
DWORD WINAPI MpegStreamer::readChildProcessOutput(void* Param)
{
	MpegStreamer* ms = (MpegStreamer*)Param;
	DWORD dwRead;
	CHAR chBuf[2512]; 
	bool command_line_printed = false;
	while (ReadFile(ms->childProcessStdErrRead, chBuf, sizeof(chBuf) - 1, &dwRead, NULL) && dwRead)
	{
		chBuf[dwRead] = '\0';
		TCHAR m[sizeof(chBuf)];
#ifdef UNICODE
		//TCHAR == WCHAR
		mbstowcs(m, chBuf, sizeof(m));
#else
		//TCHAR == char	
		strcpy((char *)m, chBuf);
#endif
		if (!command_line_printed)
		{
			command_line_printed = true;
			log->error(_T("MpegStreamer: FFMPEG COMMAND LINE: %s"), ms->commandLine);
		}
		log->error(_T("MpegStreamer: FFMPEG ERROR OUTPUT: %s"), m); 
	}
	return 0;
}

void MpegStreamer::Stop(ULONG ip)
{
	MpegStreamer* ms = MpegStreamer::get(ip);
	if (!ms)
	{
		SocketAddressIPv4 s = SocketAddressIPv4::resolve(ip, 0);
		StringStorage ss;
		s.toString(&ss);
		log->interror(_T("MpegStreamer: No service exists for address: %s"), ss.getString());
		return;
	}
	delete(ms);
}

void MpegStreamer::StopAll()
{
	AutoLock l(&lock);
	MpegStreamerList list = mpegStreamerList;//copy to another list to iterate while removing objects from the base one
	for (MpegStreamerList::iterator i = list.begin(); i != list.end(); i++)
	{
		MpegStreamer* ms = (*i);
		delete(ms);
	}
	log->message(_T("MpegStreamer: Stopped all."));
}

ULONG MpegStreamer::GetIp(SocketIPv4* s)
{
	SocketAddressIPv4 sa;
	s->getPeerAddr(&sa);
	return sa.getSockAddr().sin_addr.S_un.S_addr;
}