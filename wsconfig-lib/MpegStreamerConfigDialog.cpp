//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "MpegStreamerConfigDialog.h"
#include "ConfigDialog.h"
#include "tvnserver/resource.h"
#include "CommonInputValidation.h"

MpegStreamerConfigDialog::MpegStreamerConfigDialog()
: BaseDialog(IDD_CONFIG_MPEG_STREAMER_PAGE), m_parent(NULL)
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

	m_enableMpegStreamer.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_ENABLED));
	m_MpegStreamerDestinationPort.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_DESTINATION_PORT));
	m_MpegStreamerFramerate.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_FRAMERATE));
	m_MpegStreamerDelayMss.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_START_DELAY));
	m_turnOffRfbVideo.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_TURN_OFF_RFB_VIDEO));
	m_hideStreamerWindow.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_HIDE_WINDOW));
}

BOOL MpegStreamerConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
	switch (controlID)
	{
	case IDC_MPEG_STREAMER_ENABLED:
		if (notificationID == BN_CLICKED)
			onMpegStreamerEnabledClick();
		break;
	case IDC_MPEG_STREAMER_DESTINATION_PORT:
	case IDC_MPEG_STREAMER_FRAMERATE:
	case IDC_MPEG_STREAMER_START_DELAY:
		if (notificationID == EN_UPDATE)
			onMpegStreamerTextChange();
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

void MpegStreamerConfigDialog::onMpegStreamerEnabledClick()
{
	m_MpegStreamerDestinationPort.setEnabled(m_enableMpegStreamer.isChecked());
	m_MpegStreamerFramerate.setEnabled(m_enableMpegStreamer.isChecked());
	m_MpegStreamerDelayMss.setEnabled(m_enableMpegStreamer.isChecked());
	m_turnOffRfbVideo.setEnabled(m_enableMpegStreamer.isChecked());
	m_hideStreamerWindow.setEnabled(m_enableMpegStreamer.isChecked());
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

void MpegStreamerConfigDialog::onMpegStreamerTextChange()
{
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

bool MpegStreamerConfigDialog::validateInput()
{
	if (!m_enableMpegStreamer.isChecked())
		return true;

	/*if (!BonjourService::IsAvailable())
	{
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_BONJOUR_SERVICE_IS_NOT_AVAILABLE),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}*/

	StringStorage ss;
	long i;
	m_MpegStreamerDestinationPort.getText(&ss);
	if (!CommonInputValidation::parseNumber(&ss, &i) || i < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_MPEG_STREAMER_PORT_ERROR),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}
	if (!CommonInputValidation::validatePort(&m_MpegStreamerDestinationPort))
		return false;

	m_MpegStreamerFramerate.getText(&ss);
	if (!CommonInputValidation::parseNumber(&ss, &i) || i < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_MPEG_STREAMER_FRAMERATE_ERROR),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	m_MpegStreamerDelayMss.getText(&ss);
	if (!CommonInputValidation::parseNumber(&ss, &i))
	{
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_MPEG_STREAMER_START_DELAY_ERROR),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	return true;
}

void MpegStreamerConfigDialog::updateUI()
{
	m_enableMpegStreamer.check(m_config->isMpegStreamerEnabled()); 
	MpegStreamerConfigDialog::onMpegStreamerEnabledClick();

	TCHAR ts[255];
	m_MpegStreamerDestinationPort.setText(_itot(m_config->getMpegStreamerDestinationPort(), ts, 10));

	m_MpegStreamerFramerate.setText(_itot(m_config->getMpegStreamerFramerate(), ts, 10));

	m_MpegStreamerDelayMss.setText(_itot(m_config->getMpegStreamerDelayMss(), ts, 10));

	m_turnOffRfbVideo.check(m_config->isMpegStreamerRfbVideoTunedOff());

	m_hideStreamerWindow.check(m_config->isMpegStreamerWindowHidden());
}

void MpegStreamerConfigDialog::apply()
{
	AutoLock al(m_config);

	m_config->enableMpegStreamer(m_enableMpegStreamer.isChecked());

	StringStorage ss;
	m_MpegStreamerDestinationPort.getText(&ss);
	m_config->setMpegStreamerDestinationPort(_ttoi(ss.getString()));

	m_MpegStreamerFramerate.getText(&ss);
	m_config->setMpegStreamerFramerate(_ttoi(ss.getString()));

	m_MpegStreamerDelayMss.getText(&ss);
	m_config->setMpegStreamerDelayMss(_ttoi(ss.getString()));

	m_config->turnOffMpegStreamerRfbVideo(m_turnOffRfbVideo.isChecked());

	m_config->hideMpegStreamerWindow(m_hideStreamerWindow.isChecked());
}
