/*                       -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2020  Szymon Blachuta
 *               2020, 2021  Felix Salfelder
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
 * qucs output functions (print dc only). to be moved to output plugin.
 *
 * this is hopeless, but maybe useful for testing.
 * need sensible data container.
 */
#include "u_sim_data.h"
#include "u_status.h"
#include "m_wave.h"
#include "u_prblst.h"
#include "declare.h"	/* plottr, plopen */
#include "s__.h"
/*--------------------------------------------------------------------------*/
static const int ofKEY=2048;
OMSTREAM* out_hack;
std::vector<std::string> _labels;
std::vector<double> _keys;
std::vector<int> _counts;
std::vector<std::vector<double>> _ctx;
unsigned _count;
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
  if (outflags & ofKEY){
    if(_keys.size() == _labels.size()-1){
      _counts.push_back(_count);
      _keys.clear();
      _count = 0;
    }else{
    }
    _keys.push_back(x);
  }else{
  }
  if (outflags & ofKEEP) { untested();
    _sim->keep_voltages();
  }else{
  }
  if (outflags & ofPRINT) {
    _ctx.push_back(_keys);
    ++_count;
    for(auto i : _keys){
      _out << i;
    }
    store_results(x);
    _sim->reset_iteration_counter(iPRINTSTEP);
    ::status.hidden_steps = 0;
  }else{
    ++::status.hidden_steps;
  }
  if (outflags & ofSTORE) { untested();
//    store_results(x);
  }else{
  }
  ::status.output.stop();
}
/*--------------------------------------------------------------------------*/
/* SIM::head: print column headings and draw plot borders
 */
void SIM::head(double start, double stop, const std::string& col1)
{
  if(_labels.size()){
  }else{
    // init?
    if (_sim->_waves) {
      delete [] _sim->_waves;
    }else{
    }
  }

  _sim->_waves = new WAVE [printlist().size()];
  out_hack = &_out;

  _labels.push_back(col1);
}
/*--------------------------------------------------------------------------*/
/* SIM::print_results: print the list of results (text form) to _out
 * The argument is the first column (independent variable, aka "x")
 */
void SIM::print_results(double x)
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
// "OUTPUT::init"
void init_hack(SIM* s)
{
  _labels.clear();
  _keys.clear();
  _ctx.clear();
  _counts.clear();
  _count = 0;
}
/*--------------------------------------------------------------------------*/
void finish_hack(SIM* s)
{
  // TODO: this is currently within the obsolete control script
  //outFile << "<Qucs Dataset>"  << endl; //"<Qucs Dataset 0.0.19>"
  //
  // TODO: make it work with dc, ac etc.
  // TODO: move to output plugin
  auto& PL = SIM::_probe_lists->print[s_TRAN];
  unsigned number_of_probes = PL.size();
  trace1("finish_hack", number_of_probes);

  assert(out_hack);
  auto& outFile = *out_hack;
  std::string dep, SW;

//  for(auto i: _labels){ untested();
//    outFile << "sweep " << i << "\n";
//  }

  if(number_of_probes){
    unsigned n = 0;
    for(auto d : CKT_BASE::_sim->_waves[0]){
      ++n;
    }
    // BUG: only works for <= 2 sweeps
    if(_labels.size() > 1){
      int cc = _counts[0];
      outFile << "<indep " << _labels[0] << " " << n/cc << ">\n";
      int i=0;
      for(auto d : CKT_BASE::_sim->_waves[0]){
	outFile << "  " << d.first << "\n";
	i+= n/cc;
	if(i>=n){
	  break;
	}
      }
      outFile << "</indep>\n";
    }else{
    }

    if(n==1){
      dep = "indep";
      SW = "1";
    }else{
      dep = "dep";
      SW = _labels[0];
      if(_labels.size()>1){
       	SW += " " + _labels[1];
      }
      // DUP
      if(_labels.size()==1){
	outFile << "<indep " << _labels.back() << " " << n << ">\n";
	for(auto d : CKT_BASE::_sim->_waves[0]){
	  outFile << d.first << "\n";
	}
	outFile << "</indep>\n";
      }else if(_labels.size()==2){
	outFile << "<indep " << _labels.back() << " " << n << ">\n";
	for(unsigned i=0; i<n;){
	  assert(_ctx[i].size());
	  outFile << "  " << _ctx[i][0] << "\n";
	  int cc = _counts[0];
	  i+=cc;
	}
	outFile << "</indep>\n";
      }
    }
  }else{ untested();
  }
  std::vector<std::string> names;

  unsigned i=0;
  for(auto ii : PL){
    names.push_back(ii.label());
    ++i;
  }

  for(unsigned i=0; i<number_of_probes; ++i){
    std::string itm = names[i];
    outFile << "<" << dep << " " << itm.substr(2, itm.length()-3 ) << "."<< (char)toupper(itm.at(0))
            << " " << SW << ">\n";
    for(auto d : CKT_BASE::_sim->_waves[i]){
      outFile << d.second << "\n";
    }
    outFile << "</" << dep << ">\n";
  }
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
