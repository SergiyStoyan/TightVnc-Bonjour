//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "BonjourConfigDialog.h"
#include "ConfigDialog.h"
#include "tvnserver/resource.h"

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
  m_BonjourAgentName.setWindow(GetDlgItem(dialogHwnd, IDC_EDIT_BONJOUR_SERVICE_NAME));
}

BOOL BonjourConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
	switch (controlID)
	{
	case IDC_CHECK_BONJOUR_ENABLED:
			onBonjourEnabledClick();
		switch (notificationID)
		{
		case LBN_SELCHANGE:
			break;
		case LBN_DBLCLK:
			break;
		}
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
	//ConfigDialog *configDialog = (ConfigDialog *)m_parent;
	m_enableBonjourService.check(m_config->isBonjourServiceEnabled());

	StringStorage ss;
	m_config->getBonjourAgentName(&ss);
	m_BonjourAgentName.setText(ss.getString());
	m_BonjourAgentName.setEnabled(m_config->isBonjourServiceEnabled());
}

void BonjourConfigDialog::apply()
{
	AutoLock al(m_config);
	m_config->enableBonjourService(m_enableBonjourService.isChecked());

	StringStorage ss;
	m_BonjourAgentName.getText(&ss);
	m_config->setBonjourAgentName(ss.getString());
}
