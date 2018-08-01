#pragma once

/**
 * @file save_data.hpp
 * @brief Save data header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <string>
#include "json.hpp"

/** For convenience. */
using json = nlohmann::json;

namespace ds
{
	/**
	 * Saved desktop.
	 */
	class SavedDesktop
	{
	public:

		/**
		 * Constructor.
		 * @param Save name.
		 * @param Path to save folder.
		 */
		SavedDesktop(const std::string& name, const std::string& path);

		/**
		 * Get the save name.
		 * @return Save name.
		 */
		inline std::string get_name() const noexcept;

		/**
		 * Save the current desktop.
		 */
		void save();

		/**
		 * Load the current desktop.
		 */
		void load();

	private:

		/** Save name. */
		const std::string m_name;

		/** Path to save folder. */
		const std::string m_path;
	};

	/**
	 * New desktop return codes.
	 */
	enum class NewDesktopResult
	{
		Success = 0,
		NameTaken = 1,
		ActiveDesktopInvalid = 2
	};

	/**
	 * Load desktop return codes.
	 */
	enum class LoadDesktopResult
	{
		Success = 0,
		InvalidSaveName = 1,
		ActiveDesktopInvalid = 2,
		CantLoadActiveDesktop = 3
	};

	/**
	 * Object to manage save data.
	 */
	class SaveData
	{
	public:

		/**
		 * Constructor.
		 * @param Path to Desktop-Saver folder.
		 */
		SaveData(const std::string& path);

		/**
		 * Save the current state.
		 */
		void save();

		/**
		 * Get the number of saves.
		 * @param Number of saves.
		 */
		inline size_t get_save_count() const;

		/**
		 * Get the active desktop.
		 * @return Active desktop.
		 */
		inline SavedDesktop& get_active_desktop();

		/**
		 * Get a save by index.
		 * @param Save index.
		 * @return Save file.
		 */
		inline SavedDesktop& get_save(size_t i);

		/**
		 * Get a save by name.
		 * @param Save name.
		 * @param Save file.
		 */
		inline SavedDesktop& get_save(const std::string& name);

		/**
		 * Create a new desktop.
		 * @param Name.
		 * @return Result of creating the new desktop.
		 */
		NewDesktopResult new_desktop(const std::string& name);

		/**
		 * Load a desktop.
		 * @param Desktop name.
		 * @return Result of loading the desktop.
		 */
		LoadDesktopResult load_desktop(const std::string& name);

	private:

		/** Path to Desktop-Saver folder. */
		const std::string m_path;

		/** Saved desktops. */
		std::vector<SavedDesktop> m_desktops;

		/** Name of the active desktop. */
		std::string m_active_desktop;
	};
}

#include "save_data.imp.hpp"