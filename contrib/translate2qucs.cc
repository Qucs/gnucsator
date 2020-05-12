/*                -*- C++ -*-
 * Copyright (C) 2020 Qucs Team
 * Author: Szymon Blachuta
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
 */

#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using std::string;
using std::cin;
using std::cout;
using std::vector;
using std::istream;
using std::ostream;
using std::stringstream;
using std::endl;

class Translate2qucs{
private:
  static vector<string> split(const string &s, char delim);
  static void wrData(const vector<string> *d, ostream &outFile);
  static void collectData(vector<vector<string>*> &data, const string line, int cols);
public:
  static void qnucapTR2qucs(istream &inFile, ostream &outFile);
};

vector<string> Translate2qucs::split(const string &s, char delim) {
  stringstream ss(s);
  string item;
  vector<string> elems;
  while (getline(ss, item, delim)) {
    if (item.length()==0 || item.at(0)==delim) continue;
    elems.push_back(item);
  }
  return elems;
}
void Translate2qucs::wrData(const vector<string> *d, ostream &outFile){
  string el;
  for(int i=0; i<d->size(); ++i){
    el = (*d)[i];
    if (el.at(0)!='-'){
      el = '+' + el;  // 0.123 -> +0.123
    }
    outFile << "  " << el << endl;
  }
  delete d;
}
void Translate2qucs::collectData(vector<vector<string>*> &data, const string line, int cols){
  if (data.size()==0){
    //initialize
    for(int i=0; i<cols; ++i){
      data.push_back(new vector<string>());
    }
  }
  vector<string> datVect = split(line, ' ');
  for(int i=0; i<cols; ++i){
    data[i]->push_back(datVect[i]);
  }
}
/*
 *   input : iName
 *   output: oName
 */
void Translate2qucs::qnucapTR2qucs(istream &inFile, ostream &outFile){
  enum STATES {wait, tran, dc, verbose} state;
  state=wait;
  vector<string> probes;
  vector<vector<string>*> data;
  int n = 0;
  string line;
  while (getline(inFile, line)){
    if (line.length()==0) break;
    switch(state){
    case wait:
      if (line=="@@@VERBOSE@@@")        {state = verbose; break;}
      if (line.substr(0, 5) == "#Time") {state = tran; probes = split(line, ' '); n = 0; break;}
      break;

    case tran:
      if (line == "===DC")               {state = dc  ; break;}
      if (line.substr(0, 1) == "#")      {state = wait; break;}
      //  500.000000000000088E-6  0.  0. 35.9799269592363657 35.9799269592543602 0.
      ++n;
      collectData(data, line, probes.size());
      break;

    case dc:
      if (line.substr(0, 5) == "#Time") {state = tran; probes = split(line, ' '); n = 0; break;}
      if (line.substr(0, 1) == "#")     {state = wait; break;}
      //  500.000000000000088E-6  0.  0. 35.9799269592363657 35.9799269592543602 0.
      ++n;
      collectData(data, line, probes.size());
      break;

    case verbose:
      if (line == "@@@VERBOSE@@@")      {state = wait; break;}
      break;
    }
  }//while

  //outFile << "<Qucs Dataset>"  << endl; //"<Qucs Dataset 0.0.19>"
  outFile << "<indep time " << n << ">" << endl; //<indep time 19>
  wrData(data[0], outFile);
  outFile << "</indep>" << endl;

  for(int px=1; px<probes.size(); ++px){
    // "v(_net0)" -> "_net0"
    string itm = probes[px];
    outFile << "<dep " << itm.substr(2, itm.length()-3 ) << "."<< (char)toupper(itm.at(0)) << "t time>" <<endl;
    wrData(data[px], outFile);
    outFile << "</dep>" << endl;
  }
  data.clear();
}

int main(int argc, char *const* argv){
//cout << argv[0] << argc << endl;
//main_0(); exit(0);
  if (argc>1){
     cout << "usage: translate2qucs [--help]" <<endl;
     cout << "usage: translate2qucs < inFile > outFile " <<endl;
     exit(0);
  }
  Translate2qucs::qnucapTR2qucs(cin, cout);
  return 0;
}
