//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "BonjourService.h"

void BonjourServiceConfigReloadListener::onConfigReload(ServerConfig *serverConfig)
{
	if (serverConfig->isBonjourServiceEnabled())
	{
		StringStorage ss;
		Configurator::getInstance()->getServerConfig()->getBonjourAgentName(&ss);
		BonjourService::Start(ss.getString());
	}
	else
		BonjourService::Stop();
}

void BonjourServiceConfigReloadListener::onTvnServerShutdown()
{
	BonjourService::Stop();
}

BonjourServiceConfigReloadListener BonjourService::bonjourServiceConfigReloadListener = BonjourServiceConfigReloadListener();
bool BonjourService::started = false;

void BonjourService::Initialize(TvnServer *tvnServer, Configurator *configurator)
{
	tvnServer->addListener(&BonjourService::bonjourServiceConfigReloadListener);
	configurator->addListener(&BonjourService::bonjourServiceConfigReloadListener);
	BonjourService::bonjourServiceConfigReloadListener.onConfigReload(configurator->getServerConfig());
}

void BonjourService::Start(const TCHAR *bonjourAgentName)
{
	if (BonjourService::started)
		return;
	BonjourService::started = true;
}

void BonjourService::Stop()
{
	if (!BonjourService::started)
		return;
	BonjourService::started = false;
}