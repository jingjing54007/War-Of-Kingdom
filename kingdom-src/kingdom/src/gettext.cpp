/* $Id: gettext.cpp 46186 2010-09-01 21:12:38Z silene $ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "gettext.hpp"

#include <cstring>
/*
char *libintl_gettext (const char *__msgid)
{
	return NULL;
}
char *libintl_dgettext (const char *__domainname, const char *__msgid)
{
	return NULL;
}
char *libintl_dcgettext (const char *__domainname, const char *__msgid, int __category)
{
	return NULL;
}
char *libintl_ngettext (const char *__msgid1, const char *__msgid2, unsigned long int __n)
{
	return NULL;
}
char *libintl_dngettext (const char *__domainname, const char *__msgid1, const char *__msgid2, unsigned long int __n)
{
	return NULL;
}
char *libintl_dcngettext (const char *__domainname, const char *__msgid1, const char *__msgid2, unsigned long int __n, int __category)
{
	return NULL;
}
char *libintl_textdomain (const char *__domainname)
{
	return NULL;
}
char *libintl_bindtextdomain (const char *__domainname, const char *__dirname)
{
	return NULL;
}
char *libintl_bind_textdomain_codeset (const char *__domainname, const char *__codeset)
{
	return NULL;
}
*/

char const *egettext(char const *msgid)
{
	return msgid[0] == '\0' ? msgid : gettext(msgid);
}

const char* sgettext (const char *msgid)
{
	const char *msgval = gettext (msgid);
	if (msgval == msgid) {
		msgval = std::strrchr (msgid, '^');
		if (msgval == NULL)
			msgval = msgid;
		else
			msgval++;
	}
	return msgval;
}

const char* dsgettext (const char * domainname, const char *msgid)
{
	bind_textdomain_codeset(domainname, "UTF-8");
	const char *msgval = dgettext (domainname, msgid);
	if (msgval == msgid) {
		msgval = std::strrchr (msgid, '^');
		if (msgval == NULL)
			msgval = msgid;
		else
			msgval++;
	}
	return msgval;
}

const char* sngettext (const char *singular, const char *plural, int n)
{
	const char *msgval = ngettext (singular, plural, n);
	if (msgval == singular) {
		msgval = std::strrchr (singular, '^');
		if (msgval == NULL)
			msgval = singular;
		else
			msgval++;
	}
	return msgval;
}

const char* dsngettext (const char * domainname, const char *singular, const char *plural, int n)
{
	bind_textdomain_codeset(domainname, "UTF-8");
	const char *msgval = dngettext (domainname, singular, plural, n);
	if (msgval == singular) {
		msgval = std::strrchr (singular, '^');
		if (msgval == NULL)
			msgval = singular;
		else
			msgval++;
	}
	return msgval;
}