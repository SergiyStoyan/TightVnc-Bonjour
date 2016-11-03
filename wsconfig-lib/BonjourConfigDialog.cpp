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

  m_BonjourEnabledCheckBox.setWindow(GetDlgItem(dialogHwnd, IDC_CHECK_BONJOUR_ENABLED));
  m_BonjourServiceNameTextBox.setWindow(GetDlgItem(dialogHwnd, IDC_EDIT_BONJOUR_SERVICE_NAME));
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
	if (!m_BonjourEnabledCheckBox.isChecked())
		return true;
	StringStorage name;
	m_BonjourServiceNameTextBox.getText(&name);
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
	ConfigDialog *configDialog = (ConfigDialog *)m_parent;
}

void BonjourConfigDialog::apply()
{
	/*StringStorage qtStringStorage;
	m_queryTimeout.getText(&qtStringStorage);

	int timeout = 0;
	StringParser::parseInt(qtStringStorage.getString(), &timeout);

	AutoLock al(m_config);

	m_config->allowLoopbackConnections(m_allowLoopbackConnections.isChecked());
	m_config->acceptOnlyLoopbackConnections(m_onlyLoopbackConnections.isChecked());
	m_config->setDefaultActionToAccept(m_defaultActionAccept.isChecked());
	m_config->setQueryTimeout(timeout);*/
}
