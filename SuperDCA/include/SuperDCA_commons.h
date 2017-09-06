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
#include <fstream>
#include <sstream>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp

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

template< typename StreamT >
class stream_name_association
{
public:
	template< typename StringT >
	stream_name_association( StreamT&& s, const StringT& name )
	: m_stream(std::move(s)), m_name(name) { }

	StreamT* stream() { return &m_stream; }

	const std::string& name() const { return m_name; }

	void close() { m_stream.close(); }

private:
	StreamT m_stream;
	std::string m_name;
};

template< typename StringT >
stream_name_association<std::ofstream> get_unique_ofstream( StringT filename )
{
	boost::filesystem::path filepath;

	int index = 0;
	do
	{
		std::ostringstream index_os; index_os << index;
		filepath = std::string(filename) + ( index == 0 ? "" : "."+index_os.str() );
	}
	while( boost::filesystem::exists( filepath ) && ++index );

	std::ofstream outfile( filepath.c_str(), std::ios_base::binary );

	return stream_name_association<std::ofstream>( std::move(outfile), filepath.c_str() );
}

} // namespace superdca

#endif // SUPERDCA_COMMONS_H
