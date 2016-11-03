
#include "BonjourService.h"

#include "ServerCommandLine.h"
#include "tvnserver-app/NamingDefs.h"

#include "win-system/SCMClient.h"
#include "win-system/Environment.h"

BonjourService::BonjourService(WinServiceEvents *winServiceEvents,
                       NewConnectionEvents *newConnectionEvents)
: Service(ServiceNames::SERVICE_NAME),// m_tvnServer(0),
//  m_winServiceEvents(winServiceEvents),
//  m_newConnectionEvents(newConnectionEvents),
  m_logServer(LogNames::LOG_PIPE_PUBLIC_NAME),
  m_clientLogger(LogNames::LOG_PIPE_PUBLIC_NAME, LogNames::SERVER_LOG_FILE_STUB_NAME)
{
}

BonjourService::~BonjourService()
{
}

void BonjourService::onStart()
{
	try {
		m_winServiceEvents->enable();
		// FIXME: Use real logger instead of zero.
		//m_tvnServer = new TvnServer(true, m_newConnectionEvents, this, &m_clientLogger);
		//m_tvnServer->addListener(this);
		m_winServiceEvents->onSuccServiceStart();
	}
	catch (Exception &e) {
		m_winServiceEvents->onFailedServiceStart(&StringStorage(e.getMessage()));
	}
}

void BonjourService::install()
{
  StringStorage binPath;

  //TvnService::getBinPath(&binPath);

  SCMClient scManager;

  scManager.installService(ServiceNames::SERVICE_NAME,
                           ServiceNames::SERVICE_NAME_TO_DISPLAY,
                           binPath.getString(), _T(""));
}

void BonjourService::remove()
{
  SCMClient scManager;

  scManager.removeService(ServiceNames::SERVICE_NAME);
}

void BonjourService::reinstall()
{
  try {
    remove();
  } catch (...) { }

  BonjourService::install();
}

void BonjourService::start(bool waitCompletion)
{
  SCMClient scManager;

  scManager.startService(ServiceNames::SERVICE_NAME, waitCompletion);
}

void BonjourService::stop(bool waitCompletion)
{
  SCMClient scManager;

  scManager.stopService(ServiceNames::SERVICE_NAME, waitCompletion);
}

//bool BonjourService::getBinPath(StringStorage *binPath)
//{
//  StringStorage pathToServiceBinary;
//
//  // Get executable folder first.
//  if (!Environment::getCurrentModulePath(&pathToServiceBinary)) {
//    return false;
//  }
//
//  // Create formatted binary path.
//  binPath->format(_T("\"%s\" %s"),
//                  pathToServiceBinary.getString(),
//                  SERVICE_COMMAND_LINE_KEY);
//
//  return true;
//}


