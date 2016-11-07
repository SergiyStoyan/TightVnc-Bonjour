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
	static void GetServiceName(StringStorage *agentName);
private:
	static BonjourServiceConfigReloadListener bonjourServiceConfigReloadListener;
	static bool initialized;
	static bool started;
	static StringStorage currentAgentName;
	static void start();
	static void stop();
	static HWND WINAPI bogus_hwnd;//used for WTSRegisterSessionNotificationEx to monitor user logon
	static HANDLE bogus_window_thread;
	static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI BogusWindowRun(void* Param);
};

#endif
