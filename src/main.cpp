/** Desktop-Saver */
#include "util.hpp"
#include "save_data.hpp"

/** Windows */
#include <combaseapi.h>
#include <winerror.h>
#include <commctrl.h>
#include <atlbase.h>
#include <shlobj.h>

/** STL */
#include <iostream>
#include <fstream>

/**
 * Check's if the Desktop-Saver folder exists, and if not, it creates it and the necessary files.
 */
void check_data_folder()
{
	// Get the path to the AppData folder
	std::string path = ds::get_desktop_saver_path();

	// Create the directory if needed
	CreateDirectory(path.c_str(), NULL);

	// Check if the saves file exists
	std::string saves_file = path + "\\saves.json";
	if (PathFileExists(saves_file.c_str()) == FALSE)
	{
		// Create the file
		std::ofstream stream(saves_file);
		stream << "{ \"active_desktop\" : \"Default\", \"saves\" : [] }";
	}

	// Check if the saves folder exists
	const std::string saves_folder = path + "\\saves";
	CreateDirectory(saves_folder.c_str(), NULL);

	// Check the "Default" save folder
	const std::string default_save_folder = path + "\\saves\\Default";
	CreateDirectory(default_save_folder.c_str(), NULL);

	// Check the "Default" save icons folder
	const std::string default_save_icons = path + "\\saves\\Default\\icons";
	CreateDirectory(default_save_icons.c_str(), NULL);

	// Check the "Default" save locations file
	std::string default_save_loc = path + "\\saves\\Default\\locations.json";
	if (PathFileExists(default_save_loc.c_str()) == FALSE)
	{
		// Create the file
		std::ofstream stream(default_save_loc);
		stream << "{ \"icons\" : [] }";
	}
}

/**
 * Entry point.
 * @param Number of arguments passed.
 * @param Command line arguments.
 * @note argv[0] is the program name.
 */
int main(int argc, char* argv[])
{
	// Must take in atleast 2 args (First is the program name)
	if (argc < 2)
	{
		std::cerr << "ERROR: Missing arguments";
		return ERROR_BAD_ARGUMENTS;
	}

	// Initialize the COM library
	CoInitialize(NULL);
	
	// Initialize the Desktop-Saver folder if needed
	check_data_folder();
	
	// Get the desktop saver path
	const auto& path = ds::get_desktop_saver_path();
	
	// Create the saved data manager
	ds::SaveData save_data(path);
	
	// Get the operation
	const std::string operation = argv[1];
	
	// Read saved desktops
	if (operation == "-r")
	{
		// Loop over every save
		for (size_t i = 0; i < save_data.get_save_count(); ++i)
			// Print out the save name
			std::cout << save_data.get_save(i).get_name() << '\n';
	}
	// New desktop
	else if (operation == "-n")
	{
		// Must have a third argument
		if (argc < 3)
		{
			std::cerr << "ERROR: Missing save name.";
			return ERROR_BAD_ARGUMENTS;
		}
	
		// Get the save name
		const std::string save_name = argv[2];
		std::cout << "Saving current desktop with name \"" + save_name + "\"\n";
	
		// Save the desktop
		ds::NewDesktopResult result = save_data.new_desktop(save_name);
	
		switch (result)
		{
		case ds::NewDesktopResult::ActiveDesktopInvalid:
			std::cerr << "ERROR: The active desktop must be valid.";
			return 0;
	
		case ds::NewDesktopResult::NameTaken:
			std::cerr << "ERROR: Desktop with that name is already taken.";
			return 0;
	
		default:
			std::cout << "Created new desktop \"" + save_name + "\"";
		}
	}
	// Load desktop
	else if (operation == "-l")
	{
		// Must have a third argument
		if (argc < 3)
		{
			std::cerr << "ERROR: Missing save name.";
			return ERROR_BAD_ARGUMENTS;
		}
	
		// Get the load name
		const std::string load_name = argv[2];
		std::cout << "Loading current desktop with name \"" + load_name + "\"\n";
	
		// Load the desktop
		ds::LoadDesktopResult result = save_data.load_desktop(load_name);
	
		switch (result)
		{
		case ds::LoadDesktopResult::InvalidSaveName:
			std::cerr << "ERROR: A save with that name does not exist.";
			return 0;
	
		case ds::LoadDesktopResult::ActiveDesktopInvalid:
			std::cerr << "ERROR: The active desktop is invalid.";
			return 0;
	
		case ds::LoadDesktopResult::CantLoadActiveDesktop:
			std::cerr << "ERROR: Already the active desktop";
			return 0;
	
		default:
			std::cout << "Loaded save \"" + load_name + "\"";
		}
	}
	// Help
	else if (operation == "-h")
	{
		std::cout <<	"-n NAME : Create a \"New\" desktop with the name, NAME.\n"
						"-l NAME : \"Load\" the desktop with the name, NAME.\n"
						"-r      : \"Read\" all the saved desktops.\n"	
						"-h      : Ask for \"help.\"";
	}
	// Invalid argument
	else
	{
		std::cerr << "ERROR: Invalid argument.";
		return ERROR_BAD_ARGUMENTS;
	}

	return 0;
}