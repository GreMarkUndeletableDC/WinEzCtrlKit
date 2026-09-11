#pragma once

#if !ECK_OPT_NO_AUTO_ADD_LIB
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Advpack.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Ncrypt.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "Msi.lib")
#pragma comment(lib, "Taskschd.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Strmiids.lib")

#ifndef ECK_OPT_CRT_DLL
#  ifdef _DEBUG
#    define ECK_OPT_CRT_DLL 1
#  else
#    define ECK_OPT_CRT_DLL 0
#  endif
#endif

#endif// !ECK_OPT_NO_AUTO_ADD_LIB