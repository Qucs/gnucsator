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

#include <string>
#include <vector>
using namespace std;

class Translate2qucs{
private:
  static vector<string> split(const string &s, char delim);
  static void wrData(const vector<string> *d, ostream &outFile);
  static void collectData(vector<vector<string>*> &data, const string line, int cols);
public:
  static void qnucapTR2qucs(istream &inFile, ostream &outFile);
};
