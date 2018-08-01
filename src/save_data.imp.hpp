#pragma once

/**
 * @file save_data.imp.hpp
 * @brief Save data header implementation file.
 * @author Connor J. Bramham (ReeCocho)
 */

namespace ds
{
	inline std::string SavedDesktop::get_name() const noexcept
	{
		return m_name;
	}

	inline size_t SaveData::get_save_count() const
	{
		return m_desktops.size();
	}

	inline SavedDesktop& SaveData::get_active_desktop()
	{
		return get_save(m_active_desktop);
	}

	inline SavedDesktop& SaveData::get_save(size_t i)
	{
		return m_desktops[i];
	}

	inline SavedDesktop& SaveData::get_save(const std::string& name)
	{
		// Loop over every desktop and find one with a matching name
		for (auto& desktop : m_desktops)
			if (desktop.get_name() == name)
				return desktop;

		throw std::runtime_error("Unable to find a desktop with the desired name.");
	}
}