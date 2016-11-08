//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "BonjourService.h"
#include <tchar.h>

void BonjourServiceConfigReloadListener::onConfigReload(ServerConfig *serverConfig)
{
	if (serverConfig->isBonjourServiceEnabled())
		BonjourService::Start();
	else
		BonjourService::Stop();
}

void BonjourServiceConfigReloadListener::onTvnServerShutdown()
{
	BonjourService::Stop();
}

BonjourServiceConfigReloadListener BonjourService::bonjourServiceConfigReloadListener = BonjourServiceConfigReloadListener();
bool BonjourService::initialized = false;
bool BonjourService::started = false;
StringStorage BonjourService::current_service_name = NULL;
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
			BonjourService::Start();
			break;
		case WTS_SESSION_LOGOFF:
			log->message(_T("BonjourService: WTS_SESSION_LOGOFF."));
			BonjourService::Start();
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
		throw Exception(_T("BonjourService: Could not RegisterClassEx!"));
	}
	bogus_hwnd = CreateWindowEx(0, class_name, _T("Bogus Window For Listening Messages"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
	if (!bogus_hwnd)
	{
		log->interror(_T("BonjourService: Could not CreateWindowEx!"));
		throw Exception(_T("BonjourService: Could not CreateWindowEx!"));
	}

	MSG msg; 
	BOOL bRet;
	while ((bRet = GetMessage(&msg, bogus_hwnd, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			log->interror(_T("BonjourService: GetMessage returned -1"));
			throw Exception(_T("BonjourService: GetMessage returned -1"));
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
		throw Exception(_T("BonjourService: Is already initialized!"));
	}
		
	bogus_window_thread = CreateThread(0, 0, BogusWindowRun, 0, 0, 0);
	if (!bogus_window_thread)
	{
		BonjourService::log->interror(_T("BonjourService: Could not CreateThread!"));
		throw Exception(_T("BonjourService: Could not CreateThread!"));
	}

	tvnServer->addListener(&bonjourServiceConfigReloadListener);
	configurator->addListener(&bonjourServiceConfigReloadListener);
	started = false;
	current_service_name = NULL;
	initialized = true;
	bonjourServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());
}

void BonjourService::Start()
{
	if (!initialized)
	{
		log->interror(_T("BonjourService: Is not initialized!"));
		throw Exception(_T("BonjourService: Is not initialized!"));
	}

	StringStorage service_name;
	GetServiceName(&service_name);
	if (current_service_name.isEqualTo(&service_name))
	{
		if (started)
			return;
	}
	else
	{
		if (started)
			Stop();
		current_service_name = service_name;
	}
	
	int i = 0;
	while (!bogus_hwnd)//the window is created by another thread
	{
		if (i++ > 20)
		{
			log->interror(_T("BonjourService: bogus_hwnd is not created for too long time!"));
			throw Exception(_T("BonjourService: bogus_hwnd is not created for too long time!"));
		}
		Sleep(100);
	}
	if (!WTSRegisterSessionNotification(bogus_hwnd, NOTIFY_FOR_ALL_SESSIONS))
	{
		log->interror(_T("BonjourService: Could not WTSRegisterSessionNotification!"));
		throw Exception(_T("BonjourService: Could not WTSRegisterSessionNotification!"));
	}

	start();
}

void BonjourService::start()
{



	started = true;

	log->message(_T("BonjourService: Started. Service name: %s"), current_service_name.getString());
}

void BonjourService::GetServiceName(StringStorage *agentName)
{
	ServerConfig *sc = Configurator::getInstance()->getServerConfig();
	if (sc->isWindowsUserAsBonjourServiceNameUsed())
	{
		DWORD session_id = WTSGetActiveConsoleSessionId();
		if (session_id == 0xFFFFFFFF)
			agentName->setString(_T("-NO_PHYSICAL_CONSOLE_SESSION-"));
		else
		{
			LPWSTR user_name;
			DWORD user_name_size = sizeof(user_name);
			if (!WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, session_id, WTSUserName, &user_name, &user_name_size))
			{
				log->interror(_T("BonjourService: Could not WTSQuerySessionInformation!"));
				throw Exception(_T("BonjourService: Could not WTSQuerySessionInformation!"));
			}
			agentName->setString(user_name);
		}

		/*TCHAR user_name[255];
		DWORD user_name_size = sizeof(user_name);
		if (!GetUserName(user_name, &user_name_size))
			throw Exception(_T("Could not GetUserName!"));
		if (user_name_size < 1)
			_tcscpy(user_name, _T("-NOBODY-"));
		agentName->setString(user_name);*/
	}
	else
		sc->getBonjourServiceName(agentName);
	//MessageBox(0, agentName->getString(), _T("qqqqq"), MB_OK | MB_ICONERROR);
}

void BonjourService::Stop()
{
	if (!initialized)
	{
		log->interror(_T("BonjourService: Is not initialized!"));
		throw Exception(_T("BonjourService: Is not initialized!"));
	}

	if (!started)
		return;

	if (!WTSUnRegisterSessionNotification(bogus_hwnd))
	{
		log->interror(_T("BonjourService: Could not WTSUnRegisterSessionNotification!"));
		throw Exception(_T("BonjourService: Could not WTSUnRegisterSessionNotification!"));
	}
}

void BonjourService::stop()
{



	stop();
	started = false;

	log->message(_T("BonjourService: Stopped."));
}