/* $Id: hero_list.cpp 49598 2011-05-22 17:55:54Z mordante $ */
/*
   Copyright (C) 2008 - 2011 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "kingdom-lib"

#include "gui/dialogs/hero_selection.hpp"

#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "team.hpp"
#include "artifical.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_load
 *
 * == Load a game ==
 *
 * This shows the dialog to select and load a savegame file.
 *
 * @begin{table}{dialog_widgets}
 *
 * txtFilter & & text & m &
 *         The filter for the listbox items. $
 *
 * savegame_list & & listbox & m &
 *         List of savegames. $
 *
 * -filename & & control & m &
 *         Name of the savegame. $
 *
 * -date & & control & o &
 *         Date the savegame was created. $
 *
 * preview_pane & & widget & m &
 *         Container widget or grid that contains the items for a preview. The
 *         visible status of this container depends on whether or not something
 *         is selected. $
 *
 * -minimap & & minimap & m &
 *         Minimap of the selected savegame. $
 *
 * -imgLeader & & image & m &
 *         The image of the leader in the selected savegame. $
 *
 * -lblScenario & & label & m &
 *         The name of the scenario of the selected savegame. $
 *
 * -lblSummary & & label & m &
 *         Summary of the selected savegame. $
 *
 * @end{table}
 */

REGISTER_DIALOG(hero_selection)

thero_selection::thero_selection(std::vector<team>* teams, unit_map* units, hero_map& heros, std::vector<std::pair<int, unit*> >& pairs, int side, const std::string& disable_str)
	: teams_(teams)
	, units_(units)
	, heros_(heros)
	, side_(side)
	, pairs_(pairs)
	, max_checked_size_(pairs.size())
	, current_page_(twidget::npos)
{
	disables_ = utils::split(disable_str);
}

void thero_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	max_checked_size_ = 1;

	page_panel_ = find_widget<tstacked_widget>(&window, "page", false, true);
	lists_.push_back(find_widget<tlistbox>(&window, "ownership_list", false, true));
	lists_.push_back(find_widget<tlistbox>(&window, "ability_list", false, true));
	lists_.push_back(find_widget<tlistbox>(&window, "feature_list", false, true));
	lists_.push_back(find_widget<tlistbox>(&window, "adaptability_list", false, true));
	lists_.push_back(find_widget<tlistbox>(&window, "command_list", false, true));
	lists_.push_back(find_widget<tlistbox>(&window, "personal_list", false, true));
	lists_.push_back(find_widget<tlistbox>(&window, "relation_list", false, true));

	catalog_page(window, OWNERSHIP_PAGE, false);

	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "ownership", false)
		, boost::bind(
			&thero_selection::catalog_page
			, this
			, boost::ref(window)
			, (int)OWNERSHIP_PAGE
			, true));
	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "ability", false)
		, boost::bind(
			&thero_selection::catalog_page
			, this
			, boost::ref(window)
			, (int)ABILITY_PAGE
			, true));
	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "feature", false)
		, boost::bind(
			&thero_selection::catalog_page
			, this
			, boost::ref(window)
			, (int)FEATURE_PAGE
			, true));
	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "adaptability", false)
		, boost::bind(
			&thero_selection::catalog_page
			, this
			, boost::ref(window)
			, (int)ADAPTABILITY_PAGE
			, true));
	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "command", false)
		, boost::bind(
			&thero_selection::catalog_page
			, this
			, boost::ref(window)
			, (int)COMMAND_PAGE
			, true));
	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "personal", false)
		, boost::bind(
			&thero_selection::catalog_page
			, this
			, boost::ref(window)
			, (int)PERSONAL_PAGE
			, true));
	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "relation", false)
		, boost::bind(
			&thero_selection::catalog_page
			, this
			, boost::ref(window)
			, (int)RELATION_PAGE
			, true));

	tbutton* ok = find_widget<tbutton>(&window, "ok", false, true);
	ok->set_active(false);
}

void thero_selection::post_show(twindow& window)
{
}

void thero_selection::hero_toggled(twidget* widget)
{
	ttoggle_button* toggle = dynamic_cast<ttoggle_button*>(widget);
	int toggled_index = toggle->get_data();
	std::set<size_t>::iterator result = checked_pairs_.find(toggled_index);

	if (result == checked_pairs_.end()) {
		// toggled button isn't in checked_heros_
		if (toggle->get_value()) {
			if (checked_pairs_.size() < max_checked_size_) {
				checked_pairs_.insert(toggled_index);
			} else {
				// At most select max_checked_size_ heros. decheck it!
				toggle->set_value(false);
				return;
			}			
		} else {
			VALIDATE(false, "hero_toggled program error #1");
		}
	} else if (toggle->get_value()) { 
		VALIDATE(false, "hero_toggled program error #2");
	} else {
		checked_pairs_.erase(result);
	}

	twindow* window = toggle->get_window();
	tbutton* ok = find_widget<tbutton>(window, "ok", false, true);
	ok->set_active(checked_pairs_.empty()? false: true);
}

void thero_selection::fill_table(tlistbox& list, int catalog)
{
	std::vector<int> features;
	std::stringstream str;
	std::string color;

	hero& leader = *(*teams_)[side_ - 1].leader();
	int hero_index = 0;
	for (std::vector<std::pair<int, unit*> >::iterator itor = pairs_.begin(); itor != pairs_.end(); ++ itor, hero_index ++) {
		/*** Add list item ***/
		string_map table_item;
		std::map<std::string, string_map> table_item_item;

		hero& h = heros_[itor->first];
		unit& u = *itor->second;

		int catalog_diff = posix_abs((int)leader.base_catalog_ - h.base_catalog_);
		if (catalog_diff > HERO_MAX_LOYALTY / 2) {
			catalog_diff = HERO_MAX_LOYALTY - catalog_diff;
		}

		if (catalog == OWNERSHIP_PAGE) {
			table_item["label"] = h.name();
			table_item_item.insert(std::make_pair("name", table_item));

			if (h.side_ != HEROS_INVALID_SIDE) {
				table_item["label"] =  (*teams_)[h.side_].name();
			} else {
				table_item["label"] = "---";
			}
			table_item_item.insert(std::make_pair("side", table_item));

			artifical* city = units_? units_->city_from_cityno(h.city_): NULL;
			if (city) {
				table_item["label"] = city->name();
			} else {
				table_item["label"] = "---";
			}
			table_item_item.insert(std::make_pair("city", table_item));

			str.str("");
			if (h.side_ != HEROS_INVALID_SIDE) {
				str << h.loyalty(leader);
			} else {
				str << "--";
			}
			table_item["label"] = str.str();
			table_item_item.insert(std::make_pair("loyalty", table_item));

			str.str("");
			str << (int)h.base_catalog_;
			if (HERO_MAX_LOYALTY - catalog_diff >= game_config::move_loyalty_threshold) {
				color = "green";
			} else if (HERO_MAX_LOYALTY - catalog_diff >= game_config::wander_loyalty_threshold) {
				color = "yellow";
			} else {
				color = "red";
			}
			str << "(" << tintegrate::generate_format(catalog_diff, color) << ")";
			table_item["label"] = str.str();
			table_item_item.insert(std::make_pair("hero_catalog", table_item));

			str.str("");
			if (!u.is_artifical()) {
				str << u.name() << _("name^Troop");
			} else if (u.is_city()) {
				str << u.name();
			} else {
				str << u.name() << u.type_name();
			}
			table_item["label"] = str.str();
			table_item_item.insert(std::make_pair("position", table_item));

			table_item["label"] = lexical_cast<std::string>(h.meritorious_);
			table_item_item.insert(std::make_pair("meritorious", table_item));

		} else if (catalog == ABILITY_PAGE) {
			table_item["label"] = h.name();
			table_item_item.insert(std::make_pair("name", table_item));

			table_item["label"] = lexical_cast<std::string>(fxptoi9(h.leadership_));
			table_item_item.insert(std::make_pair("leadership", table_item));

			table_item["label"] = lexical_cast<std::string>(fxptoi9(h.force_));
			table_item_item.insert(std::make_pair("force", table_item));

			table_item["label"] = lexical_cast<std::string>(fxptoi9(h.intellect_));
			table_item_item.insert(std::make_pair("intellect", table_item));

			table_item["label"] = lexical_cast<std::string>(fxptoi9(h.spirit_));
			table_item_item.insert(std::make_pair("spirit", table_item));

			table_item["label"] = lexical_cast<std::string>(fxptoi9(h.charm_));
			table_item_item.insert(std::make_pair("charm", table_item));

			table_item["label"] = hero::status_str(h.status_);
			table_item_item.insert(std::make_pair("action", table_item));

		} else if (catalog == FEATURE_PAGE) {
			table_item["label"] = h.name();
			table_item_item.insert(std::make_pair("name", table_item));

			table_item["label"] = hero::feature_str(h.feature_);
			table_item_item.insert(std::make_pair("feature", table_item));

			table_item["label"] = hero::feature_desc_str(h.feature_);
			table_item_item.insert(std::make_pair("explain", table_item));

		} else if (catalog == ADAPTABILITY_PAGE) {
			table_item["label"] = h.name();
			table_item_item.insert(std::make_pair("name", table_item));

			table_item["label"] = hero::adaptability_str2(h.arms_[0]);
			table_item_item.insert(std::make_pair("arm0", table_item));

			table_item["label"] = hero::adaptability_str2(h.arms_[1]);
			table_item_item.insert(std::make_pair("arm1", table_item));

			table_item["label"] = hero::adaptability_str2(h.arms_[2]);
			table_item_item.insert(std::make_pair("arm2", table_item));

			table_item["label"] = hero::adaptability_str2(h.arms_[3]);
			table_item_item.insert(std::make_pair("arm3", table_item));

			table_item["label"] = hero::adaptability_str2(h.arms_[4]);
			table_item_item.insert(std::make_pair("arm4", table_item));

		} else if (catalog == COMMAND_PAGE) {
			table_item["label"] = h.name();
			table_item_item.insert(std::make_pair("name", table_item));

			table_item["label"] = "----";
			table_item_item.insert(std::make_pair("command", table_item));

		} else if (catalog == PERSONAL_PAGE) {
			table_item["label"] = h.name();
			table_item_item.insert(std::make_pair("name", table_item));

			table_item["label"] = hero::gender_str(h.gender_);
			table_item_item.insert(std::make_pair("gender", table_item));

		} else if (catalog == RELATION_PAGE) {
			table_item["label"] = h.name();
			table_item_item.insert(std::make_pair("name", table_item));

			if (h.parent_[0].hero_ != HEROS_INVALID_NUMBER) {
				table_item["label"] = heros_[h.parent_[0].hero_].name();
			} else {
				table_item["label"] = "";
			}
			table_item_item.insert(std::make_pair("father", table_item));

			if (h.parent_[1].hero_ != HEROS_INVALID_NUMBER) {
				table_item["label"] = heros_[h.parent_[1].hero_].name();
			} else {
				table_item["label"] = "";
			}
			table_item_item.insert(std::make_pair("mother", table_item));

			str.str("");
			for (uint32_t i = 0; i < HEROS_MAX_OATH; i ++) {
				if (h.oath_[i].hero_ != HEROS_INVALID_NUMBER) {
					if (i == 0) {
						str << heros_[h.oath_[i].hero_].name();
					} else {
						str << " " << heros_[h.oath_[i].hero_].name();
					}
				}
			}
			table_item["label"] = str.str();
			table_item_item.insert(std::make_pair("oath", table_item));

			str.str("");
			for (uint32_t i = 0; i < HEROS_MAX_CONSORT; i ++) {
				if (h.consort_[i].hero_ != HEROS_INVALID_NUMBER) {
					if (i == 0) {
						str << heros_[h.consort_[i].hero_].name();
					} else {
						str << " " << heros_[h.consort_[i].hero_].name();
					}
				}
			}
			table_item["label"] = str.str();
			table_item_item.insert(std::make_pair("consort", table_item));

			str.str("");
			for (uint32_t i = 0; i < HEROS_MAX_INTIMATE; i ++) {
				if (h.intimate_[i].hero_ != HEROS_INVALID_NUMBER) {
					if (i == 0) {
						str << heros_[h.intimate_[i].hero_].name();
					} else {
						str << " " << heros_[h.intimate_[i].hero_].name();
					}
				}
			}
			table_item["label"] = str.str();
			table_item_item.insert(std::make_pair("intimate", table_item));

			str.str("");
			for (uint32_t i = 0; i < HEROS_MAX_HATE; i ++) {
				if (h.hate_[i].hero_ != HEROS_INVALID_NUMBER) {
					if (i == 0) {
						str << heros_[h.hate_[i].hero_].name();
					} else {
						str << " " << heros_[h.hate_[i].hero_].name();
					}
				}
			}
			table_item["label"] = str.str();
			table_item_item.insert(std::make_pair("hate", table_item));

		}
		list.add_row(table_item_item);

		twidget* grid_ptr = list.get_row_panel(hero_index);
		ttoggle_button* toggle = dynamic_cast<ttoggle_button*>(grid_ptr->find("prefix", true));
		toggle->set_callback_state_change(boost::bind(&thero_selection::hero_toggled, this, _1));
		toggle->set_data(hero_index);
		if (checked_pairs_.find(hero_index) != checked_pairs_.end()) {
			toggle->set_value(true);
		}

		bool enable = true;
		for (std::vector<std::string>::const_iterator it2 = disables_.begin(); it2 != disables_.end() && enable; ++ it2) {
			const std::string& tag = *it2;
			if (tag == "loyalty") {
				if (h.side_ != HEROS_INVALID_SIDE) {
					hero& ownership_leader = *(*teams_)[h.side_].leader();
					if (ownership_leader.base_catalog_ == h.base_catalog_) {
						enable = false;
					}
				}
			} else if (tag == "hate") {
				if (leader.is_hate(h)) {
					enable = false;
				}
			}
		}
		if (enable && h.player_ == HEROS_INVALID_NUMBER && std::find(disables_.begin(), disables_.end(), "member") != disables_.end()) {
			if (runtime_groups::exist_member(h, leader)) {
				toggle->set_active(false);
			}
		}
		if (!enable) {
			// grid_ptr->set_active(false);
			toggle->set_active(false);
		}
	}
}

void thero_selection::catalog_page(twindow& window, int page, bool swap)
{
	if (!page_panel_) {
		return;
	}
	if (current_page_ == page) {
		return;
	}

	int selected_row = 0;
	if (swap) {
		// because sort, order is changed.
		tlistbox& list = *lists_[current_page_];
		selected_row = list.get_selected_row();
		list.clear();
	}

	page_panel_->set_radio_layer(page);
	tlistbox& list = *lists_[page];

	fill_table(list, page);

	if (swap) {
		list.select_row(selected_row);
		list.invalidate_layout(true);
	}
	current_page_ = page;
}

} // namespace gui2
