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
	m_desktops.setWindow(GetDlgItem(dialogHwnd, IDC_COMBO_MPEG_STREAMER_MONOTORS));
	m_windows.setWindow(GetDlgItem(dialogHwnd, IDC_COMBO_MPEG_STREAMER_WINDOWS));
}

BOOL MpegStreamerConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
	switch (controlID)
	{
	case IDC_MPEG_STREAMER_ENABLED:
	case IDC_MPEG_STREAMER_TURN_OFF_RFB_VIDEO:
	case IDC_MPEG_STREAMER_HIDE_WINDOW:
		if (notificationID == BN_CLICKED)
			onMpegStreamerEnabledClick();
		break;
	case IDC_MPEG_STREAMER_DESTINATION_PORT:
	case IDC_MPEG_STREAMER_FRAMERATE:
	case IDC_MPEG_STREAMER_START_DELAY:
		if (notificationID == EN_UPDATE)
			onMpegStreamerChange();
		break;
	case IDC_COMBO_MPEG_STREAMER_MONOTORS:
	case IDC_COMBO_MPEG_STREAMER_WINDOWS:
		if (notificationID == BN_CLICKED)
			onMpegStreamerChange();
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

void MpegStreamerConfigDialog::onMpegStreamerChange()
{
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

bool MpegStreamerConfigDialog::validateInput()
{
	if (!m_enableMpegStreamer.isChecked())
		return true;

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

	set_monitors();

}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX* mi = new MONITORINFOEX();
	mi->cbSize = sizeof(MONITORINFOEX);
	if (!GetMonitorInfo(hMonitor, mi))
		return TRUE;// continue enumerating
	//for (MpegStreamerConfigDialog::ScreenList::iterator i = MpegStreamerConfigDialog::Screens.begin(); i != MpegStreamerConfigDialog::Screens.end(); i++)
	//{
	//	MpegStreamerConfigDialog::Screen* s = (*i);
	//	/*if (!wmemcmp(s->DeviceName, mi->szDevice, wcslen(s->DeviceName)))
	//	{
	//		s->x = mi->rcMonitor.left;
	//		s->y = mi->rcMonitor.top;
	//		s->width = mi->rcMonitor.right - mi->rcMonitor.left;
	//		s->height = mi->rcMonitor.bottom - mi->rcMonitor.top;
	//		return FALSE;//stop ennumerating
	//	}*/
	//}

	MpegStreamerConfigDialog::Screen* s = new MpegStreamerConfigDialog::Screen();
	s->x = mi->rcMonitor.left;
	s->y = mi->rcMonitor.top;
	s->width = mi->rcMonitor.right - mi->rcMonitor.left;
	s->height = mi->rcMonitor.bottom - mi->rcMonitor.top;
	wcsncpy(s->DeviceName, mi->szDevice, sizeof(s->DeviceName) / sizeof(WCHAR));
	/*s->x = lprcMonitor->left;
	s->y = lprcMonitor->top;
	s->width = lprcMonitor->right - lprcMonitor->left;
	s->height = lprcMonitor->bottom - lprcMonitor->top;*/
	MpegStreamerConfigDialog::Screens.push_back(s);
	return TRUE;// continue enumerating
}
MpegStreamerConfigDialog::ScreenList MpegStreamerConfigDialog::Screens = MpegStreamerConfigDialog::ScreenList();
void MpegStreamerConfigDialog::set_monitors() 
{
	int numberOfScreens = GetSystemMetrics(SM_CMONITORS);
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	Screens.clear();

	//getting dimensions of the monitors
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

	//getting names of real monitors
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(dd);
	//for (int i = 0; EnumDisplayDevices(NULL, i, &dd, 0); i++)
	for (ScreenList::iterator i = Screens.begin(); i != Screens.end(); i++)
	{
		//Screen* s = new Screen();
		Screen* s = (*i);
		EnumDisplayDevices(s->DeviceName, 0, &dd, 0);
		wcsncpy(s->DeviceName, dd.DeviceName, sizeof(s->DeviceName) / sizeof(WCHAR));
		wcsncpy(s->DeviceString, dd.DeviceString, sizeof(s->DeviceString) / sizeof(WCHAR));
		//MpegStreamerConfigDialog::Screens.push_back(s);
	}

	//fill dropdown list
	StringStorage ss;
	for (ScreenList::iterator i = Screens.begin(); i != Screens.end(); i++)
	{
		Screen* s = (*i);
		ss.format(_T("%d,%d,%d,%d-%s"), s->x, s->y, s->width, s->height, s->DeviceString);
		m_desktops.addItem(ss.getString(), s->DeviceName);
	}

	//select
	StringStorage dn;
	m_config->getMpegStreamerCapturedDesktopDeviceName(&dn);
	for (int i = m_desktops.getItemsCount() - 1; i >= 0; i--)
	{
		WCHAR* ws = (WCHAR*)m_desktops.getItemData(i);
		StringStorage ss;
#ifdef UNICODE
		//TCHAR == WCHAR
		ss = StringStorage(ws);
#else
		//TCHAR == char	
		TO BE IMPLEMENTED!
#endif
		if (ss.isEqualTo(&dn))
		{
			m_desktops.setSelectedItem(i);
			break;
		}
	}
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

	if (m_desktops.getSelectedItemIndex() >= 0)
	{
		WCHAR* ws = (WCHAR*)m_desktops.getItemData(m_desktops.getSelectedItemIndex());
#ifdef UNICODE
		//TCHAR == WCHAR
		ss = StringStorage(ws);
#else
		//TCHAR == char	
		TO BE IMPLEMENTED!
#endif
	}
	else
		ss.format(_T(""));
	m_config->setMpegStreamerCapturedDesktopDeviceName(ss.getString());

	//MessageBox(m_ctrlThis.getWindow(),
	//	ss.getString(),
	//	_T("222"), MB_ICONSTOP | MB_OK);
}
