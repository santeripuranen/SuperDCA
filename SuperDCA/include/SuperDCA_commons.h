/** @file SuperDCA_commons.h

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
#ifndef SUPERDCA_COMMONS_H
#define SUPERDCA_COMMONS_H

#include <string>
#include <iostream> // for std::cin, std::istream, std::cout

namespace superdca {

// Forward declaration; print msg to cout and exit with flag set to EXIT_SUCCESS if success.
void Exit(bool exit_flag, std::string msg = "");

/** Ask user to prompt either yes or no or y or n until he/she does.
  	First print your question and then call this function.
  	Requires
  	<code>
  	#include <string>
  	</code>
  	@return True if answer was yes, false if anwer was no.
*/
bool readYesNoAnswer( std::istream *in = &std::cin, std::ostream *out = &std::cout );

} // namespace superdca

#endif // SUPERDCA_COMMONS_H
