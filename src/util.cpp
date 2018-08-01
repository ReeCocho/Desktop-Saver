/**
 * @file util.cpp
 * @brief Utilities source file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <shlobj.h>
#include <codecvt>
#include <atlbase.h>
#include "util.hpp"

namespace ds
{
	void find_desktop_folder_view(REFIID riid, void** ppv)
	{
		// Create a shell window object
		CComPtr<IShellWindows> shell_windows = nullptr;
		HRESULT res = shell_windows.CoCreateInstance(CLSID_ShellWindows);
		if (res != S_OK) throw std::runtime_error("Unable to find a shell window.");

		// Use the shell window object to locate the desktop
		CComVariant loc(CSIDL_DESKTOP);
		CComVariant dummy = {};
		long hwnd = NULL;
		CComPtr<IDispatch> dispatch = nullptr;
		shell_windows->FindWindowSW(&loc, &dummy, SWC_DESKTOP, &hwnd, SWFO_NEEDDISPATCH, &dispatch);
		if (dispatch == nullptr) throw std::runtime_error("Unable to locate the desktop.");

		// Ask for a browser object so we can do stuff to the desktop
		CComPtr<IShellBrowser> browser = nullptr;
		CComQIPtr<IServiceProvider>(dispatch)->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&browser));

		// Find the shell view
		CComPtr<IShellView> view = nullptr;
		browser->QueryActiveShellView(&view);
		if (view == nullptr) throw std::runtime_error("Unable to find a shell view.");

		// Ask for a IFolderView interface
		view->QueryInterface(riid, ppv);
	}

	bool compare_file_names(const std::string n1, const std::string n2)
	{
		// Do a regular comparison first
		if (n1 == n2) return true;

		// Otherwise we need to truncate one of them
		if (n1.size() > n2.size())
		{
			std::string nn1 = n1;
			nn1.resize(n2.size());
			return nn1 == n2;
		}
		else
		{
			std::string nn2 = n2;
			nn2.resize(n1.size());
			return nn2 == n1;
		}
	}

	std::string wide_to_reg(const std::wstring wstr)
	{
		using ct = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<ct, wchar_t> converter = {};
		return converter.to_bytes(wstr);
	}

	std::wstring reg_to_wide(const std::string str)
	{
		using ct = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<ct, wchar_t> converter = {};
		return converter.from_bytes(str);
	}

	std::string get_desktop_path()
	{
		// Get the path to the AppData folder
		PWSTR c_path = NULL;
		auto res = SHGetKnownFolderPath(FOLDERID_Desktop, KF_FLAG_DEFAULT, NULL, &c_path);

		if (res != S_OK)
		{
			// Free the C string
			CoTaskMemFree(c_path);
			throw std::runtime_error("Unable to locate the Desktop folder");
		}

		// Convert the path into a more C++ friendly format
		std::wstring path(c_path);

		// Free the C string
		CoTaskMemFree(c_path);

		return wide_to_reg(path);
	}

	std::vector<std::string> get_desktop_file_names()
	{
		// Get path to the desktop
		const std::string desktop = get_desktop_path() + "\\*";

		// List of files
		std::vector<std::string> files = {};

		// Found file data
		WIN32_FIND_DATA ffd = {};

		// Get the first file on the desktop
		HANDLE file = FindFirstFile(desktop.c_str(), &ffd);

		// Start adding files
		if (file != NULL)
		{
			// Loop until we can find no more files
			do
			{
				// We don't care about these files
				if (std::strcmp(ffd.cFileName, ".") == 0 || std::strcmp(ffd.cFileName, "..") == 0)
					continue;

				// Add the files name
				files.push_back(ffd.cFileName);

			} while (FindNextFile(file, &ffd) != NULL);
		}

		// Close the file
		FindClose(file);

		return files;
	}

	std::string get_desktop_saver_path()
	{
		// Get the path to the AppData folder
		PWSTR c_path = NULL;
		auto res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &c_path);

		if (res != S_OK)
		{
			// Free the C string
			CoTaskMemFree(c_path);
			throw std::runtime_error("Unable to locate the AppData folder");
		}

		// Convert the path into a more C++ friendly format
		std::wstring path(c_path);

		// Free the C string
		CoTaskMemFree(c_path);

		// Add the folder path
		path += L"\\Desktop-Saver";

		return wide_to_reg(path);
	}
}