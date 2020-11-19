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
#include "io_error.h"
#include "io_trace.h"

template <class T>
inline bool QucsGet(CS& cmd, const std::string& key, T* val)
{
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

template <class T>
inline bool QucsSet(CS& cmd, const std::string& key, T* val, const T x)
{
  bool ret = false;
  if (!cmd.umatch("{=}")){ untested();
    cmd.check(bDANGER, "need =");
  }else{
    unsigned here = cmd.cursor();
    std::string s = cmd.ctos(",=;)", "\"'{(", "\"'})");
    trace2("QucsSet", key, s);

    CS newcmd (CS::_STRING, s);
    ret = Set(newcmd, key, val, x);
    if(!ret){untested();
      cmd.reset(here);
    }
  }
  return ret;
}

bool QucsGuessParam(std::string& s)
{
  char const* p = s.c_str();
  std::string temp;
  for (unsigned i = 0; p[i] != '\0'; i++){
    if (p[i] != ' '){
      temp.append(1, p[i]);
    }else if(!p[++i]){ untested();
      // end after blank (strange)
      break;
    }else if( p[i] == 'a' // -18
	   || p[i] == 'f' // -15
	   || p[i] == 'p' // -12
	   || p[i] == 'n' // -9
	   || p[i] == 'u' // -6
	   || p[i] == 'm' // -3
	   || p[i] == 'c' // -2
	   || p[i] == 'k' // 3
	   || p[i] == 'K' // 3
	   || p[i] == 'M' // 6
	   || p[i] == 'G' // 9
	   || p[i] == 'T' // 12
	   ){
      // pass through
      temp.append(1, p[i]);
      break;
    }else if( p[i] == 'A' ){
      // this could be "Ampere". ignore.
    }else if( p[i] == 'd' && p[i+1] == 'B' && p[i+2] == 'm' ){
      // dBm? need to substitute x for 10*log_10(x * 1000) (or so).
      incomplete();
      temp = s.substr(0,i); // ??
      break;
    }else if( ONE_OF
	   || p[i] == 'D' // 1
	   || p[i] == 'h' // 2
	   || p[i] == 'P' // 15
	   || p[i] == 'E' // 18
	   ){
      incomplete();
      error(bDANGER, "what is %s?\n", p);
      break;
    }else{
      break;
    }
  }
  trace2("p", p, temp);
  s.swap(temp);

  bool ok=true; // incomplete.
  return ok;
}

#endif
// vim:ts=8:sw=2:noet
