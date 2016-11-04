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
	static void Start(const TCHAR *bonjourAgentName);
	static void Stop();
private:
	static BonjourServiceConfigReloadListener bonjourServiceConfigReloadListener;
	static bool started;
};

#endif
