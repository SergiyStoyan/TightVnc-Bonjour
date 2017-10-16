//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "util/base64.h"
#include "util/AnsiStringStorage.h"

#include "MpegStreamer.h"
#include <tchar.h>
#include <iostream>
#include <ctime>

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
MpegStreamer::MpegStreamerList MpegStreamer::mpegStreamerList;
LocalMutex MpegStreamer::lock;
//ServerConfig* MpegStreamer::serverConfig;
HANDLE MpegStreamer::anti_zombie_job;

void MpegStreamer::Initialize(LogWriter *log, TvnServer *tvnServer)
{
	AutoLock l(&lock);

	MpegStreamer::log = log;
	if (initialized)
	{
		MpegStreamer::log->interror(_T("MpegStreamer: Is already initialized"));
		return;
	}

	tvnServer->addListener(&mpegStreamerConfigReloadListener);
	Configurator *configurator = Configurator::getInstance();
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

void MpegStreamer::Start(ULONG ip, USHORT port, BYTE framerate, BYTE* aesKeySalt)
{
	if (!initialized)
	{
		log->interror(_T("MpegStreamer: Is not initialized"));
		return;
	}
	
	ServerConfig *config = Configurator::getInstance()->getServerConfig();

	/*if (config->getMpegStreamerDelayMss() > 0)
		Sleep(config->getMpegStreamerDelayMss());*/

	AutoLock l(&lock);

	MpegStreamer* ms;
	for (ms = MpegStreamer::get(ip); ms; ms = MpegStreamer::get(ip))
		delete(ms);

	ms = new MpegStreamer(ip, port);
	StringStorage ip_ss; 
	ms->address.toString2(&ip_ss);
	StringStorage command_line1;
	command_line1.format(_T("ffmpeg.exe -f gdigrab -framerate %d"), framerate);
	StringStorage command_line2;
	if (aesKeySalt == NULL)
		command_line2.format(_T("-f mpegts udp://%s"), ip_ss.getString());
	else
	{
		base64 b;
		size_t aes_key_salt_l;
		char* aes_key_salt_ = b.encode(aesKeySalt, CisteraHandshake::serverResponse_mpegStreamAesKeySalt_SIZE, &aes_key_salt_l);
		char mpegStreamAesKeySalt[41];
		memcpy(mpegStreamAesKeySalt, aes_key_salt_, sizeof(mpegStreamAesKeySalt) - 1);
		mpegStreamAesKeySalt[sizeof(mpegStreamAesKeySalt) - 1] = '\0';
		AnsiStringStorage ass(mpegStreamAesKeySalt);
		StringStorage aes_key_salt_ss;
		ass.toStringStorage(&aes_key_salt_ss);
		command_line2.format(_T("-f rtp_mpegts -srtp_out_suite AES_CM_128_HMAC_SHA1_80 -srtp_out_params %s srtp://%s"), aes_key_salt_ss.getString(), ip_ss.getString());
	}
	try
	{
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
				ms->commandLine.format(_T("%s -i desktop %s"), command_line1.getString(), command_line2.getString());
			}
			else
				ms->commandLine.format(_T("%s -offset_x %d -offset_y %d -video_size %dx%d -show_region 1 -i desktop %s"), command_line1.getString(), x, y, w, h, command_line2.getString());
			break;
		}
		case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_AREA:
		{
			LONG x, y, w, h;
			config->getMpegStreamerCapturedArea(&x, &y, &w, &h);
			ms->commandLine.format(_T("%s -offset_x %d -offset_y %d -video_size %dx%d -show_region 1 -i desktop %s"), command_line1.getString(), x, y, w, h, command_line2.getString());
			break;
		}
		case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_WINDOW:
		{
			StringStorage wt;
			config->getMpegStreamerCapturedWindowTitle(&wt);
			ms->commandLine.format(_T("%s -i title=\"%s\" %s"), command_line1.getString(), wt.getString(), command_line2.getString());
			break;
		}
		default:
			throw new Exception(_T("Unexpected option"));
		}
		//ms->commandLine.format(_T("ffmpeg.exe -f gdigrab -framerate %d -i desktop1 -f mpegts udp://%s"), config->getMpegStreamerFramerate(), ss.getString());//TEST
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		//if (config->isMpegStreamerWindowHidden())
			//ms->redirect_process_output2log(&si);//!!!BUG (windows7?)!!! - stdout could not be separated from strerr
		ZeroMemory(&ms->processInformation, sizeof(ms->processInformation));
		if (config->logMpegStreamerProcessOutput())
		{
			StringStorage log_dir;
			config->getLogFileDir(&log_dir);
			WIN32_FIND_DATA fd;
			StringStorage log_pattern;
			log_pattern.format(_T("%s\\ffmpeg*.log"), log_dir.getString());
			HANDLE hf = FindFirstFile(log_pattern.getString(), &fd);
			if (hf != INVALID_HANDLE_VALUE)
			{
				UINT32 delete_older_than_secs = 60 * 60 * 24 * 3;
				FILETIME delete_older_than_this;
				GetSystemTimeAsFileTime(&delete_older_than_this);
				((ULARGE_INTEGER *)&delete_older_than_this)->QuadPart -= (delete_older_than_secs * 10000000LLU);
				do
				{
					if (CompareFileTime(&delete_older_than_this, &fd.ftLastWriteTime) > 0)
					{
						StringStorage log;
						log.format(_T("%s\\%s"), log_dir.getString(), fd.cFileName);
						DeleteFile(log.getString());
					}
				} while (FindNextFile(hf, &fd));
				FindClose(hf);
			}
			StringStorage log_file;
			SYSTEMTIME st;
			GetSystemTime(&st);
			log_file.format(_T("%s\\ffmpeg_%d-%d-%d-%d-%d-%d.log"), log_dir.getString(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(sa);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = TRUE;
			HANDLE h = CreateFile(log_file.getString(), GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(h == INVALID_HANDLE_VALUE)
				log->error(_T("MpegStreamer: Could not create child process log:\r\n%s\r\nCreateFile: Error: %d"), log_file.getString(), GetLastError());
			else
			{
				StringStorage ss;
				log->message(_T("MpegStreamer: FFMPEG log:\r\n%s"), log_file.getString());
				ss.format(_T("COMMAND LINE:\r\n%s\r\n\r\n"), ms->commandLine.getString(), time);
#ifdef UNICODE
				//TCHAR == WCHAR
				char buffer[2000];
				wcstombs(buffer, ss.getString(), ss.getSize() > sizeof(buffer) ? ss.getSize() : sizeof(buffer));
#else
				//TCHAR == char	
				TO BE IMPLEMENTED
#endif
				DWORD dwBytesWritten = 0;
				WriteFile(h, buffer, strlen(buffer), &dwBytesWritten, NULL);
				SYSTEMTIME t;
				GetLocalTime(&t);
				sprintf(buffer, "STARTED:\r\n%d-%d-%d %d:%d:%d\r\n\r\n", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
				WriteFile(h, buffer, strlen(buffer), &dwBytesWritten, NULL);

				si.hStdError = h;
				si.hStdOutput = h;
				si.dwFlags |= STARTF_USESTDHANDLES;
			}
		}
		log->message(_T("MpegStreamer: Launching:\r\n%s"), ms->commandLine.getString());
		DWORD dwCreationFlags = 0;
		if(config->hideMpegStreamerProcessWidnow())
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
			si->hStdOutput = INVALID_HANDLE_VALUE;// GetStdHandle(STD_OUTPUT_HANDLE);
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