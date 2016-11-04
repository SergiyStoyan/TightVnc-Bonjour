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

BonjourServiceConfigReloadListener BonjourService::bonjourServiceConfigReloadListener = BonjourServiceConfigReloadListener();
bool BonjourService::started = false;

void BonjourService::Initialize()
{
	Configurator::getInstance()->addListener(&BonjourService::bonjourServiceConfigReloadListener);
	BonjourService::bonjourServiceConfigReloadListener.onConfigReload(Configurator::getInstance()->getServerConfig());
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