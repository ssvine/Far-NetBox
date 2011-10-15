//---------------------------------------------------------------------------
#include "stdafx.h"

#include "Common.h"
#include "CoreMain.h"
#include "FarConfiguration.h"
#include "WinSCPPlugin.h"
//---------------------------------------------------------------------------

TConfiguration * CreateConfiguration()
{
  return new TFarConfiguration(FarPlugin);
}
//---------------------------------------------------------------------------
void ShowExtendedException(const std::exception * E)
{
  assert(FarPlugin != NULL);
  TWinSCPPlugin * WinSCPPlugin = dynamic_cast<TWinSCPPlugin *>(FarPlugin);
  assert(WinSCPPlugin != NULL);
  WinSCPPlugin->ShowExtendedException(E);
}
std::wstring AppNameString()
{
  return L"WinSCP";
}

//---------------------------------------------------------------------------
std::wstring GetRegistryKey()
{
#ifndef NETBOX_DEBUG
  return L"Software\\Martin Prikryl\\WinSCP 2";
#else
  return L"Software\\Michael Lukashov\\TestNetBox";
#endif
}
//---------------------------------------------------------------------------
void Busy(bool /*Start*/)
{
  // nothing
}
//---------------------------------------------------------------------------
std::wstring SshVersionString()
{
  return FORMAT(L"WinSCP-FAR-release-%s", Configuration->GetVersion().c_str());
}

//---------------------------------------------------------------------------
int StartThread(void * SecurityAttributes, unsigned StackSize,
  const threadfunc_slot_type &ThreadFunc, void * Parameter, unsigned CreationFlags,
  unsigned & ThreadId)
{
  ::Error(SNotImplemented, 1002);
  // FIXME return BeginThread(SecurityAttributes, StackSize, ThreadFunc, Parameter,
    // CreationFlags, ThreadId);
  return 0;
}
//---------------------------------------------------------------------------
void CopyToClipboard(std::wstring Text)
{
  assert(FarPlugin != NULL);
  FarPlugin->FarCopyToClipboard(Text);
}
