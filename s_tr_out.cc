/*                       -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2020  Szymon Blachuta
 *               2020  Felix Salfelder
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
 * qucs output functions (print tr only). to be moved to output plugin.
 */
#include "u_status.h"
#include "m_wave.h"
#include "u_prblst.h"
#include "u_sim_data.h"
#include "declare.h"	/* plottr, plopen */
#include "s__.h"

/*--------------------------------------------------------------------------*/
/* SIM::____list: access probe lists
 */
const PROBELIST& SIM::alarmlist()const
{ untested();
  return _probe_lists->alarm[_sim->_mode];
}
const PROBELIST& SIM::plotlist()const
{ untested();
  return _probe_lists->plot[_sim->_mode];
}
const PROBELIST& SIM::printlist()const
{
  return _probe_lists->print[_sim->_mode];
}
const PROBELIST& SIM::storelist()const
{ untested();
  return _probe_lists->store[_sim->_mode];
}
/*--------------------------------------------------------------------------*/
/* SIM::out: output the data, "keep" for ac reference
 * x = the x coordinate
 * print = selected points, "print" to screen, files, etc.
 * store = all points, for internal postprocessing, measure
 * keep = after the command is done, dcop for ac
 */
void SIM::outdata(double x, int outflags)
{
  ::status.output.start();
  if (outflags & ofKEEP) {
    _sim->keep_voltages();
  }else{
  }
  if (outflags & ofPRINT) {
    store_results(x);
    _sim->reset_iteration_counter(iPRINTSTEP);
    ::status.hidden_steps = 0;
  }else{
    ++::status.hidden_steps;
  }
  if (outflags & ofSTORE) {
//    store_results(x);
  }else{
  }
  ::status.output.stop();
}
/*--------------------------------------------------------------------------*/
/* SIM::head: print column headings and draw plot borders
 */
void SIM::head(double, double, const std::string&)
{
  if (_sim->_waves) { untested();
    delete [] _sim->_waves;
  }else{
  }

  _sim->_waves = new WAVE [printlist().size()];
}
/*--------------------------------------------------------------------------*/
/* SIM::print_results: print the list of results (text form) to _out
 * The argument is the first column (independent variable, aka "x")
 */
void SIM::print_results(double)
{ untested();
  unreachable();
}
/*--------------------------------------------------------------------------*/
/* SIM::alarm: print a message when a probe is out of range
 */
void SIM::alarm(void)
{ untested();
  _out.setfloatwidth(OPT::numdgt, OPT::numdgt+6);
  for (PROBELIST::const_iterator
	 p=alarmlist().begin();  p!=alarmlist().end();  ++p) { untested();
    if (!p->in_range()) { untested();
      _out << p->label() << '=' << p->value() << '\n';
    }else{ untested();
    }
  }
}
/*--------------------------------------------------------------------------*/
/* SIM::store: store data in preparation for post processing
 */
void SIM::store_results(double x)
{
  int ii = 0;
  for (PROBELIST::const_iterator
	 p=printlist().begin();  p!=printlist().end();  ++p) {
    _sim->_waves[ii++].push(x, p->value());
  }
}
/*--------------------------------------------------------------------------*/
void finish_hack(SIM*, OMSTREAM& outFile)
{
  // TODO: this is currently within the obsolete control script
  //outFile << "<Qucs Dataset>"  << endl; //"<Qucs Dataset 0.0.19>"
  //
  // TODO: make it work with dc, ac etc.
  // TODO: move to output plugin
  auto& PL = SIM::_probe_lists->print[s_TRAN];
  unsigned number_of_probes = PL.size();
  trace1("finish_hack", number_of_probes);

  if(number_of_probes){
    unsigned n = 0;
    for(auto d : CKT_BASE::_sim->_waves[0]){
      (void) d;
      ++n;
    }
    outFile << "<indep time " << n << ">\n";
    for(auto d : CKT_BASE::_sim->_waves[0]){
      outFile << d.first << "\n";
    }
    outFile << "</indep>\n";
  }else{ untested();
  }
  std::vector<std::string> names;

  unsigned i=0;
  for(auto ii : PL){
    std::string itm = ii.label();
    outFile << "<dep " << itm.substr(2, itm.length()-3 ) << "." << (char)toupper(itm.at(0)) << "t time>\n";
    for(auto d : CKT_BASE::_sim->_waves[i]){
      outFile << d.second << "\n";
    }
    outFile << "</dep>\n";
    ++i;
  }

  assert(number_of_probes == i);
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
