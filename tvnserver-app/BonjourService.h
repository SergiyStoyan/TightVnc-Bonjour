//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#ifndef _BONJOUR_SERVICE_H_
#define _BONJOUR_SERVICE_H_

#include "server-config-lib/Configurator.h"

class BonjourServiceConfigReloadListener : public ConfigReloadListener
{
public:
	void onConfigReload(ServerConfig *serverConfig);
};

class BonjourService
{
public:
	static void Initialize();
	static void Start(const TCHAR *bonjourAgentName);
	static void Stop();
private:
	static BonjourServiceConfigReloadListener bonjourServiceConfigReloadListener;
	static bool started;
};

#endif
