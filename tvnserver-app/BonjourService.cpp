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
HWND WINAPI BonjourService::bogus_hwnd = NULL;
StringStorage BonjourService::currentAgentName = NULL;

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_WTSSESSION_CHANGE)
	{
		if (wParam == WTS_SESSION_LOGON)
		{
			TCHAR user_name[255];
			DWORD user_name_size = sizeof(user_name);
			if (!GetUserName(user_name, &user_name_size))
				throw Exception(_T("Could not GetUserName!"));
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void BonjourService::Initialize(TvnServer *tvnServer, Configurator *configurator)
{
	if(initialized)
		throw Exception(_T("BonjourService is already initialized."));
	
	//bogus_hwnd = CreateWindow(
	//	_T("BogusClass"),        // name of window class 
	//	_T("Bogus"),            // title-bar string 
	//	WS_OVERLAPPEDWINDOW, // top-level window 
	//	CW_USEDEFAULT,       // default horizontal position 
	//	CW_USEDEFAULT,       // default vertical position 
	//	CW_USEDEFAULT,       // default width 
	//	CW_USEDEFAULT,       // default height 
	//	(HWND)NULL,         // no owner window 
	//	(HMENU)NULL,        // use class menu 
	//	(HINSTANCE)NULL,           // handle to application instance 
	//	(LPVOID)NULL);      // no window-creation data 
	//if (!bogus_hwnd)
	//	throw Exception(_T("Could not create a bogus window!"));

	LPCWSTR class_name = _T("MESSAGE_ONLY_CLASS");
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WindowProcedure;        // function which will handle messages
	wx.hInstance = NULL;
	wx.lpszClassName = class_name;
	if (!RegisterClassEx(&wx))
		throw Exception(_T("Could not RegisterClassEx!")); 
	BonjourService::bogus_hwnd = CreateWindowEx(0, class_name, _T("MESSAGE_ONLY"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	if (!BonjourService::bogus_hwnd)
		throw Exception(_T("Could not create a bogus window!"));

	//HANDLE thread = CreateThread(0, 0, ThreadProc, 0, 0, 0);


	tvnServer->addListener(&bonjourServiceConfigReloadListener);
	configurator->addListener(&bonjourServiceConfigReloadListener);
	bonjourServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());
	started = false;
	currentAgentName = NULL;

	initialized = true;
}

void g()
{
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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
	
	if (!WTSRegisterSessionNotification(BonjourService::bogus_hwnd, NOTIFY_FOR_ALL_SESSIONS))
		throw Exception(_T("Could not WTSRegisterSessionNotification!"));



	started = true;
}

void BonjourService::start()
{
}

void BonjourService::getAgentName(StringStorage *agentName)
{
	ServerConfig *sc = Configurator::getInstance()->getServerConfig();
	if (sc->isWindowsUserNameAsBonjourAgentNameUsed())
	{

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


	started = false;
}