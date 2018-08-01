/**
 * @file save_data.cpp
 * @brief Save data source file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <fstream>
#include "save_data.hpp"
#include "util.hpp"

/** Windows */
#include <combaseapi.h>
#include <winerror.h>
#include <commctrl.h>
#include <atlbase.h>
#include <shlobj.h>

namespace
{
	/**
	 * Get a list of desktop icons.
	 * @param Desktop folder view.
	 * @param Desktop shell folder.
	 * @return Desktop icons.
	 */
	std::vector<ds::DesktopIcon> get_desktop_icons(CComPtr<IFolderView2>& view, CComPtr<IShellFolder>& shell)
	{
		// Get the number of icons
		int icon_count = -1;
		view->ItemCount(SVGIO_ALLVIEW, &icon_count);
		if (icon_count == -1) throw std::runtime_error("Unable to get the number of icons.");

		// List of icons
		std::vector<ds::DesktopIcon> icons(static_cast<size_t>(icon_count));

		// Ask for an item enumeration object
		CComPtr<IEnumIDList> item_enum = nullptr;
		view->Items(SVGIO_ALLVIEW, IID_PPV_ARGS(&item_enum));
		if (item_enum == nullptr) throw std::runtime_error("Unable to get item enumerator.");

		// Loop over every item
		size_t i = 0;
		for (CComHeapPtr<ITEMID_CHILD> item; item_enum->Next(1, &item, nullptr) == S_OK; item.Free())
		{
			// Get the icon's name
			STRRET str = {};
			shell->GetDisplayNameOf(item, SHGDN_NORMAL, &str);
			CComHeapPtr<char> name = {};
			StrRetToStr(&str, item, &name);

			// Get the icon's location
			POINT pt = {};
			view->GetItemPosition(item, &pt);

			// Store icon data
			icons[i].name = name;
			icons[i].point = pt;

			++i;
		}

		return icons;
	}
}

namespace ds
{
	SavedDesktop::SavedDesktop(const std::string& name, const std::string& path) :
		m_name(name), 
		m_path(path)
	{
		// Make sure the folder exists
		CreateDirectory(path.c_str(), NULL);

		// Make sure the icon folder exists
		const std::string icon_folder = path + "\\icons";
		CreateDirectory(icon_folder.c_str(), NULL);

		// Check if the locations file exists
		std::string loc_file = path + "\\locations.json";
		if (PathFileExists(loc_file.c_str()) == FALSE)
		{
			// Create the file
			std::ofstream stream(loc_file);
			stream << "{ \"icons\" : [] }";
		}
	}

	void SavedDesktop::save()
	{
		// Get a folder view for the desktop
		CComPtr<IFolderView2> desktop_view = nullptr;
		ds::find_desktop_folder_view(IID_PPV_ARGS(&desktop_view));
		if (desktop_view == nullptr) throw std::runtime_error("Unable to locate a desktop view.");

		// Ask for the folder aswell
		CComPtr<IShellFolder> desktop_folder = nullptr;
		desktop_view->GetFolder(IID_PPV_ARGS(&desktop_folder));
		if (desktop_folder == nullptr) throw std::runtime_error("Unable to locate a desktop folder.");

		// Icons folder
		const std::string icons_path = m_path + "\\icons";

		// Create the folders if needed
		CreateDirectory(m_path.c_str(), NULL);
		CreateDirectory(icons_path.c_str(), NULL);

		// Save information about every icon
		json icon_info = {};
		const auto icons = get_desktop_icons(desktop_view, desktop_folder);
		for (size_t i = 0; i < icons.size(); ++i)
		{
			// Save icon name and position
			icon_info["icons"][i]["name"] = icons[i].name;
			icon_info["icons"][i]["location"][0] = icons[i].point.x;
			icon_info["icons"][i]["location"][1] = icons[i].point.y;
		}

		// Save the icons
		const std::string desktop_path = get_desktop_path();
		const auto names = get_desktop_file_names();
		for (const auto& icon_name : names)
		{
			// Path to icon
			const std::string icon_path = desktop_path + "\\" + icon_name;

			// New icon path
			const std::string new_icon_path = icons_path + "\\" + icon_name;

			// Move the icon
			MoveFileEx(icon_path.c_str(), new_icon_path.c_str(), MOVEFILE_WRITE_THROUGH);
		}

		// Force the desktop to update
		// SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);

		// Force the desktop to update
		SendMessage(GetDesktopWindow(), WM_KEYDOWN, VK_F5, 0);

		// Wait until the only icon left is the recycle bin
		int new_icon_count = static_cast<int>(icons.size());
		while (new_icon_count >= icons.size())
			desktop_view->ItemCount(SVGIO_ALLVIEW, &new_icon_count);

		// Save 
		std::ofstream icon_info_stream(m_path + "\\locations.json");
		icon_info_stream << icon_info.dump(4);
	}

	void SavedDesktop::load()
	{
		// Get a folder view for the desktop
		CComPtr<IFolderView2> desktop_view = nullptr;
		ds::find_desktop_folder_view(IID_PPV_ARGS(&desktop_view));
		if (desktop_view == nullptr) throw std::runtime_error("Unable to locate a desktop view.");

		// Ask for the folder aswell
		CComPtr<IShellFolder> desktop_folder = nullptr;
		desktop_view->GetFolder(IID_PPV_ARGS(&desktop_folder));
		if (desktop_folder == nullptr) throw std::runtime_error("Unable to locate a desktop folder.");
		
		// Icons folder
		const std::string icons_path = m_path + "\\icons";

		// Get path to desktop
		const std::string desktop_path = get_desktop_path();

		// Read the desktop icons
		std::ifstream icon_stream(m_path + "\\locations.json");
		json icon_info = {};
		icon_info << icon_stream;
		icon_stream.close();

		// List of icon names
		std::vector<std::string> names = {};

		WIN32_FIND_DATA ffd = {};
		HANDLE file = FindFirstFile((icons_path + "\\*").c_str(), &ffd);
		do
		{
			// File name
			const std::string file_name = std::string(ffd.cFileName);

			// Path to icon
			const std::string icon_path = icons_path + "\\" + file_name;

			// New icon path
			const std::string new_icon_path = desktop_path + "\\" + file_name;

			// Move the icon
			MoveFileEx(icon_path.c_str(), new_icon_path.c_str(), MOVEFILE_WRITE_THROUGH);

		} while (FindNextFile(file, &ffd) != FALSE);

		// TODO: Hide the icons

		// Wait until the icons update
		int new_icon_count = 0;
		while (new_icon_count < icon_info["icons"].size())
		{
			// Get icon count
			desktop_view->ItemCount(SVGIO_ALLVIEW, &new_icon_count);
		}

		// Force the desktop to update
		SendMessage(GetDesktopWindow(), WM_KEYDOWN, VK_F5, 0);

		// Disable alignment to grid
		desktop_view->SetCurrentFolderFlags(FWF_AUTOARRANGE | FWF_SNAPTOGRID, 0);

		// Ask for an item enumeration object
		CComPtr<IEnumIDList> item_enum = nullptr;
		desktop_view->Items(SVGIO_ALLVIEW, IID_PPV_ARGS(&item_enum));
		if (item_enum == nullptr) throw std::runtime_error("Unable to get item enumerator.");

		// Move the icons back
		size_t i = 0;
		for (CComHeapPtr<ITEMID_CHILD> item; item_enum->Next(1, &item, nullptr) == S_OK; item.Free())
		{
			// Get the icon's name
			STRRET str = {};
			desktop_folder->GetDisplayNameOf(item, SHGDN_NORMAL, &str);
			CComHeapPtr<char> name = {};
			StrRetToStr(&str, item, &name);

			// Loop over every one of our icons
			for (size_t j = 0; j < icon_info["icons"].size(); ++j)
			{
				const auto& icon = icon_info["icons"][j];

				// If the names are the same, we found our icon
				if (std::strcmp(icon["name"].get<std::string>().c_str(), name.m_pData) == 0)
				{
					// Update the icon position
					POINT p = {};
					p.x = icon["location"][0];
					p.y = icon["location"][1];

					PCITEMID_CHILD child_item[1] = { item };
					desktop_view->SelectAndPositionItems(1, child_item, &p, SVSI_POSITIONITEM);

					// Remove this icon so we don't iterate over it again
					icon_info["icons"].erase(j);
					break;
				}
			}
			++i;
		}

		// Force the desktop to update
		// SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);

		// Force the desktop to update
		// SendMessage(GetDesktopWindow(), WM_KEYDOWN, VK_F5, 0);

		// TODO: Show the icons

		// Enable alignment to grid
		desktop_view->SetCurrentFolderFlags(FWF_SNAPTOGRID, FWF_SNAPTOGRID);
	}

	SaveData::SaveData(const std::string& path) : m_path(path), m_desktops({}), m_active_desktop("")
	{
		// Read the saved data
		std::ifstream stream(path + "\\saves.json");
		json j = {};
		j << stream;

		// Load every desktop
		for (const auto& desktop : j["saves"])
		{
			const std::string save_name = desktop.get<std::string>();
			m_desktops.push_back(SavedDesktop(save_name, path + "\\saves\\" + save_name));
		}

		// Get the active desktop
		m_active_desktop = j["active_desktop"].get<std::string>();
	}

	void SaveData::save()
	{
		// Update the saves file
		json save_data = {};

		// Add saves
		for (size_t i = 0; i < m_desktops.size(); ++i)
			save_data["saves"].push_back(m_desktops[i].get_name());

		// Add the new save name
		save_data["active_desktop"] = m_active_desktop;

		// Write new file
		std::ofstream save_stream(m_path + "\\saves.json");
		save_stream << save_data.dump(4);
		save_stream.close();
	}

	NewDesktopResult SaveData::new_desktop(const std::string& name)
	{
		// Make sure a desktop doesn't already exist with that name
		for (const auto& desktop : m_desktops)
			if (desktop.get_name() == name)
				return NewDesktopResult::NameTaken;

		// Save the active desktop
		try
		{
			// Get the active desktop
			SavedDesktop& active_desktop = get_active_desktop();
			active_desktop.save();
		}
		catch (...)
		{ return NewDesktopResult::ActiveDesktopInvalid; }

		// Add the new desktop
		m_desktops.push_back(SavedDesktop(name, m_path + "\\saves\\" + name));

		// Update the active desktop
		m_active_desktop = name;

		// Save the state
		save();

		return NewDesktopResult::Success;
	}

	LoadDesktopResult SaveData::load_desktop(const std::string& name)
	{
		// Get the desktop we want to load
		SavedDesktop* desktop = nullptr;
		try
		{ desktop = &get_save(name); }
		catch(...)
		{ return LoadDesktopResult::InvalidSaveName; }

		// Can't load the active desktop
		if(name == m_active_desktop)
		{ return LoadDesktopResult::CantLoadActiveDesktop; }

		// Save the active desktop
		try
		{
			// Get the active desktop
			SavedDesktop& active_desktop = get_active_desktop();
			active_desktop.save();
		}
		catch (...)
		{ return LoadDesktopResult::ActiveDesktopInvalid;}

		// Load the desktop
		desktop->load();

		// Update the active desktop
		m_active_desktop = name;

		// Save the state
		save();

		return LoadDesktopResult::Success;
	}
}