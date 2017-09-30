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
		TCHAR name_[255];
		mbstowcs(name_, name, sizeof(name_));
		if (errorCode == kDNSServiceErr_NoError)
		{
			if (flags & kDNSServiceFlagsAdd)
				log->info(_T("BonjourService: Service %s is registered and active."), name_);
			else
				log->info(_T("BonjourService: Service %s is unregistered."), name_);
		}
		else if (errorCode == kDNSServiceErr_NameConflict)
			log->error(_T("BonjourService: Service name %s is in use, please choose another."), name_);
		else
			log->error(_T("BonjourService: Error: %d"), errorCode);
	}

	static DWORD WINAPI handleEvents(void* param)
	{
		//while (true)
		//{
		//	while (!BonjourService::is_started())
		//		Sleep(100);

			try
			{
				int dns_sd_fd = DNSServiceRefSockFD(service);
				fd_set readfds;
				struct timeval tv;
				tv.tv_sec = 100;
				tv.tv_usec = 0;
				int result;

				while (BonjourService::is_started())
				{
					FD_ZERO(&readfds);
					FD_SET(dns_sd_fd, &readfds);
					result = select(dns_sd_fd + 1, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
					if (result > 0 && FD_ISSET(dns_sd_fd, &readfds))
					{
						DNSServiceErrorType err = kDNSServiceErr_NoError;
						err = DNSServiceProcessResult(service);
						if (err)
						{
							Sleep(100);//give time to stop service if it is being stopped
							if (BonjourService::is_started())
								log->error(_T("BonjourService: DNSServiceProcessResult returned %d"), err);
							break;
						}
					}
					else if (result < 0)
					{
						LPCSTR error = strerror(errno);
						TCHAR error_[512];
						mbstowcs(error_, error, sizeof(error_));
						log->error(_T("BonjourService: select() returned %d errno %d %s"), result, errno, error_);
						if (errno != EINTR)
							break;
					}
				}
			}
			catch(Exception e)
			{//ignore an exception if service was stopped and set to NULL
				if (BonjourService::is_started())
					log->interror(_T("BonjourService: Excpetion in handleEvents: %s"), e.getMessage());
			}
		//}
		return 0;
	}
	
	static HANDLE handleEvents_thread;
};

HANDLE BonjourService::dns_sd::handleEvents_thread = NULL;

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
StringStorage BonjourService::service_name = StringStorage(_T("-UNKNOWN-"));
uint16_t BonjourService::port = 5353;
StringStorage BonjourService::service_type = StringStorage(_T("_rfb._tcp"));
LogWriter *BonjourService::log;
HWND WINAPI BonjourService::bogus_hwnd = NULL;//used for WTSRegisterSessionNotificationEx to monitor user logon
HANDLE BonjourService::bogusWindowRun_thread = NULL;

LRESULT CALLBACK BonjourService::windowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

DWORD WINAPI BonjourService::bogusWindowRun(void* Param)
{
	LPCWSTR class_name = _T("MESSAGE_ONLY_CLASS");
	HINSTANCE hInstance = NULL;
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = windowProcedure;        // function which will handle messages
	wx.hInstance = hInstance;
	wx.lpszClassName = class_name;
	if (!RegisterClassEx(&wx))
	{
		log->interror(_T("BonjourService: Could not RegisterClassEx"));
		return 1;
	}
	bogus_hwnd = CreateWindowEx(0, class_name, _T("Bogus Window For Listening Messages"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
	if (!bogus_hwnd)
	{
		log->interror(_T("BonjourService: Could not CreateWindowEx"));
		return 1;
	}

	MSG msg; 
	BOOL bRet;
	while ((bRet = GetMessage(&msg, bogus_hwnd, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			log->interror(_T("BonjourService: GetMessage returned -1"));
			return 1;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

void BonjourService::Initialize(LogWriter *log, TvnServer *tvnServer)
{
	BonjourService::log = log;
	if (initialized)
	{
		BonjourService::log->interror(_T("BonjourService: Is already initialized"));
		return;
	}
		
	bogusWindowRun_thread = CreateThread(0, 0, bogusWindowRun, 0, 0, 0);
	if (!bogusWindowRun_thread)
	{
		BonjourService::log->interror(_T("BonjourService: Could not create bogusWindowRun_thread"));
		return;
	}

	/*dns_sd::handleEvents_thread = CreateThread(0, 0, dns_sd::handleEvents, NULL, 0, 0);
	if (!dns_sd::handleEvents_thread)
	{
		BonjourService::log->interror(_T("BonjourService: Could not create handleEvents_thread"));
		return;
	}*/

	tvnServer->addListener(&bonjourServiceConfigReloadListener);
	Configurator *configurator = Configurator::getInstance();
	configurator->addListener(&bonjourServiceConfigReloadListener);
	initialized = true;
	bonjourServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());
}

void BonjourService::start()
{
	if (!initialized)
	{
		log->interror(_T("BonjourService: Is not initialized"));
		return;
	}

	StringStorage service_name2;
	get_service_name(&service_name2);
	ServerConfig *sc = Configurator::getInstance()->getServerConfig();
	uint16_t port2 = sc->getBonjourServicePort();
	StringStorage service_type2;
	sc->getBonjourServiceType(&service_type2);
	if (service_name.isEqualTo(&service_name2) && port == port2 && service_type.isEqualTo(&service_type2))
	{
		if (is_started())
			return;
	}
	else
	{
		if (is_started())
			stop();
		service_name = service_name2;
		port = port2;
		service_type = service_type2;
	}

	int i = 0;
	while (!bogus_hwnd)//the window is created by another thread
	{
		if (i++ > 20)
		{
			log->interror(_T("BonjourService: bogus_hwnd is not created for too long time."));
			return;
		}
		Sleep(100);
	}
	if (!WTSRegisterSessionNotification(bogus_hwnd, NOTIFY_FOR_ALL_SESSIONS))
	{
		log->interror(_T("BonjourService: Could not WTSRegisterSessionNotification"));
		return;
	}

	if (dns_sd::service != NULL)
	{
		log->interror(_T("BonjourService: dns_sd::service != NULL. Refuse starting."));
		return;
	}

	DNSServiceFlags flags = 0;// kDNSServiceFlagsDefault;
	uint32_t interfaceIndex = 0;

	char service_name_[255];
	char service_type_[255];
#ifdef UNICODE
	//TCHAR == WCHAR
	wcstombs(service_name_, service_name.getString(), sizeof(service_name_));
	wcstombs(service_type_, service_type.getString(), sizeof(service_type_));
#else
	//TCHAR == char	
	strcpy(service_name_, (char *)service_name.getString());
	strcpy(service_type_, (char *)service_type.getString());
#endif

	const char* domain = NULL; // default domain
	const char* host = NULL; // default host
	uint16_t txtLen = 0;
	const char* txtRecord = NULL;
	void* context = NULL;

	DNSServiceErrorType err = DNSServiceRegister(
		&dns_sd::service,
		flags,
		interfaceIndex,
		service_name_,
		service_type_,
		domain,
		host,
		port, //htons(port),
		txtLen,
		txtRecord,
		dns_sd::serviceRegisterReply,
		context
	);
	if (err != kDNSServiceErr_NoError)
	{
		log->error(_T("BonjourService: Could not DNSServiceRegister. Error code: %d. Service name: %s. Port: %d. Service type: %s"), err, service_name.getString(), port, service_type.getString());
		return;
	}
	log->message(_T("BonjourService: Started. Service name: %s. Port: %d. Service type: %s"), service_name.getString(), port, service_type.getString());

	if (dns_sd::handleEvents_thread != NULL && WaitForSingleObject(dns_sd::handleEvents_thread, 0) != WAIT_OBJECT_0) //the thread is still alive
		log->interror(_T("BonjourService: handleEvents_thread is not NULL. A new handleEvents_thread will not be created."));
	else
	{
		dns_sd::handleEvents_thread = CreateThread(0, 0, dns_sd::handleEvents, NULL, 0, 0);
		if (!dns_sd::handleEvents_thread)
		{
			BonjourService::log->interror(_T("BonjourService: Could not create handleEvents_thread"));
			return;
		}
	}
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
			log->interror(_T("BonjourService: Could not WTSQuerySessionInformation"));
			return;
		}
		if (user_name_size < 1)
			serviceName->setString(_T("-UNKNOWN-"));
		else
			serviceName->setString(user_name);
	}

	/*TCHAR user_name[255];
	DWORD user_name_size = sizeof(user_name);
	if (!GetUserName(user_name, &user_name_size))
	throw Exception(_T("Could not GetUserName"));
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
	}

	if (!is_started())
		return;

	if (!WTSUnRegisterSessionNotification(bogus_hwnd))
	{
		log->interror(_T("BonjourService: Could not WTSUnRegisterSessionNotification"));
		return;
	}

	if (dns_sd::service != NULL)
	{
		DNSServiceRefDeallocate(dns_sd::service);
		dns_sd::service = NULL;
	}

	if (dns_sd::handleEvents_thread != NULL)
	{
		switch (WaitForSingleObject(dns_sd::handleEvents_thread, 3000))
		{
		case WAIT_TIMEOUT:
			log->interror(_T("BonjourService: Timeout on shutdown handleEvents_thread."));
			break;
		case WAIT_FAILED:
			log->interror(_T("BonjourService: Could not shutdown handleEvents_thread: %d"), GetLastError());
			break;
		default:
			CloseHandle(dns_sd::handleEvents_thread);
			dns_sd::handleEvents_thread = NULL;
		}
	}

	log->message(_T("BonjourService: Stopped."));
}

bool BonjourService::IsAvailable()
{
	void* q = GetModuleHandle(_T("dnssd"));
	//void* w = LoadLibrary(_T("dnssd"));
	return q != NULL;
}