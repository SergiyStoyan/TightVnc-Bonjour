//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#ifndef _BONJOUR_SERVICE_H_
#define _BONJOUR_SERVICE_H_

#include "server-config-lib/Configurator.h"
#include "TvnServer.h"

class BonjourServiceConfigReloadListener : public ConfigReloadListener, public TvnServerListener
{
public:
	void onConfigReload(ServerConfig *serverConfig);
	void onTvnServerShutdown();
};

class BonjourService
{
public:
	static void Initialize(TvnServer *tvnServer, Configurator *configurator);
	static void Start();
	static void Stop();
private:
	static BonjourServiceConfigReloadListener bonjourServiceConfigReloadListener;
	static bool initialized;
	static bool started;
	static StringStorage currentAgentName;
	static void start();
	static void stop();
	static void getAgentName(StringStorage *agentName);
	//static HWND WINAPI bogus_hwnd;//used for WTSRegisterSessionNotificationEx to monitor user logon
};

#endif
