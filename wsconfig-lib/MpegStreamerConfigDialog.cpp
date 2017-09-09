//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "MpegStreamerConfigDialog.h"
#include "ConfigDialog.h"
#include "tvnserver/resource.h"
#include "tvnserver-app/BonjourService.h"
#include "CommonInputValidation.h"

MpegStreamerConfigDialog::MpegStreamerConfigDialog()
: BaseDialog(IDD_CONFIG_BONJOUR_PAGE), m_parent(NULL)
{
}

MpegStreamerConfigDialog::~MpegStreamerConfigDialog()
{
}

void MpegStreamerConfigDialog::setParentDialog(BaseDialog *dialog)
{
  m_parent = dialog;
}

void MpegStreamerConfigDialog::initControls()
{
	HWND dialogHwnd = m_ctrlThis.getWindow();

	m_enableBonjourService.setWindow(GetDlgItem(dialogHwnd, IDC_BONJOUR_ENABLED));
	m_useWindowsUserAsBonjourServiceName.setWindow(GetDlgItem(dialogHwnd, IDC_BONJOUR_USE_WINDOWS_USER_AS_SERVICE_NAME));
	m_BonjourServiceName.setWindow(GetDlgItem(dialogHwnd, IDC_BONJOUR_SERVICE_NAME));
	m_BonjourServicePort.setWindow(GetDlgItem(dialogHwnd, IDC_BONJOUR_SERVICE_PORT));
	m_BonjourServiceType.setWindow(GetDlgItem(dialogHwnd, IDC_BONJOUR_SERVICE_TYPE));
}

BOOL MpegStreamerConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
	switch (controlID)
	{
	case IDC_BONJOUR_ENABLED:
		if (notificationID == BN_CLICKED)
			onBonjourEnabledClick();
		break;
	case IDC_BONJOUR_USE_WINDOWS_USER_AS_SERVICE_NAME:
		if (notificationID == BN_CLICKED)
			onUseWindowsUserAsBonjourServiceNameClick();
		break;
	case IDC_BONJOUR_SERVICE_NAME:
	case IDC_BONJOUR_SERVICE_PORT:
	case IDC_BONJOUR_SERVICE_TYPE:
		if (notificationID == EN_UPDATE)
			onBonjourTextChange();
		break;
	}
	return TRUE;
}

BOOL MpegStreamerConfigDialog::onInitDialog()
{
	m_config = Configurator::getInstance()->getServerConfig();

	initControls();
	updateUI();

  return TRUE;
}

void MpegStreamerConfigDialog::onBonjourEnabledClick()
{
	m_useWindowsUserAsBonjourServiceName.setEnabled(m_enableBonjourService.isChecked());
	m_BonjourServiceName.setEnabled(m_enableBonjourService.isChecked() && !m_useWindowsUserAsBonjourServiceName.isChecked());
	m_BonjourServicePort.setEnabled(m_enableBonjourService.isChecked());
	m_BonjourServiceType.setEnabled(m_enableBonjourService.isChecked());
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

void MpegStreamerConfigDialog::onUseWindowsUserAsBonjourServiceNameClick()
{
	m_BonjourServiceName.setEnabled(!m_useWindowsUserAsBonjourServiceName.isChecked());
	if (m_useWindowsUserAsBonjourServiceName.isChecked())
	{
		StringStorage service_name;
		BonjourService::GetWindowsUserName(&service_name);
		m_BonjourServiceName.setText(service_name.getString());
	}
	((ConfigDialog *)m_parent)->updateApplyButtonState();	
}

void MpegStreamerConfigDialog::onBonjourTextChange()
{
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

bool MpegStreamerConfigDialog::validateInput()
{
	if (!m_enableBonjourService.isChecked())
		return true;

	if (!BonjourService::IsAvailable())
	{
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_BONJOUR_SERVICE_IS_NOT_AVAILABLE),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	StringStorage ss;
	m_BonjourServiceName.getText(&ss);
	if (ss.getLength() < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_BONJOUR_SERVICE_NAME_NOTIFICATION),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	m_BonjourServicePort.getText(&ss);
	if (ss.getLength() < 1 || _ttoi(ss.getString()) < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_BONJOUR_SERVICE_PORT_NOTIFICATION),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}
	if(!CommonInputValidation::validatePort(&m_BonjourServicePort))
		return false;

	m_BonjourServiceType.getText(&ss);
	if (ss.getLength() < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_BONJOUR_SERVICE_TYPE_NOTIFICATION),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	return true;
}

void MpegStreamerConfigDialog::updateUI()
{
	m_enableBonjourService.check(m_config->isBonjourServiceEnabled());

	m_useWindowsUserAsBonjourServiceName.setEnabled(m_enableBonjourService.isChecked());
	
	StringStorage ss;
	m_useWindowsUserAsBonjourServiceName.check(m_config->isWindowsUserAsBonjourServiceNameUsed());
	if (m_useWindowsUserAsBonjourServiceName.isChecked())
	{
		m_BonjourServiceName.setEnabled(false);
		StringStorage service_name;
		BonjourService::GetWindowsUserName(&service_name);
		m_BonjourServiceName.setText(service_name.getString());
	}
	else
	{
		m_BonjourServiceName.setEnabled(m_config->isBonjourServiceEnabled());
		m_config->getBonjourServiceName(&ss);
		m_BonjourServiceName.setText(ss.getString());
	}

	m_BonjourServicePort.setEnabled(m_enableBonjourService.isChecked());
	TCHAR ts[255];
	m_BonjourServicePort.setText(_itot(m_config->getBonjourServicePort(), ts, 10));

	m_BonjourServiceType.setEnabled(m_enableBonjourService.isChecked());
	m_config->getBonjourServiceType(&ss);
	m_BonjourServiceType.setText(ss.getString());
}

void MpegStreamerConfigDialog::apply()
{
	AutoLock al(m_config);
	m_config->enableBonjourService(m_enableBonjourService.isChecked());
	m_config->useWindowsUserAsBonjourServiceName(m_useWindowsUserAsBonjourServiceName.isChecked());
	StringStorage ss;
	m_BonjourServiceName.getText(&ss);
	m_config->setBonjourServiceName(ss.getString());
	m_BonjourServicePort.getText(&ss);
	m_config->setBonjourServicePort(_ttoi(ss.getString()));
	m_BonjourServiceType.getText(&ss);
	m_config->setBonjourServiceType(ss.getString());
}
