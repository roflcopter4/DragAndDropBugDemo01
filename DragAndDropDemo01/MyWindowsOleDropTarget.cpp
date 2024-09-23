#include "MyWindowsOleDropTarget.h"
#include <vector>

ND static std::vector<std::wstring> getFilePathsViaWin32Idlist(BYTE const *rawData);

/****************************************************************************************/

STDMETHODIMP MyWindowsOleDropTarget::QueryInterface(IID const &riid, void **ppvObject)
{
    if (!ppvObject)
        return E_POINTER;
    if (riid == __uuidof(IUnknown)) {
        *ppvObject = static_cast<IUnknown *>(static_cast<IDropTarget *>(this));
        AddRef();
        return S_OK;
    }
    if (riid == __uuidof(IDropTarget)) {
        *ppvObject = static_cast<IDropTarget *>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MyWindowsOleDropTarget::AddRef()
{
    return ++referenceCount_;
}

STDMETHODIMP_(ULONG) MyWindowsOleDropTarget::Release()
{
    ULONG referenceCount = --referenceCount_;
    if (referenceCount == 0)
        delete this;
    return referenceCount;
}

STDMETHODIMP MyWindowsOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, UU DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    if (IDropTargetHelper *dh = dropHelper())
        dh->DragEnter(hWnd_, pDataObj, reinterpret_cast<POINT *>(&pt), *pdwEffect);
    setDropDataObject(pDataObj);
    pDataObj->AddRef();
    return NOERROR;
}

STDMETHODIMP MyWindowsOleDropTarget::DragOver(UU DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    if (IDropTargetHelper *dh = dropHelper())
        dh->DragOver(reinterpret_cast<POINT *>(&pt), *pdwEffect);
    return NOERROR;
}

STDMETHODIMP MyWindowsOleDropTarget::DragLeave()
{
    if (IDropTargetHelper *dh = dropHelper())
        dh->DragLeave();
    releaseDropDataObject();
    return NOERROR;
}

STDMETHODIMP MyWindowsOleDropTarget::Drop(LPDATAOBJECT pDataObj, UU DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    if (IDropTargetHelper *dh = dropHelper())
        dh->Drop(pDataObj, reinterpret_cast<POINT *>(&pt), *pdwEffect);
    {
        auto hdropList = convertToMime(pDataObj);
        util::DumpFmt(L"Found {} files via CF_HDROP\n"sv, hdropList.size());
    }
    {
        auto data  = getData(CF_SHELLIDLIST, pDataObj);
        auto files = getFilePathsViaWin32Idlist(data.data());
        util::DumpFmt(L"Found {} files via CF_SHELLIDLIST\n"sv, files.size());
        for (size_t i = 0; i < files.size(); ++i)
            util::DumpFmt(L"File {:3d}: {}\n"sv, i, files[i]);
    }
    util::DumpToConsole(L"\n"sv);
    releaseDropDataObject();
    return NOERROR;
}

/*--------------------------------------------------------------------------------------*/

MyWindowsOleDropTarget::MyWindowsOleDropTarget(HWND hWnd)
    : hWnd_(hWnd),
      CF_SHELLIDLIST(::RegisterClipboardFormatW(L"Shell IDList Array")),
      CF_INETURL(::RegisterClipboardFormatW(L"UniformResourceLocator")),
      CF_INETURL_W(::RegisterClipboardFormatW(L"UniformResourceLocatorW"))
{}

MyWindowsOleDropTarget::~MyWindowsOleDropTarget()
{
    if (cachedDropTargetHelper_)
        cachedDropTargetHelper_->Release();
}

void MyWindowsOleDropTarget::releaseDropDataObject()
{
    if (dropDataObject_) {
        dropDataObject_->Release();
        dropDataObject_ = nullptr;
    }
}

IDropTargetHelper *MyWindowsOleDropTarget::dropHelper()
{
    if (!cachedDropTargetHelper_) {
        HRESULT res = ::CoCreateInstance(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER,
                                         IID_PPV_ARGS(&cachedDropTargetHelper_));
        if (FAILED(res))
            util::DumpErrorAndCrash(res);
    }
    return cachedDropTargetHelper_;
}

std::vector<BYTE> MyWindowsOleDropTarget::getData(UINT cf, IDataObject *pDataObj, int lindex)
{
    STGMEDIUM s;
    FORMATETC formatetc = {
        .cfFormat = static_cast<CLIPFORMAT>(cf),
        .ptd      = nullptr,
        .dwAspect = DVASPECT_CONTENT,
        .lindex   = lindex,
        .tymed    = TYMED_HGLOBAL,
    };
    std::vector<BYTE> data;
    if (pDataObj->GetData(&formatetc, &s) == S_OK) {
        LPCVOID val  = ::GlobalLock(s.hGlobal);
        SIZE_T  size = ::GlobalSize(s.hGlobal);
        data.resize(size);
        memcpy(data.data(), val, size);
        ::GlobalUnlock(s.hGlobal);
        ::ReleaseStgMedium(&s);
    }
    return data;
}

bool MyWindowsOleDropTarget::canGetData(UINT cf, IDataObject *pDataObj)
{
    FORMATETC formatetc = {CLIPFORMAT(cf), nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    return pDataObj->QueryGetData(&formatetc) == S_OK;
}

ND std::vector<std::wstring>
MyWindowsOleDropTarget::convertToMime(LPDATAOBJECT pDataObj) const
{
    if (canGetData(CF_HDROP, pDataObj)) {
        std::vector<std::wstring> urls;
        auto data = getData(CF_HDROP, pDataObj);
        if (data.empty()) {
            return {};
        }
        auto   hdrop = reinterpret_cast<DROPFILES const *>(data.data());
        size_t i     = 0;
        if (hdrop->fWide) {
            auto filesw = reinterpret_cast<wchar_t const *>(data.data() + hdrop->pFiles);
            while (filesw[i]) {
                auto fileurl = std::wstring(filesw + i);
                urls.emplace_back(fileurl);
                i += fileurl.length() + 1;
            }
        } else {
            auto files = reinterpret_cast<char const *>(data.data() + hdrop->pFiles);
            while (files[i]) {
                urls.emplace_back(util::NarrowToWide(files + i));
                i += strlen(files + i) + 1;
            }
        }
        if (!urls.empty())
            return urls;
    } else if (canGetData(CF_INETURL_W, pDataObj)) {
        auto data = getData(CF_INETURL_W, pDataObj);
        if (data.empty())
            return {};
        return {};
    } else if (canGetData(CF_INETURL, pDataObj)) {
        auto data = getData(CF_INETURL, pDataObj);
        if (data.empty())
            return {};
        return {util::NarrowToWide(reinterpret_cast<char const *>(data.data()))};
    }
    return {};
}

/****************************************************************************************/

static void dumpComError(wchar_t const *loc, HRESULT res)
{
    util::DumpFmt(L"COM error in {}: {}\n"sv, loc, util::GetErrorMessage(res));
}

ND static std::vector<std::wstring> getFilePathsViaWin32Idlist(BYTE const *rawData)
{
#define GetPIDLFolder(pida)  (reinterpret_cast<LPCITEMIDLIST>((reinterpret_cast<BYTE const *>(pida)) + (pida)->aoffset[0]))
#define GetPIDLItem(pida, i) (reinterpret_cast<LPCITEMIDLIST>((reinterpret_cast<BYTE const *>(pida)) + (pida)->aoffset[(i) + 1U]))

    auto idList = reinterpret_cast<::CIDA const *>(rawData);

    LPWSTR  str = nullptr;
    HRESULT res = ::SHGetNameFromIDList(GetPIDLFolder(idList), SIGDN_DESKTOPABSOLUTEPARSING, &str);
    if (FAILED(res)) {
        dumpComError(L"SHGetNameFromIDList", res);
        return {};
    }
    std::wstring rootDir = str;
    rootDir.push_back(L'\\');
    ::CoTaskMemFree(str);

    std::vector<std::wstring> ret;
    ret.reserve(idList->cidl);
    for (unsigned i = 0; i < idList->cidl; ++i) {
        str = nullptr;
        res = ::SHGetNameFromIDList(GetPIDLItem(idList, i), SIGDN_PARENTRELATIVE, &str);
        if (FAILED(res)) {
            dumpComError(L"SHGetNameFromIDList", res);
            continue;
        }
        std::wstring path = rootDir;
        path.append(str);
        ::CoTaskMemFree(str);
        ret.push_back(std::move(path));
    }
    return ret;

#undef GetPIDLFolder
#undef GetPIDLItem
}

