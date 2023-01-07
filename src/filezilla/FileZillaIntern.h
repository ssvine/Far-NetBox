
#pragma once

#include <nbsystem.h>

class TFileZillaIntf;

class TFileZillaIntern // : public TObject
{
CUSTOM_MEM_ALLOCATION_IMPL
NB_DISABLE_COPY(TFileZillaIntern)
public:
  explicit TFileZillaIntern(TFileZillaIntf * AOwner) noexcept;

  bool FZPostMessage(WPARAM wParam, LPARAM lParam) const;
  CString GetOption(int OptionID) const;
  int GetOptionVal(int OptionID) const;

  inline const TFileZillaIntf * GetOwner() const { return FOwner; }

  int GetDebugLevel() const;
  void SetDebugLevel(int DebugLevel);

protected:
  TFileZillaIntf * FOwner{nullptr};
  int FDebugLevel{0};
};

