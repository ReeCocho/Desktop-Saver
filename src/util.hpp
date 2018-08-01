#pragma once

/**
 * @file util.hpp
 * @brief Utilities header file.
 * @author Connor J. Bramham (ReeCocho)
 */

 /** Don't need extra includes */
#define WIN32_LEAN_AND_MEAN

/** Includes. */
#include <windows.h>
#include <string>
#include <vector>

namespace ds
{
	/**
	 * Desktop icon data.
	 */
	struct DesktopIcon
	{
		/** Name. */
		std::string name = "";

		/** Location. */
		POINT point = {};
	};

	/**
	 * Find the desktop folder view.
	 * @param Reference ID.
	 * @param Pointer to pointer of IFolderView.
	 */
	extern void find_desktop_folder_view(REFIID riid, void** ppv);

	/**
	 * Compare two file names to see if they are "equal."
	 * @param First name.
	 * @param Second name.
	 * @return if the two file names point to the same file.
	 */
	extern bool compare_file_names(const std::string n1, const std::string n2);

	/**
	 * Convert a wide string into a regular string.
	 * @param Wide string.
	 * @return Regular string.
	 */
	extern std::string wide_to_reg(const std::wstring wstr);

	/**
	 * Convert a regular string into a wide string.
	 * @param Regular string.
	 * @return Wide string.
	 */
	extern std::wstring reg_to_wide(const std::string str);

	/**
	 * Get the desktop path.
	 * @return Desktop path.
	 */
	extern std::string get_desktop_path();

	/**
	 * Get a list of files on the desktop.
	 * @return Files on the desktop.
	 */
	extern std::vector<std::string> get_desktop_file_names();

	/**
	 * Get the Desktop-Saver path.
	 * @return Desktop-Saver path.
	 */
	extern std::string get_desktop_saver_path();
}