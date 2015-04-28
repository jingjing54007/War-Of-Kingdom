/* $Id: mp_connect.cpp 49307 2011-04-25 07:19:14Z mordante $ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/mp_connect.hpp"

#include "game_preferences.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "video.hpp"

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

namespace gui2 {

namespace {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_server_list
 *
 * == Multiplayer server list ==
 *
 * This shows the dialog with a list of predefined multiplayer servers.
 *
 * @begin{table}{dialog_widgets}
 *
 * server_list & & listbox & m &
 *         Listbox with the predefined servers to connect to. $
 *
 * -name & & control & o &
 *         Widget which shows the name of the server. $
 *
 * -address & & control & m &
 *         The address/host_name of the server. $
 *
 * @end{table}
 */

class tmp_server_list : public tdialog
{
public:
	tmp_server_list() :
		host_name_()
	{}

	const std::string& host_name() const { return host_name_; }

private:
	std::string host_name_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

REGISTER_DIALOG(mp_server_list)

void tmp_server_list::pre_show(CVideo& /*video*/, twindow& window)
{
	window.set_canvas_variable("border", variant("default-border"));

	tlistbox& list = find_widget<tlistbox>(&window, "server_list", false);

	window.keyboard_capture(&list);

	const std::vector<game_config::server_info>&
		pref_servers = preferences::server_list();

	BOOST_FOREACH (const game_config::server_info& server, pref_servers) {

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = server.name;
		data.insert(std::make_pair("name", item));

		item["label"] = server.address;
		data.insert(std::make_pair("address", item));

		list.add_row(data);
	}
}

void tmp_server_list::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {

		const tlistbox& list = find_widget<const tlistbox>(
				&window, "server_list", false);

		const twidget* row = list.get_row_panel(list.get_selected_row());

		host_name_ = find_widget<const tcontrol>(row, "address", false).label();
	}
}

} // namespace

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_connect
 *
 * == Multiplayer connect ==
 *
 * This shows the dialog to the MP server to connect to.
 *
 * @begin{table}{dialog_widgets}
 *
 * start_table & & text_box & m &
 *         The name of the server to connect to. $
 *
 * list & & button & o &
 *         Shows a dialog with a list of predefined servers to connect to. $
 *
 * @end{table}
 */

REGISTER_DIALOG(mp_connect)

twindow* window_;
static void show_server_list(
		  CVideo& video
		, tfield_text* host_name)
{
	assert(host_name);
	twindow& window = *window_;

	tmp_server_list dlg;
	dlg.show(video);

	if(dlg.get_retval() == twindow::OK) {
		host_name->set_widget_value(window, dlg.host_name());
	}
}

tmp_connect::tmp_connect()
	: host_name_(register_text("host_name"
			, true
			, preferences::network_host
			, preferences::set_network_host
			, true))
{
}

void tmp_connect::pre_show(CVideo& video, twindow& window)
{
	assert(host_name_);
	window_ = &window;

	// Set view list callback button.
	if(tbutton* button = find_widget<tbutton>(&window, "list", false, false)) {

		connect_signal_mouse_left_click(*button, boost::bind(
				  show_server_list
				, boost::ref(video)
				, host_name_));
	}

}

tdialog* tmp_connect::mp_server_list_for_unit_test()
{
	return new tmp_server_list();
}

} // namespace gui2
