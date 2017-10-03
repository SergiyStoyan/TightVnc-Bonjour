//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#ifndef _MPEG_STREAMER_H_
#define _MPEG_STREAMER_H_

#include "server-config-lib/Configurator.h"
#include "TvnServer.h"
#include "win-system/Process.h"
#include <list>

/*
This service creates a video stream to the client INDEPENDENTLY on RFB connection.
*/

class MpegStreamer
{
	class MpegStreamerConfigReloadListener : public ConfigReloadListener, public TvnServerListener
	{
	public:
		void onConfigReload(ServerConfig* serverConfig);
		void onTvnServerShutdown();
	};

	~MpegStreamer();

public:
	static void Initialize(LogWriter* log, TvnServer* tvnServer);

	static void Start(ULONG ip, USHORT port, BYTE aesKeySalt[30] = NULL);
	static void Stop(ULONG ip);
	static void StopAll();

	static ULONG GetIp(SocketIPv4* s);

private:
	MpegStreamer(ULONG ip, USHORT port);

	StringStorage commandLine;
	PROCESS_INFORMATION processInformation;
	//Process* process;
	SocketAddressIPv4 address; 

	BOOL redirect_process_output2log(STARTUPINFO* si);
	HANDLE childProcessStdErrRead;
	HANDLE childProcessStdErrWrite;
	HANDLE readChildProcessOutputThread;
	static DWORD WINAPI readChildProcessOutput(void* Param);

	static HANDLE anti_zombie_job;
	static MpegStreamer* get(ULONG ip);
	static LocalMutex lock;
	typedef list<MpegStreamer *> MpegStreamerList;
	static MpegStreamerList mpegStreamerList;
	static MpegStreamerConfigReloadListener mpegStreamerConfigReloadListener;
	static bool initialized;
	static LogWriter* log;
	//static ServerConfig* serverConfig;
	static BOOL get_display_virtual_area(const StringStorage display_name, LONG* x, LONG* y, LONG* width, LONG* height);
	static MONITORINFOEX display_info;
	static WCHAR display_device_name[CCHDEVICENAME];
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
};

#endif
