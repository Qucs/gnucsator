/*$Id: s__out.cc 2016/09/22 $ -*- C++ -*-
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
 * tr,dc analysis output functions (and some ac)
 */
//testing=obsolete,script 2005.09.17
#include "u_sim_data.h"
#include "u_status.h"
#include "m_wave.h"
#include "u_prblst.h"
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
{ untested();
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
{ untested();
  ::status.output.start();
  if (outflags & ofKEEP) { untested();
    _sim->keep_voltages();
  }else{ untested();
  }
  if (outflags & ofPRINT) { untested();
    plottr(x, plotlist());
    print_results(x);
    _sim->reset_iteration_counter(iPRINTSTEP);
    ::status.hidden_steps = 0;
  }else{ untested();
    ++::status.hidden_steps;
  }
  if (outflags & ofSTORE) { untested();
    alarm();
    store_results(x);
  }else{ untested();
  }
  ::status.output.stop();
}
/*--------------------------------------------------------------------------*/
/* SIM::head: print column headings and draw plot borders
 */
void SIM::head(double start, double stop, const std::string& col1)
{ untested();
  if (_sim->_waves) { untested();
    delete [] _sim->_waves;
  }else{ untested();
  }

  _sim->_waves = new WAVE [storelist().size()];


  if (!plopen(start, stop, plotlist())) { untested();
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    int width = std::min(OPT::numdgt+5, BIGBUFLEN-10);
    char format[20];
    //sprintf(format, "%%c%%-%u.%us", width, width);
    sprintf(format, "%%c%%-%us", width);

    _out.form(format, '#', col1.c_str());

    for (PROBELIST::const_iterator
	   p=printlist().begin();  p!=printlist().end();  ++p) { untested();
      _out.form(format, ' ', p->label().c_str());
    }
    _out << '\n';
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
/* SIM::print_results: print the list of results (text form) to _out
 * The argument is the first column (independent variable, aka "x")
 */
void SIM::print_results(double x)
{ untested();
  if (!IO::plotout.any()) { untested();
    _out.setfloatwidth(OPT::numdgt, OPT::numdgt+6);
    assert(x != NOT_VALID);
    _out << x;
    for (PROBELIST::const_iterator
	   p=printlist().begin();  p!=printlist().end();  ++p) { untested();
      _out << p->value();
    }
    _out << '\n';
  }else{ untested();
  }
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
{ untested();
  int ii = 0;
  for (PROBELIST::const_iterator
	 p=storelist().begin();  p!=storelist().end();  ++p) { untested();
    _sim->_waves[ii++].push(x, p->value());
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
