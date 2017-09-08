//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#ifndef _SCREEN_STREAMING_SERVICE_H_
#define _SCREEN_STREAMING_SERVICE_H_

#include "server-config-lib/Configurator.h"
#include "TvnServer.h"

/*
WISHES:

*/

/*
This service creates a video stream to the client INDEPENDENTLY on RFB connection.
*/

using namespace std;

class ScreenStreamingService
{
	typedef list<ScreenStreamingService *> ScreenStreamingServiceList;

	class ScreenStreamingServiceConfigReloadListener : public ConfigReloadListener, public TvnServerListener
	{
	public:
		void onConfigReload(ServerConfig* serverConfig);
		void onTvnServerShutdown();
	};

public:
	static void Initialize(LogWriter* log, TvnServer* tvnServer, Configurator* configurator);

	static ScreenStreamingService* Start(const TCHAR* host);
	static ScreenStreamingService* Get(const TCHAR* host);
	static void Stop(const TCHAR* host);
	void Stop();
	bool IsRunning();
	static void StopAll();

	static void GetIpString(SocketIPv4* s, StringStorage* ip);

private:
	ScreenStreamingService(const TCHAR* host, USHORT port);

	LPPROCESS_INFORMATION lpProcessInformation;
	SocketAddressIPv4 address; 

	static LocalMutex lock;
	static ScreenStreamingServiceList screenStreamingServiceList;
	static ScreenStreamingServiceConfigReloadListener screenStreamingServiceConfigReloadListener;
	static bool initialized;
	static LogWriter* log;
	static ServerConfig* serverConfig;
};


#endif
