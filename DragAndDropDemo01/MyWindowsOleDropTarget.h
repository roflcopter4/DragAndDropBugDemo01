#pragma once
#include "Common.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <memory>
#include <vector>

class MyWindowsOleDropTarget final : public IDropTarget
{
  public:
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    // IDropTarget methods
    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) override;
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) override;
    STDMETHOD(DragLeave)() override;
    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) override;

    explicit MyWindowsOleDropTarget(HWND hWnd);
    DELETE_COPY_MOVE_ROUTINES(MyWindowsOleDropTarget);

  protected:
    virtual ~MyWindowsOleDropTarget();

  public:
    ND auto dropDataObject() const { return dropDataObject_; }
    void    setDropDataObject(IDataObject *dataObject) { dropDataObject_ = dataObject; }
    void    releaseDropDataObject();

    ND IDropTargetHelper *dropHelper();
    ND std::vector<std::wstring> convertToMime(LPDATAOBJECT pDataObj) const;

  private:
    static std::vector<BYTE> getData(UINT cf, IDataObject *pDataObj, int lindex = -1);
    static bool canGetData(UINT cf, IDataObject *pDataObj);

    HWND hWnd_;
    UINT CF_SHELLIDLIST;
    UINT CF_INETURL;
    UINT CF_INETURL_W;

    std::atomic<ULONG> referenceCount_         = 1;
    IDataObject       *dropDataObject_         = nullptr;
    IDropTargetHelper *cachedDropTargetHelper_ = nullptr;
};


