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
StringStorage BonjourService::currentAgentName = NULL;

HWND WINAPI bogus_hwnd = NULL;//used for WTSRegisterSessionNotificationEx to monitor user logon
HANDLE bogus_window_thread = NULL;

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_WTSSESSION_CHANGE)
	{
		if (wParam == WTS_SESSION_LOGON)
		{
			BonjourService::Start();
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI BogusWindowRun(void* Param)
{
	LPCWSTR class_name = _T("MESSAGE_ONLY_CLASS");
	HINSTANCE hInstance = NULL;
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WindowProcedure;        // function which will handle messages
	wx.hInstance = hInstance;
	wx.lpszClassName = class_name;
	if (!RegisterClassEx(&wx))
		throw Exception(_T("Could not RegisterClassEx!"));
	bogus_hwnd = CreateWindowEx(0, class_name, _T("Bogus Window For Listening Messages"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
	if (!bogus_hwnd)
		throw Exception(_T("Could not create a bogus window!"));

	MSG msg; 
	BOOL bRet;
	while ((bRet = GetMessage(&msg, bogus_hwnd, 0, 0)) != 0)
	{
		if (bRet == -1)
			throw Exception(_T("GetMessage returned -1"));
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

void BonjourService::Initialize(TvnServer *tvnServer, Configurator *configurator)
{
	if(initialized)
		throw Exception(_T("BonjourService is already initialized."));
		
	bogus_window_thread = CreateThread(0, 0, BogusWindowRun, 0, 0, 0);
	if (!bogus_window_thread)
		throw Exception(_T("Could not CreateThread."));

	tvnServer->addListener(&bonjourServiceConfigReloadListener);
	configurator->addListener(&bonjourServiceConfigReloadListener);
	started = false;
	currentAgentName = NULL;
	initialized = true;
	bonjourServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());
}

void BonjourService::Start()
{
	if (!initialized)
		throw Exception(_T("BonjourService is not initialized."));

	StringStorage agent_name;
	getAgentName(&agent_name);
	if (currentAgentName.isEqualTo(&agent_name))
	{
		if (started)
			return;
	}
	else
	{
		if (started)
			Stop();
		currentAgentName = agent_name;
	}
	
	int i = 0;
	while (!bogus_hwnd)//the window is created by another thread
	{
		if (i++ > 20)
			throw Exception(_T("bogus_hwnd is not created too long!"));
		Sleep(100);
	}
	if (!WTSRegisterSessionNotification(bogus_hwnd, NOTIFY_FOR_ALL_SESSIONS))
		throw Exception(_T("Could not WTSRegisterSessionNotification!"));

	start();
	started = true;

	//test
	//SendMessage(bogus_hwnd, WM_WTSSESSION_CHANGE, WTS_SESSION_LOGON, 0);
}

void BonjourService::start()
{
}

void BonjourService::getAgentName(StringStorage *agentName)
{
	ServerConfig *sc = Configurator::getInstance()->getServerConfig();
	if (sc->isWindowsUserNameAsBonjourAgentNameUsed())
	{
		TCHAR user_name[255];
		DWORD user_name_size = sizeof(user_name);
		if (!GetUserName(user_name, &user_name_size))
			throw Exception(_T("Could not GetUserName!"));
		agentName->setString(user_name);
	}
	else
		sc->getBonjourAgentName(agentName);
}

void BonjourService::Stop()
{
	if (!initialized)
		throw Exception(_T("BonjourService is not initialized."));

	if (!started)
		return;

	if(!WTSUnRegisterSessionNotification(bogus_hwnd))
		throw Exception(_T("Could not WTSUnRegisterSessionNotification!"));

	stop();
	started = false;
}

void BonjourService::stop()
{
}