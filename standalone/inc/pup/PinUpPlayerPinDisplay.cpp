#include "stdafx.h"
#include "PinUpPlayerPinDisplay.h"

PinUpPlayerPinDisplay::PinUpPlayerPinDisplay()
{
}

PinUpPlayerPinDisplay::~PinUpPlayerPinDisplay()
{
}

STDMETHODIMP PinUpPlayerPinDisplay::Init(LONG ScreenNum, BSTR RootDir) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::playlistadd(LONG ScreenNum, BSTR folder, LONG sort, LONG restSeconds) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::playlistplay(LONG ScreenNum, BSTR playlist) { return E_NOTIMPL; }

STDMETHODIMP PinUpPlayerPinDisplay::playlistplayex(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG forceplay)
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::play(LONG ScreenNum, BSTR playlist, BSTR playfilename) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::setWidth(LONG ScreenNum, LONG width) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::setHeight(LONG ScreenNum, LONG Height) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::setPosX(LONG ScreenNum, LONG Posx) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::setPosY(LONG ScreenNum, LONG PosY) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::setAspect(LONG ScreenNum, LONG aspectWide, LONG aspectHigh) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::setVolume(LONG ScreenNum, LONG vol) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::playpause(LONG ScreenNum) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::playresume(LONG ScreenNum) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::playstop(LONG ScreenNum) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::CloseApp() { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::get_isPlaying(LONG ScreenNum, LONG *Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::put_isPlaying(LONG ScreenNum, LONG Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::SetLength(LONG ScreenNum, LONG StopSecs) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::SetLoop(LONG ScreenNum, LONG LoopState) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::SetBackGround(LONG ScreenNum, LONG Mode) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::BlockPlay(LONG ScreenNum, LONG Mode) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::SetScreen(LONG ScreenNum) { return E_NOTIMPL; }

STDMETHODIMP PinUpPlayerPinDisplay::SetScreenEx(LONG ScreenNum, LONG xpos, LONG ypos, LONG swidth, LONG sheight, LONG popup) 
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_SN(LONG *Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::put_SN(LONG Value) { return E_NOTIMPL; }

STDMETHODIMP PinUpPlayerPinDisplay::B2SData(BSTR tIndex, int Value)
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_B2SFilter(BSTR *Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::put_B2SFilter(BSTR Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::Show(LONG ScreenNum) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::Hide(LONG ScreenNum) { return E_NOTIMPL; }

STDMETHODIMP PinUpPlayerPinDisplay::B2SInit(BSTR TName, BSTR RomName)
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SendMSG(BSTR cMSG) { return E_NOTIMPL; }

STDMETHODIMP PinUpPlayerPinDisplay::LabelNew(LONG ScreenNum, BSTR LabelName, BSTR FontName, LONG Size, LONG Color, LONG Angle, LONG xAlign, LONG yAlign, LONG xMargin, LONG yMargin, LONG PageNum, LONG Visible)
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelSet(LONG ScreenNum, BSTR LabelName, BSTR Caption, LONG Visible, BSTR Special)
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelSetEx() { return E_NOTIMPL; }

STDMETHODIMP PinUpPlayerPinDisplay::LabelShowPage(LONG ScreenNum, LONG PageNum, LONG Seconds, BSTR Special)
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelInit(LONG ScreenNum)
{
    return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_GetGame(BSTR *Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::put_GetGame(BSTR Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::get_GetRoot(BSTR *Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::put_GetRoot(BSTR Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::SoundAdd(BSTR sname, BSTR fname, LONG svol, double sX, double sy, BSTR SP) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::SoundPlay(BSTR sname) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::PuPSound(BSTR sname, LONG sX, LONG sy, LONG sz, LONG vol, BSTR SP) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::InitPuPMenu(LONG Param1) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::get_B2SDisplays(BSTR *Value) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::setVolumeCurrent(LONG ScreenNum, LONG vol) { return E_NOTIMPL; }
STDMETHODIMP PinUpPlayerPinDisplay::get_GameUpdate(BSTR GameTitle, LONG Func, LONG FuncData, BSTR Extra, LONG *Value) { return E_NOTIMPL; }
