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


class ScreenStreamingService
{
	class ScreenStreamingServiceConfigReloadListener : public ConfigReloadListener, public TvnServerListener
	{
	public:
		void onConfigReload(ServerConfig* serverConfig);
		void onTvnServerShutdown();
	};

public:
	static void Initialize(LogWriter* log, TvnServer* tvnServer, Configurator* configurator);
	static bool IsAvailable();

private:
	static void start();
	static void stop();
	static ScreenStreamingServiceConfigReloadListener screenStreamingServiceConfigReloadListener;
	static bool initialized;
	static bool is_started();
	static StringStorage service_name;
	static uint16_t port;
	static StringStorage service_type;
	static LogWriter* log;
};

#endif
