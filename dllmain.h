// dllmain.h : Declaration of module class.

class CTiddlyIEBHOModule : public ATL::CAtlDllModuleT< CTiddlyIEBHOModule >
{
public :
	DECLARE_LIBID(LIBID_TiddlyIEBHOLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SAVER2BHO, "{864465B5-585A-458F-8800-021DEB2677B7}")
};

extern class CTiddlyIEBHOModule _AtlModule;
