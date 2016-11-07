//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "BonjourConfigDialog.h"
#include "ConfigDialog.h"
#include "tvnserver/resource.h"
#include "tvnserver-app/BonjourService.h"

BonjourConfigDialog::BonjourConfigDialog()
: BaseDialog(IDD_CONFIG_BONJOUR_PAGE), m_parent(NULL)
{
}

BonjourConfigDialog::~BonjourConfigDialog()
{
}

void BonjourConfigDialog::setParentDialog(BaseDialog *dialog)
{
  m_parent = dialog;
}

void BonjourConfigDialog::initControls()
{
	HWND dialogHwnd = m_ctrlThis.getWindow();

	m_enableBonjourService.setWindow(GetDlgItem(dialogHwnd, IDC_CHECK_BONJOUR_ENABLED));
	m_useWindowsUserAsBonjourAgentName.setWindow(GetDlgItem(dialogHwnd, IDC_CHECK_BONJOUR_USE_WINDOWS_USER_NAME_AS_AGENT_NAME));
	m_BonjourAgentName.setWindow(GetDlgItem(dialogHwnd, IDC_EDIT_BONJOUR_SERVICE_NAME));
}

BOOL BonjourConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
	switch (controlID)
	{
	case IDC_CHECK_BONJOUR_ENABLED:
		if (notificationID == BN_CLICKED) 
			onBonjourEnabledClick();
		break;
	case IDC_CHECK_BONJOUR_USE_WINDOWS_USER_NAME_AS_AGENT_NAME:
		if (notificationID == EN_UPDATE)
			onUseWindowsUserAsBonjourAgentNameClick();
		break;
	case IDC_EDIT_BONJOUR_SERVICE_NAME:
		if (notificationID == EN_UPDATE)
			onBonjourAgentNameChange();
		break;
}
	return TRUE;
}

BOOL BonjourConfigDialog::onInitDialog()
{
	m_config = Configurator::getInstance()->getServerConfig();

	initControls();
	updateUI();

  return TRUE;
}

void BonjourConfigDialog::onBonjourEnabledClick()
{
	m_useWindowsUserAsBonjourAgentName.setEnabled(m_enableBonjourService.isChecked());
	m_BonjourAgentName.setEnabled(m_enableBonjourService.isChecked() && !m_useWindowsUserAsBonjourAgentName.isChecked());
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

void BonjourConfigDialog::onUseWindowsUserAsBonjourAgentNameClick()
{
	m_BonjourAgentName.setEnabled(!m_useWindowsUserAsBonjourAgentName.isChecked());
	if (m_useWindowsUserAsBonjourAgentName.isChecked())
	{
		StringStorage agent_name;
		BonjourService::GetAgentName(&agent_name);
		m_BonjourAgentName.setText(agent_name.getString());
	}
	((ConfigDialog *)m_parent)->updateApplyButtonState();	
}

void BonjourConfigDialog::onBonjourAgentNameChange()
{
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

bool BonjourConfigDialog::validateInput()
{
	if (!m_enableBonjourService.isChecked())
		return true;

	StringStorage name;
	m_BonjourAgentName.getText(&name);
	if (name.getLength() < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_BONJOUR_SERVICE_NAME_NOTIFICATION),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	return true;
}

void BonjourConfigDialog::updateUI()
{
	m_enableBonjourService.check(m_config->isBonjourServiceEnabled());

	m_useWindowsUserAsBonjourAgentName.setEnabled(m_enableBonjourService.isChecked());

	m_useWindowsUserAsBonjourAgentName.check(m_config->isWindowsUserAsBonjourAgentNameUsed());
	if (m_useWindowsUserAsBonjourAgentName.isChecked())
	{
		m_BonjourAgentName.setEnabled(false);
		StringStorage agent_name;
		BonjourService::GetAgentName(&agent_name);
		m_BonjourAgentName.setText(agent_name.getString());
	}
	else
	{
		m_BonjourAgentName.setEnabled(m_config->isBonjourServiceEnabled());
		StringStorage ss;
		m_config->getBonjourAgentName(&ss);
		m_BonjourAgentName.setText(ss.getString());
	}
}

void BonjourConfigDialog::apply()
{
	AutoLock al(m_config);
	m_config->enableBonjourService(m_enableBonjourService.isChecked());

	StringStorage ss;
	m_BonjourAgentName.getText(&ss);
	m_config->setBonjourAgentName(ss.getString());
}
