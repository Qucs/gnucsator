/*$Id: u_prblst.cc,v 26.137 2010/04/10 02:37:33 al Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
 * probe list functions
 */
//testing=script,sparse 2006.07.14
#include "u_prblst.h"
#include "e_cardlist.h"
#include "e_node.h"
#include "e_card.h"
#include "u_nodemap.h"
#include "ap.h"
/*--------------------------------------------------------------------------*/
void PROBE_LISTS::purge(CKT_BASE* brh)
{ untested();
  for (int i = 0;  i < sCOUNT;  ++i) { untested();
    alarm[i].remove_one(brh);
    plot[i] .remove_one(brh);
    print[i].remove_one(brh);
    store[i].remove_one(brh);
  }
}
/*--------------------------------------------------------------------------*/
void PROBELIST::listing(const std::string& label)const
{ untested();
  IO::mstdout.form("%-7s", label.c_str());
  for (const_iterator p = begin();  p != end();  ++p) { untested();
    IO::mstdout << ' ' << p->label();
    if (p->range() != 0.) {untested();
      IO::mstdout.setfloatwidth(5) 
	<< '(' << p->lo() << ',' << p->hi() << ')';
    }else{ untested();
    }
  }
  IO::mstdout << '\n';
}
/*--------------------------------------------------------------------------*/
void PROBELIST::clear(void)
{ untested();
  erase(begin(), end());
}
/*--------------------------------------------------------------------------*/
/* check for match
 * called by STL remove, below
 * both are needed to support different versions of STL
 */
bool operator==(const PROBE& prb, const std::string& par)
{ untested();
  return wmatch(prb.label(), par);
}
bool operator!=(const PROBE& prb, const std::string& par)
{untested();
  //return !wmatch(prb.label(), par);
  return !(prb == par);
}
/*--------------------------------------------------------------------------*/
/* remove a complete probe, extract from CS
 * wild card match  ex:  vds(m*)
 */
void PROBELIST::remove_list(CS& cmd)
{ 
  size_t mark = cmd.cursor();
  std::string parameter(cmd.ctos(TOKENTERM) + '(');
  int paren = cmd.skip1b('(');
  parameter += cmd.ctos(TOKENTERM) + ')';
  paren -= cmd.skip1b(')');
  if (paren != 0) {untested();
    cmd.warn(bWARNING, "need )");
  }else if (parameter.empty()) {untested();
    cmd.warn(bWARNING, "what's this?");
  }else{ untested();
  }

  iterator x = remove(begin(), end(), parameter);
  if (x != end()) { untested();
    erase(x, end());
  }else{itested();
    cmd.warn(bWARNING, mark, "probe isn't set -- can't remove");
  }
}
/*--------------------------------------------------------------------------*/
/* check for match
 * called by STL remove, below
 * both are needed to support different versions of stl
 */
bool operator==(const PROBE& prb, const CKT_BASE* brh)
{ untested();
  return (prb.object() == brh);
}
bool operator!=(const PROBE& prb, const CKT_BASE* brh)
{untested();
  return (prb.object() != brh);
}
/*--------------------------------------------------------------------------*/
/* remove a brh from a PROBELIST
 * removes all probes on brh
 */
void PROBELIST::remove_one(CKT_BASE *brh)
{ untested();
  assert(brh);
  erase(remove(begin(), end(), brh), end());
  // remove .. removes all that match and compacts the list, leaving blanks at the end
  // erase  .. shortens, throw away the blanks at the end
}
/*--------------------------------------------------------------------------*/
/* add_list: add a "list" of probes, usually only one
 * This means possibly several probes with a single parameter
 * like "v(r*)" meaning all resistors
 * but not "v(r4) v(r5)" which has two parameters.
 * It also takes care of setting the range for plot or alarm.
 */
void PROBELIST::add_list(CS& cmd, CARD_LIST* scope)
{ untested();
  assert(scope);
  if(scope==&CARD_LIST::card_list){ untested();
  }else{ untested();
  }
  int oldcount = size();
  std::string what(cmd.ctos(TOKENTERM));/* parameter */
  if (what.empty()) {untested();
    cmd.warn(bWARNING, "need a probe");
  }else{ untested();
  }

  int paren = cmd.skip1b('(');		/* device, node, etc. */
  if (cmd.umatch("nodes ")) { untested();
    // all nodes
    add_all_nodes(what, scope);
  }else if (cmd.umatch("0")) { untested();
    // node 0 means system stuff
    push_new_probe(what, 0);
  }else if (cmd.is_alnum() || cmd.match1("*?")) { untested();
    // branches or named nodes
    size_t here1 = cmd.cursor();
    bool found_something = add_branches(cmd.ctos(), what, scope);
    if (!found_something) { untested();
      cmd.warn(bWARNING, here1, "no match");
    }else{ untested();
    }
    for (;;) { untested();
      // a list, as in v(r1,r2,r3) or v(1,2,3)
      if (!(cmd.is_alnum() || cmd.match1("*?"))) { untested();
	break;
      }else{ untested();
      }
      size_t here2 = cmd.cursor();
      found_something = add_branches(cmd.ctos(), what, scope);
      if (!found_something) {itested();
	cmd.reset(here2);
	break;
      }else{ untested();
      }
    }
  }else{itested();
    cmd.warn(bDANGER, "need device or node");
  }
  paren -= cmd.skip1b(')');
  if (paren != 0) {itested();
    cmd.warn(bWARNING, "need )");
  }else{ untested();
  }

  if (cmd.skip1b('(')) {	/* range for plotting and alarm */
    double lo = cmd.ctof();
    double hi = cmd.ctof();
    for (iterator p = begin() + oldcount;  p != end();  ++p) { untested();
      p->set_limit(lo,hi);
    }    
    if (!cmd.skip1b(')')) {untested();
      cmd.check(bWARNING, "need )");
    }else{ untested();
    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void PROBELIST::push_new_probe(const std::string& param,const CKT_BASE* object)
{ untested();
  bag.push_back(PROBE(param, object));
}
/*--------------------------------------------------------------------------*/
void PROBELIST::add_all_nodes(const std::string& what, CARD_LIST* scope)
{ untested();
  assert(scope);
  if(scope==&CARD_LIST::card_list){ untested();
  }else{ untested();
  }
  for (NODE_MAP::const_iterator
       i = scope->nodes()->begin();
       i != scope->nodes()->end();
       ++i) { untested();
    trace1("node probe", i->first);
    if ((i->first != "0") && (i->first.find('.') == std::string::npos)) { untested();
      NODE* node = i->second;
      assert (node);
      push_new_probe(what, node);
    }else{ untested();
    }
  }
}
/*--------------------------------------------------------------------------*/
/* add_branches: add net elements to probe list
 * 	all matching a label with wildcards
 */
bool PROBELIST::add_branches(const std::string&device, 
			     const std::string&param,
			     const CARD_LIST* scope)
{ untested();
  assert(scope);
  if(scope==&CARD_LIST::card_list){untested();
  }else{ untested();
  }
  bool found_something = false;

  std::string::size_type dotplace = device.find_first_of(".");
  if (dotplace != std::string::npos) { untested();
    // has a dot, look deeper
    { // forward (Verilog style)
      std::string dev = device.substr(dotplace+1, std::string::npos);
      std::string container = device.substr(0, dotplace);
      for (CARD_LIST::const_iterator
	     i = scope->begin();  i != scope->end();  ++i) { untested();
	CARD* card = *i;
	if (card->is_device()
	    && card->subckt()
	    && wmatch(card->short_label(), container)) { untested();
	  found_something |= add_branches(dev, param, card->subckt());
	}else{ untested();
	}
      }
    }
    { // reverse (ACS style)
      dotplace = device.find_last_of(".");
      std::string container = device.substr(dotplace+1, std::string::npos);
      std::string dev = device.substr(0, dotplace);
      for (CARD_LIST::const_iterator
	     i = scope->begin();  i != scope->end();  ++i) { untested();
	CARD* card = *i;
	if (card->is_device()
	    && card->subckt()
	    && wmatch(card->short_label(), container)) { untested();
	  found_something |= add_branches(dev, param, card->subckt());
	}else{ untested();
	}
      }
    }
  }else{ untested();
    // no dots, look here
    if (device.find_first_of("*?") != std::string::npos) { untested();
      // there's a wild card.  do linear search for all
      { // nodes
	for (NODE_MAP::const_iterator 
	     i = scope->nodes()->begin();
	     i != scope->nodes()->end();
	     ++i) { untested();
	  if (i->first != "0") { untested();
	    NODE* node = i->second;
	    assert (node);
	    if (wmatch(node->short_label(), device)) { untested();
	      push_new_probe(param, node);
	      found_something = true;
	    }else{ untested();
	    }
	  }else{ untested();
	  }
	}
      }
      {// components
	for (CARD_LIST::const_iterator 
	     i = scope->begin();  i != scope->end();  ++i) { untested();
	  CARD* card = *i;
	  if (wmatch(card->short_label(), device)) { untested();
	    push_new_probe(param, card);
	    found_something = true;
	  }else{ untested();
	  }
	}
      }
    }else{ untested();
      // no wild card.  do fast search for one
      { // nodes
	NODE* node = (*scope->nodes())[device];
	if (node) { untested();
	  push_new_probe(param, node);
	  found_something = true;
	}else{ untested();
	}
      }
      { //components
	CARD_LIST::const_iterator i = scope->find_(device);
	if (i != scope->end()) { untested();
	  push_new_probe(param, *i);
	  found_something = true;
	}else{ untested();
	}
      }
    }
  }

  return found_something;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
