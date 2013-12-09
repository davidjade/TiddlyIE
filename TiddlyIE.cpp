// TiddlyIE.cpp : Implementation of CTiddlyIE

#include "stdafx.h"

#include "atlpath.h"
#include "atlfile.h"

#include "Iepmapi.h"

#include "TiddlyIE.h"

// extension property name
#define EXTENSION_PROPERTY_NAME L"TiddlyIE"

// content validation tokens
#define DOCTYPE L"<!doctype html>"
#define MOTW L"<!-- saved from url=(0021)http://tiddlywiki.com -->\r"
#define HTML_ELEMENT L"<html>"

// number of lines of content to search for validation tokens
#define LINES_TO_SEARCH 10


// CTiddlyIE

STDMETHODIMP CTiddlyIE::SetSite(IUnknown* pUnkSite)
{
	if (pUnkSite != NULL)
	{
		// Cache the pointer to IWebBrowser2.
		HRESULT hr = pUnkSite->QueryInterface(IID_IWebBrowser2, (void **)&m_spWebBrowser);
		if (SUCCEEDED(hr))
		{
			// Register to sink events from DWebBrowserEvents2.
			hr = DispEventAdvise(m_spWebBrowser);
			if (SUCCEEDED(hr))
			{
				m_fAdvised = TRUE;
			}
		}
	}
	else
	{
		// Unregister event sink.
		if (m_fAdvised)
		{
			DispEventUnadvise(m_spWebBrowser);
			m_fAdvised = FALSE;
		}

		// Release cached pointers and other resources here.
		m_spWebBrowser.Release();
	}

	// Return the base class implementation
	return IObjectWithSiteImpl<CTiddlyIE>::SetSite(pUnkSite);
}

///
/// OnDocumentComplete - attach our custom object
///
void STDMETHODCALLTYPE CTiddlyIE::OnDocumentComplete(IDispatch *pDisp, VARIANT *pvarURL)
{
	// attach the extension COM object
	AddCustomObject((IDispatch*)this, EXTENSION_PROPERTY_NAME);
}

///
/// OnDownloadComplete - also have to check here to trap F5/Refresh
///
void STDMETHODCALLTYPE CTiddlyIE::OnDownloadComplete()
{
	// attach the extension COM object
	AddCustomObject((IDispatch*)this, EXTENSION_PROPERTY_NAME);
}


///
/// AddCustomObject - Adds a custom COM object to the DOM window object as a named property
///
/// Note: only adds it if necessary and protocol is "file:"
///
HRESULT CTiddlyIE::AddCustomObject(IDispatch *custObj, TCHAR* name)
{
	// spBrowser is of type IHTMLWindow2
	CComPtr<IDispatch> spDoc;
	HRESULT hr = m_spWebBrowser->get_Document(&spDoc);
	if (FAILED(hr))
	{
		return hr;
	}

	CComQIPtr<IHTMLDocument2> spHTMLDoc(spDoc);
	if (!spHTMLDoc)
	{
		return E_NOINTERFACE;
	}

	// only for local documents
	CComPtr<IHTMLLocation> spLocation;
	hr = spHTMLDoc->get_location(&spLocation);
	if (SUCCEEDED(hr))
	{
		CComBSTR protocol;
		hr = spLocation->get_protocol(&protocol);
		if (FAILED(hr))
		{
			return hr;
		}
		else if (StrCmpIW(protocol, L"file:") != 0)
		{
			return S_OK;
		}
	}

	CComPtr<IHTMLWindow2> spWindow;
	hr = spHTMLDoc->get_parentWindow(&spWindow);
	if (FAILED(hr))
	{
		return hr;
	}

	CComQIPtr<IDispatchEx> spDispatchEx(spWindow);
	if (!spDispatchEx)
	{
		return E_NOINTERFACE;
	}

	// check for the named property so we only create it once
	DISPID dispid;
	hr = spDispatchEx->GetDispID(CComBSTR(name), fdexNameCaseSensitive, &dispid);
	if (FAILED(hr))
	{
		// create the named property if it does not exist
		hr = spDispatchEx->GetDispID(CComBSTR(name), fdexNameEnsure, &dispid);
		if (FAILED(hr)) 
		{
			return hr;
		}

		// now get the named property
		hr = spDispatchEx->GetDispID(CComBSTR(name), fdexNameCaseSensitive, &dispid);
		if (FAILED(hr)) 
		{
			return hr;
		}

		// attach the COM object to the property
		CComVariant var((IDispatch*)this);

		DISPID putid = DISPID_PROPERTYPUT;
		DISPPARAMS params;
		params.cArgs = 1;
		params.rgvarg = &var;
		params.cNamedArgs = 1;
		params.rgdispidNamedArgs = &putid;;

		hr = spDispatchEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &params, NULL, NULL, NULL);

		if (FAILED(hr)) 
		{
			return hr;
		}
	}

	return S_OK;
}


STDMETHODIMP CTiddlyIE::save(BSTR bstrFilespec, BSTR bstrContent)
{
	if (bstrFilespec == NULL || bstrContent == NULL)
	{
		return E_INVALIDARG;
	}

	// search for required content within first 10 lines or until we find the <html> element
	CStringW strContent = bstrContent;
	int iLineCount = 0;
	int iStart = 0;
	bool bFoundDoctype = false;
	bool bFoundMOTW = false;
	while (iStart != -1 && iLineCount++ < LINES_TO_SEARCH)
	{
		CStringW strLine = strContent.Tokenize(L"\n", iStart).TrimLeft().MakeLower();

		if (strLine.Find(HTML_ELEMENT) != -1) break;

		if (strLine.Find(DOCTYPE) != -1) bFoundDoctype = true;
		if (strLine.Find(MOTW) != -1) bFoundMOTW = true;

		if (bFoundDoctype && bFoundMOTW) break;
	}

	if (!bFoundDoctype || !bFoundMOTW)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	HWND hwnd;
	HRESULT hr = m_spWebBrowser->get_HWND((LONG_PTR*)&hwnd);
	if (FAILED(hr))
	{
		return hr;
	}

	// unescape filespec
	WCHAR pwszFilespec[MAX_PATH * sizeof(WCHAR)] = { 0 };
	DWORD dw = MAX_PATH * sizeof(WCHAR);
	hr = UrlUnescapeW(bstrFilespec, pwszFilespec, &dw, 0);
	if (SUCCEEDED(hr))
	{
		// crack filespec
		CPathW path = pwszFilespec;
		int fileidx = path.FindFileName();
		if (fileidx == -1)
		{
			return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);	// no file name specified
		}

		CStringW filename = path.m_strPath.Mid(fileidx);
		path.RemoveFileSpec();

		// Get a filespec from the user (must use IE user consent API)
		HANDLE hState;
		LPWSTR pwszSelectedFilename = NULL;
		const DWORD dwSaveFlags = OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		hr = IEShowSaveFileDialog(hwnd, filename, path,
			L"TiddleyWiki Files|*.html|", L"html", 1, 
			dwSaveFlags, &pwszSelectedFilename, &hState);

		// did user cancel?
		if (S_OK != hr)
			return hr;

		CPath SelectedFilespec = pwszSelectedFilename;
		CoTaskMemFree(pwszSelectedFilename);

		// safety check - enforce .html extension
		if (!SelectedFilespec.MatchSpec(L"*.html"))
		{
			// we can't actually change the path so all we can do is fail
			return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
		}

		// Get the path to the IE cache dir, which is a dir that we're allowed
		// to write to in protected mode
		LPWSTR pwszCacheDir = NULL;
		hr = IEGetWriteableFolderPath(FOLDERID_InternetCache, &pwszCacheDir);
		if (SUCCEEDED(hr))
		{
			// Get a temp file name in that dir
			TCHAR szTempFile[MAX_PATH] = { 0 };
			GetTempFileName(CW2CT(pwszCacheDir), _T("tw"), 0, szTempFile);
			CoTaskMemFree(pwszCacheDir);

			// Write our data to that temp file
			CAtlFile file;
			hr = file.Create(szTempFile, GENERIC_WRITE, 0, CREATE_ALWAYS);
			if (SUCCEEDED(hr))
			{
				// UTF-8 encode content
				int cNeeded = WideCharToMultiByte(CP_UTF8, 0, bstrContent, -1, NULL, 0, NULL, NULL);
				CStringA strContent;
				LPSTR lpstrBuffer = strContent.GetBufferSetLength(cNeeded);
				cNeeded = WideCharToMultiByte(CP_UTF8, 0, bstrContent, -1, lpstrBuffer, cNeeded, NULL, NULL);
				strContent.ReleaseBuffer();
				if(cNeeded == 0)
				{
					// UTF-8 conversion failed
					return HRESULT_FROM_WIN32(GetLastError());
				}

				hr = file.Write(strContent, strContent.GetLength());
				file.Close();

				if (SUCCEEDED(hr))
				{
					// If we wrote the file successfully, have IE save that data to
					// the path that the user chose
					hr = IESaveFile(hState, T2CW(szTempFile));

					// Clean up our temp file.
					DeleteFile(szTempFile);
				}
				else
				{
					// We couldn't complete the save operation, so cancel it
					IECancelSaveFile(hState);
				}
			}
		}
	}

	return hr;
}
