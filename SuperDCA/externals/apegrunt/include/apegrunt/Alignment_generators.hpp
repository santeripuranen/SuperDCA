/** @file Alignment_generators.hpp

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

#ifndef APEGRUNT_ALIGNMENT_GENERATORS_HPP
#define APEGRUNT_ALIGNMENT_GENERATORS_HPP

#include <iosfwd> // for passing std::istream*
#include <algorithm> // for std::copy

//#include <boost/spirit/include/karma.hpp>
//#include <boost/spirit/include/support_istream_iterator.hpp>
//#include <boost/iostreams/device/mapped_file.hpp> // for boost::iostreams::mapped_file
//#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp

#include "Alignment_forward.h"
//#include "Alignment_generator_FASTA_grammar.hpp"

namespace apegrunt {

namespace fs = boost::filesystem;
namespace ascii	= boost::spirit::ascii;
namespace spirit = boost::spirit;

template< typename StateT >
bool generate_Alignment( Alignment_ptr<StateT> alignment, std::ostream* outstream )
{
// this is temporary file output code, used until we have a proper generator implementation.
	if( outstream && outstream->good() )
	{
		for( const auto& sequence: alignment )
		{
			*outstream << ">" << sequence->id_string() << "\n";
			for( const auto& state: sequence ) { *outstream << state; }
			*outstream << "\n";
		}
		return true;
	}
	return false;
}

/*
// Boost Spirit Karma generator
template< typename StateT >
bool generate_Alignment( Alignment_ptr<StateT> alignment, std::ostream* outstream )
{
	using iterator_t = spirit::ostream_iterator;
	bool success = false;

	if( outstream && outstream->good() )
	{
		// wrap ostream into iterator
		iterator_t iterator( *outstream );
		//iterator_t end;

		apegrunt::generators::Alignment_generator_FASTA_grammar< iterator_t, StateT > generator;
		success = boost::spirit::karma::generate(iterator, generator, alignment);
	}
	return success;
}
*/
/*
// Boost Spirit Karma generator
template< typename StateT >
bool generate_Alignment( Alignment_ptr<StateT> alignment, const std::string& outfilename )
{
	using iterator_t = char const*;
	bool success = false;

	fs::path filepath( outfilename );

	if( !fs::exists( filepath ) ) //&& fs::is_regular_file(filepath) )
	{
		boost::iostreams::mapped_file mmap(outfilename.c_str(), boost::iostreams::mapped_file::readwrite);
		iterator_t iterator = mmap.data();

		apegrunt::generators::Alignment_generator_FASTA_grammar< iterator_t, StateT > generator;
		success = boost::spirit::karma::generate(iterator, generator, alignment);
	}

	return success;
}
*/

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_GENERATORS_HPP

