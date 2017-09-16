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

/*
WISHES:

*/

/*
This service creates a video stream to the client INDEPENDENTLY on RFB connection.
*/

#include <list>

class MpegStreamer
{
	typedef list<MpegStreamer *> MpegStreamerList;

	class MpegStreamerConfigReloadListener : public ConfigReloadListener, public TvnServerListener
	{
	public:
		void onConfigReload(ServerConfig* serverConfig);
		void onTvnServerShutdown();
	};

public:
	static void Initialize(LogWriter* log, TvnServer* tvnServer, Configurator* configurator);

	static void Start(ULONG ip);
	static void Stop(ULONG ip);
	static void StopAll();

	static ULONG GetIp(SocketIPv4* s);

private:
	MpegStreamer(ULONG ip, USHORT port);
	void destroy();

	StringStorage commandLine;
	PROCESS_INFORMATION processInformation;
	//Process* process;
	SocketAddressIPv4 address; 

	static HANDLE anti_zombie_job;
	static MpegStreamer* get(ULONG ip);
	static LocalMutex lock;
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
