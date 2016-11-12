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

/*
WISHES:
- add help group to the settings tab
- change systray icon if the service is turned off or got troubles


*/


class BonjourService
{
	class BonjourServiceConfigReloadListener : public ConfigReloadListener, public TvnServerListener
	{
	public:
		void onConfigReload(ServerConfig* serverConfig);
		void onTvnServerShutdown();
	};

public:
	static void Initialize(LogWriter* log, TvnServer* tvnServer, Configurator* configurator);
	static void GetWindowsUserName(StringStorage* serviceName);
private:
	static void get_service_name(StringStorage* serviceName);
	static void start();
	static void stop();
	static BonjourServiceConfigReloadListener bonjourServiceConfigReloadListener;
	static bool initialized;
	static bool is_started();
	static StringStorage service_name;
	static uint16_t port;
	static StringStorage service_type;
	static HWND WINAPI bogus_hwnd;//used for WTSRegisterSessionNotificationEx to monitor user logon
	static HANDLE bogusWindowRun_thread;
	static LRESULT CALLBACK windowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI bogusWindowRun(void* Param);
	static LogWriter* log;
	struct dns_sd;
};

#endif
