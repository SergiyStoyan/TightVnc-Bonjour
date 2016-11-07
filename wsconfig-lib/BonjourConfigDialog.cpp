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
	m_useWindowsUserAsBonjourServiceName.setWindow(GetDlgItem(dialogHwnd, IDC_CHECK_BONJOUR_USE_WINDOWS_USER_NAME_AS_AGENT_NAME));
	m_BonjourServiceName.setWindow(GetDlgItem(dialogHwnd, IDC_EDIT_BONJOUR_SERVICE_NAME));
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
		if (notificationID == BN_CLICKED)
			onUseWindowsUserAsBonjourServiceNameClick();
		break;
	case IDC_EDIT_BONJOUR_SERVICE_NAME:
		if (notificationID == EN_UPDATE)
			onBonjourServiceNameChange();
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
	m_useWindowsUserAsBonjourServiceName.setEnabled(m_enableBonjourService.isChecked());
	m_BonjourServiceName.setEnabled(m_enableBonjourService.isChecked() && !m_useWindowsUserAsBonjourServiceName.isChecked());
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

void BonjourConfigDialog::onUseWindowsUserAsBonjourServiceNameClick()
{
	m_BonjourServiceName.setEnabled(!m_useWindowsUserAsBonjourServiceName.isChecked());
	if (m_useWindowsUserAsBonjourServiceName.isChecked())
	{
		StringStorage agent_name;
		BonjourService::GetServiceName(&agent_name);
		m_BonjourServiceName.setText(agent_name.getString());
	}
	((ConfigDialog *)m_parent)->updateApplyButtonState();	
}

void BonjourConfigDialog::onBonjourServiceNameChange()
{
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

bool BonjourConfigDialog::validateInput()
{
	if (!m_enableBonjourService.isChecked())
		return true;

	StringStorage name;
	m_BonjourServiceName.getText(&name);
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

	m_useWindowsUserAsBonjourServiceName.setEnabled(m_enableBonjourService.isChecked());

	m_useWindowsUserAsBonjourServiceName.check(m_config->isWindowsUserAsBonjourServiceNameUsed());
	if (m_useWindowsUserAsBonjourServiceName.isChecked())
	{
		m_BonjourServiceName.setEnabled(false);
		StringStorage agent_name;
		BonjourService::GetServiceName(&agent_name);
		m_BonjourServiceName.setText(agent_name.getString());
	}
	else
	{
		m_BonjourServiceName.setEnabled(m_config->isBonjourServiceEnabled());
		StringStorage ss;
		m_config->getBonjourServiceName(&ss);
		m_BonjourServiceName.setText(ss.getString());
	}
}

void BonjourConfigDialog::apply()
{
	AutoLock al(m_config);
	m_config->enableBonjourService(m_enableBonjourService.isChecked());

	StringStorage ss;
	m_BonjourServiceName.getText(&ss);
	m_config->setBonjourServiceName(ss.getString());
}
