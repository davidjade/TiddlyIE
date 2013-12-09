// TiddlyIE.h : Declaration of the CTiddlyIE

#pragma once
#include "resource.h"       // main symbols

#include <shlguid.h>     // IID_IWebBrowser2, DIID_DWebBrowserEvents2, etc.
#include <exdispid.h> // DISPID_DOCUMENTCOMPLETE, etc.

#include "TiddlyIEBHO_i.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CTiddlyIE

class ATL_NO_VTABLE CTiddlyIE :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTiddlyIE, &CLSID_TiddlyIE>,
	public IObjectWithSiteImpl<CTiddlyIE>,
	public IDispatchImpl<ITiddlyIE, &IID_ITiddlyIE, &LIBID_TiddlyIEBHOLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispEventImpl<1, CTiddlyIE, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>
{
	public:
		CTiddlyIE()
		{
		}

	private:
		CComPtr<IWebBrowser2>  m_spWebBrowser;
		BOOL m_fAdvised;

		HRESULT CTiddlyIE::AddCustomObject(IDispatch *custObj, TCHAR* name);

	public:
		DECLARE_REGISTRY_RESOURCEID(IDR_SAVER2)

		DECLARE_NOT_AGGREGATABLE(CTiddlyIE)

		BEGIN_COM_MAP(CTiddlyIE)
			COM_INTERFACE_ENTRY(ITiddlyIE)
			COM_INTERFACE_ENTRY(IDispatch)
			COM_INTERFACE_ENTRY(IObjectWithSite)
		END_COM_MAP()

		STDMETHOD(SetSite)(IUnknown *pUnkSite);

		DECLARE_PROTECT_FINAL_CONSTRUCT()

		HRESULT FinalConstruct()
		{
			return S_OK;
		}

		void FinalRelease()
		{
		}

	public:

		BEGIN_SINK_MAP(CTiddlyIE)
			SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
			SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOWNLOADCOMPLETE, OnDownloadComplete)
		END_SINK_MAP()

		// DWebBrowserEvents2
		void STDMETHODCALLTYPE OnDocumentComplete(IDispatch *pDisp, VARIANT *pvarURL);
		void STDMETHODCALLTYPE OnDownloadComplete();


		STDMETHOD(save)(BSTR filespec, BSTR content);
};

OBJECT_ENTRY_AUTO(__uuidof(TiddlyIE), CTiddlyIE)
