//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "BonjourService.h"
#include <tchar.h>
#include "dns_sd.h"

struct BonjourService::dns_sd//everything that requires Bonjour SDK (dns_sd.h)
{
	static DNSServiceRef service;

	static void DNSSD_API serviceRegisterReply(
		DNSServiceRef                       sdRef,
		DNSServiceFlags                     flags,
		DNSServiceErrorType                 errorCode,
		const char                          *name,
		const char                          *regtype,
		const char                          *domain,
		void                                *context
	)
	{
		if (errorCode == kDNSServiceErr_NoError)
		{
			if (flags & kDNSServiceFlagsAdd)
				log->info(_T("BonjourService: Service %s is registered and active\n"), name);
			else
				log->info(_T("BonjourService: Service %s registration removed\n"), name);
		}
		else if (errorCode == kDNSServiceErr_NameConflict)
			log->error(_T("BonjourService: Service name %s is in use, please choose another\n"), name);
		else
			log->error(_T("BonjourService: Error: %d\n"), errorCode);
	}
};

bool BonjourService::is_started()
{
	return dns_sd::service != NULL;
}
DNSServiceRef BonjourService::dns_sd::service = NULL;

void BonjourService::BonjourServiceConfigReloadListener::onConfigReload(ServerConfig *serverConfig)
{
	if (serverConfig->isBonjourServiceEnabled())
		BonjourService::start();
	else
		BonjourService::stop();
}

void BonjourService::BonjourServiceConfigReloadListener::onTvnServerShutdown()
{
	BonjourService::stop();
}

BonjourService::BonjourServiceConfigReloadListener BonjourService::bonjourServiceConfigReloadListener = BonjourServiceConfigReloadListener();
bool BonjourService::initialized = false;
StringStorage BonjourService::service_name = StringStorage(_T("NULL"));
uint16_t BonjourService::port = 5353;
LogWriter *BonjourService::log;
HWND WINAPI BonjourService::bogus_hwnd = NULL;//used for WTSRegisterSessionNotificationEx to monitor user logon
HANDLE BonjourService::bogus_window_thread = NULL;

LRESULT CALLBACK BonjourService::WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_WTSSESSION_CHANGE)
	{
		switch (wParam)
		{
		case WTS_SESSION_LOGON:
			log->message(_T("BonjourService: WTS_SESSION_LOGON."));
			//DWORD sessionId = lParam;
			BonjourService::start();
			break;
		case WTS_SESSION_LOGOFF:
			log->message(_T("BonjourService: WTS_SESSION_LOGOFF."));
			BonjourService::start();
			break;
			/*	WTS_CONSOLE_CONNECT
				WTS_CONSOLE_DISCONNECT
				WTS_REMOTE_CONNECT
				WTS_REMOTE_DISCONNECT
				WTS_SESSION_LOGON*/
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI BonjourService::BogusWindowRun(void* Param)
{
	LPCWSTR class_name = _T("MESSAGE_ONLY_CLASS");
	HINSTANCE hInstance = NULL;
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WindowProcedure;        // function which will handle messages
	wx.hInstance = hInstance;
	wx.lpszClassName = class_name;
	if (!RegisterClassEx(&wx))
	{
		log->interror(_T("BonjourService: Could not RegisterClassEx!"));
		return 1;
		//throw Exception(_T("BonjourService: Could not RegisterClassEx!"));
	}
	bogus_hwnd = CreateWindowEx(0, class_name, _T("Bogus Window For Listening Messages"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
	if (!bogus_hwnd)
	{
		log->interror(_T("BonjourService: Could not CreateWindowEx!"));
		return 1;
		//throw Exception(_T("BonjourService: Could not CreateWindowEx!"));
	}

	MSG msg; 
	BOOL bRet;
	while ((bRet = GetMessage(&msg, bogus_hwnd, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			log->interror(_T("BonjourService: GetMessage returned -1"));
			return 1;
			//throw Exception(_T("BonjourService: GetMessage returned -1"));
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

void BonjourService::Initialize(LogWriter *log, TvnServer *tvnServer, Configurator *configurator)
{
	BonjourService::log = log;
	if (initialized)
	{
		BonjourService::log->interror(_T("BonjourService: Is already initialized!"));
		return;
		//throw Exception(_T("BonjourService: Is already initialized!"));
	}
		
	bogus_window_thread = CreateThread(0, 0, BogusWindowRun, 0, 0, 0);
	if (!bogus_window_thread)
	{
		BonjourService::log->interror(_T("BonjourService: Could not CreateThread!"));
		return;
		//throw Exception(_T("BonjourService: Could not CreateThread!"));
	}

	tvnServer->addListener(&bonjourServiceConfigReloadListener);
	configurator->addListener(&bonjourServiceConfigReloadListener);
	initialized = true;
	bonjourServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());
}

void BonjourService::start()
{
	if (!initialized)
	{
		log->interror(_T("BonjourService: Is not initialized!"));
		return;
		//throw Exception(_T("BonjourService: Is not initialized!"));
	}

	StringStorage service_name2;
	get_service_name(&service_name2);
	if (service_name.isEqualTo(&service_name2))
	{
		if (is_started())
			return;
	}
	else
	{
		if (is_started())
			stop();
		service_name = service_name2;
	}
	
	int i = 0;
	while (!bogus_hwnd)//the window is created by another thread
	{
		if (i++ > 20)
		{
			log->interror(_T("BonjourService: bogus_hwnd is not created for too long time!"));
			return;
			//throw Exception(_T("BonjourService: bogus_hwnd is not created for too long time!"));
		}
		Sleep(100);
	}
	if (!WTSRegisterSessionNotification(bogus_hwnd, NOTIFY_FOR_ALL_SESSIONS))
	{
		log->interror(_T("BonjourService: Could not WTSRegisterSessionNotification!"));
		return;
		//throw Exception(_T("BonjourService: Could not WTSRegisterSessionNotification!"));
	}

	start_();
}

void BonjourService::start_()
{
	DNSServiceFlags flags = 0;// kDNSServiceFlagsDefault;
	uint32_t interfaceIndex = 0;
	char service_name_[255];
#ifdef UNICODE
	//It means TCHAR == WCHAR.
	//WideCharToMultiByte();
	wcstombs(service_name_, service_name.getString(), strlen(service_name_));
#else
	//It means TCHAR == char.	
	strcpy(service_name_, (char *)service_name.getString());
#endif
		
	const char* regType = "_rfb._tcp";
	const char* domain = NULL; // default domain
	const char* host = NULL; // default host	
	uint16_t txtLen = 0;
	const char* txtRecord = NULL;
	DNSServiceRegisterReply callBack = callBack;
	void* context = NULL;

	int err = DNSServiceRegister(
		&dns_sd::service,
		flags,
		interfaceIndex,
		service_name_,
		regType,
		domain,
		host,
		htons(port),
		txtLen,
		txtRecord,
		dns_sd::serviceRegisterReply,
		context
	);
	if (err != kDNSServiceErr_NoError)
	{
		log->interror(_T("BonjourService: Could not DNSServiceRegister. Error code: %d!"), err);
		return;
	}

	log->message(_T("BonjourService: Started. Service name: %s"), service_name.getString());
}

void BonjourService::get_service_name(StringStorage *serviceName)
{
	ServerConfig *sc = Configurator::getInstance()->getServerConfig();
	if (sc->isWindowsUserAsBonjourServiceNameUsed())
		GetWindowsUserName(serviceName);
	else
		sc->getBonjourServiceName(serviceName);
}

void BonjourService::GetWindowsUserName(StringStorage *serviceName)
{
	DWORD session_id = WTSGetActiveConsoleSessionId();
	if (session_id == 0xFFFFFFFF)
		serviceName->setString(_T("-NO_PHYSICAL_CONSOLE_SESSION-"));
	else
	{
		LPWSTR user_name;
		DWORD user_name_size = sizeof(user_name);
		if (!WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, session_id, WTSUserName, &user_name, &user_name_size))
		{
			log->interror(_T("BonjourService: Could not WTSQuerySessionInformation!"));
			return;
			//throw Exception(_T("BonjourService: Could not WTSQuerySessionInformation!"));
		}
		if (user_name_size < 1)
			serviceName->setString(_T("-UNKNOWN-"));
		else
			serviceName->setString(user_name);
	}

	/*TCHAR user_name[255];
	DWORD user_name_size = sizeof(user_name);
	if (!GetUserName(user_name, &user_name_size))
	throw Exception(_T("Could not GetUserName!"));
	if (user_name_size < 1)
	_tcscpy(user_name, _T("-NOBODY-"));
	agentName->setString(user_name);*/
}

void BonjourService::stop()
{
	if (!initialized)
	{
		log->interror(_T("BonjourService: Is not initialized!"));
		return;
		//throw Exception(_T("BonjourService: Is not initialized!"));
	}

	if (!is_started())
		return;

	if (!WTSUnRegisterSessionNotification(bogus_hwnd))
	{
		log->interror(_T("BonjourService: Could not WTSUnRegisterSessionNotification!"));
		return;
		//throw Exception(_T("BonjourService: Could not WTSUnRegisterSessionNotification!"));
	}

	stop_();
}

void BonjourService::stop_()
{
	if (dns_sd::service != NULL)
		DNSServiceRefDeallocate(dns_sd::service);
	dns_sd::service = NULL;
	
	log->message(_T("BonjourService: Stopped."));
}