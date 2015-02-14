/* Copyright (C) 2012 Fabian Vallon
 *           (C) 2012-2015 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 * qucs language helpers
 */

#ifndef L_QUCS_H__
#define L_QUCS_H__

#include "ap.h"
#include "io_trace.h"

template <class T>
inline bool QucsGet(CS& cmd, const std::string& key, T* val)
{ untested();
  if (cmd.umatch(key + " {=}") and OPT::language->name() == "qucs") {
    CS value (CS::_STRING, cmd.ctos(",=;)", "\"'{(", "\"'})"));
    std::string newvalue(value.ctos());
    newvalue += value.ctos();
    trace2("QucsGet MATCH:: ", key, newvalue);
    CS temp (CS::_STRING, newvalue);
    temp >> *val;
    return true;
  }else{
    return false;
  }
}

bool QucsGuessParam(std::string& p)
{
	std::string temp;
	for (unsigned i = 0; p[i] != '\0'; i++){
		if (p[i] != ' '){ untested();
			temp.append(1,p[i]);
		}else{ untested();
			break;
		}
	}
	p.swap(temp);

	bool ok=true; // incomplete.
	return ok;
}

#endif
