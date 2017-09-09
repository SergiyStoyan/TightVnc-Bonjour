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

using namespace std;

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
};

#endif
