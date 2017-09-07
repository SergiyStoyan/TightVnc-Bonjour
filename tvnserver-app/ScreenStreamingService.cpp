//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "ScreenStreamingService.h"
#include <tchar.h>

bool ScreenStreamingService::is_started()
{
	//return dns_sd::service != NULL;
	return true;
}

void ScreenStreamingService::ScreenStreamingServiceConfigReloadListener::onConfigReload(ServerConfig *serverConfig)
{
	/*if (serverConfig->isBonjourServiceEnabled())
		BonjourService::start();
	else
		BonjourService::stop();*/
}

void ScreenStreamingService::ScreenStreamingServiceConfigReloadListener::onTvnServerShutdown()
{
	ScreenStreamingService::stop();
}

ScreenStreamingService::ScreenStreamingServiceConfigReloadListener ScreenStreamingService::screenStreamingServiceConfigReloadListener = ScreenStreamingServiceConfigReloadListener();
bool ScreenStreamingService::initialized = false;
LogWriter *ScreenStreamingService::log;

void ScreenStreamingService::Initialize(LogWriter *log, TvnServer *tvnServer, Configurator *configurator)
{
	ScreenStreamingService::log = log;
	if (initialized)
	{
		ScreenStreamingService::log->interror(_T("ScreenStreamingService: Is already initialized"));
		return;
	}

	/*dns_sd::handleEvents_thread = CreateThread(0, 0, dns_sd::handleEvents, NULL, 0, 0);
	if (!dns_sd::handleEvents_thread)
	{
		BonjourService::log->interror(_T("BonjourService: Could not create handleEvents_thread"));
		return;
	}*/

	tvnServer->addListener(&screenStreamingServiceConfigReloadListener);
	configurator->addListener(&screenStreamingServiceConfigReloadListener);
	initialized = true;
	screenStreamingServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());
}

void ScreenStreamingService::start()
{
	if (!initialized)
	{
		log->interror(_T("ScreenStreamingService: Is not initialized"));
		return;
	}

	/*StringStorage service_name2;
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
	}*/
	
	//if (err != kDNSServiceErr_NoError)
	//{
	//	log->error(_T("BonjourService: Could not DNSServiceRegister. Error code: %d. Service name: %s. Port: %d. Service type: %s"), err, service_name.getString(), port, service_type.getString());
	//	return;
	//}
	//log->message(_T("BonjourService: Started. Service name: %s. Port: %d. Service type: %s"), service_name.getString(), port, service_type.getString());
}

//void ScreenStreamingService::get_service_name(StringStorage *serviceName)
//{
//	ServerConfig *sc = Configurator::getInstance()->getServerConfig();
//	/*if (sc->isWindowsUserAsBonjourServiceNameUsed())
//		GetWindowsUserName(serviceName);
//	else
//		sc->getBonjourServiceName(serviceName);*/
//}

void ScreenStreamingService::stop()
{
	if (!initialized)
	{
		log->interror(_T("ScreenStreamingService: Is not initialized!"));
		return;
	}

	if (!is_started())
		return;
	
	/*if (dns_sd::handleEvents_thread != NULL)
	{
		switch (WaitForSingleObject(dns_sd::handleEvents_thread, 3000))
		{
		case WAIT_TIMEOUT:
			log->interror(_T("ScreenStreamingService: Timeout on shutdown handleEvents_thread."));
			break;
		case WAIT_FAILED:
			log->interror(_T("ScreenStreamingService: Could not shutdown handleEvents_thread: %d"), GetLastError());
			break;
		default:
			CloseHandle(dns_sd::handleEvents_thread);
			dns_sd::handleEvents_thread = NULL;
		}
	}*/

	log->message(_T("ScreenStreamingService: Stopped."));
}

bool ScreenStreamingService::IsAvailable()
{
	/*void* q = GetModuleHandle(_T("dnssd"));
	return q != NULL;*/
	return true;
}