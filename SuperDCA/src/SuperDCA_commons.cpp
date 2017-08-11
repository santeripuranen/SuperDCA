/** @file SuperDCA_commons.cpp

	Copyright (c) 2016-2017 Santeri Puranen.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.

	@author Santeri Puranen
	$Id: $
*/

#include <cstdlib>

#include "SuperDCA_commons.h"

namespace superdca {

// Print msg to cout and exit with flag set to EXIT_SUCCESS if success.
void Exit(bool exit_flag, std::string msg)
{
	std::exit(exit_flag);
}

bool readYesNoAnswer( std::istream *in, std::ostream *out )
{
	if( in )
	{
		bool answer_ok = false;
		std::string answer;
		do
		{
			*in >> std::ws >> answer;
			if(answer.compare("yes") == 0 || answer.compare("y") == 0 || answer.compare("Y") == 0
					|| answer.compare("YES") == 0) return true;
			else if(answer.compare("no") == 0 || answer.compare("n") == 0 || answer.compare("N") == 0
					|| answer.compare("NO") == 0) return false;
			else
			{
				if( out )
				{
					*out << "Please answer either 'y' or 'n' or 'yes' or 'no': ";
					out->flush();
				}
			}
		}
		while(!answer_ok);
	}
	return false;
}

} // namespace superdca
