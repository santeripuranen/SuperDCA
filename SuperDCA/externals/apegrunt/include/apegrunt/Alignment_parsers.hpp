/** @file Alignment_parsers.hpp

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

#ifndef APEGRUNT_ALIGNMENT_PARSERS_HPP
#define APEGRUNT_ALIGNMENT_PARSERS_HPP

#include <iosfwd> // for passing std::istream*

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/iostreams/device/mapped_file.hpp> // for boost::iostreams::mapped_file
#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp

#include "Alignment.h"
#include "Alignment_parser_FASTA_grammar.hpp"

namespace apegrunt {

namespace fs = boost::filesystem;
namespace ascii	= boost::spirit::ascii;
namespace spirit = boost::spirit;

//Alignment_ptr parse_Alignment( std::istream* instream );

template< typename AlignmentT >
Alignment_ptr< typename AlignmentT::state_t > parse_Alignment( std::istream* instream )
{
	using iterator_t = spirit::istream_iterator;
	//using iterator_t = std::istream_iterator<char>;
	Alignment_ptr< typename AlignmentT::state_t > alignment;
	if( instream && instream->good() )
	{
		instream->unsetf(std::ios::skipws); // No white space skipping!

		// wrap istream into iterator
		iterator_t begin( *instream );
		iterator_t end;

		apegrunt::parsers::Alignment_parser_FASTA_grammar< iterator_t, AlignmentT > parser;
		bool success = boost::spirit::qi::phrase_parse(begin, end, parser, ascii::space, alignment);
	}
	return alignment;
}

template< typename AlignmentT >
Alignment_ptr< typename AlignmentT::state_t > parse_Alignment( const std::string& infilename )
{
	using iterator_t = char const*;

	Alignment_ptr< typename AlignmentT::state_t > alignment;

	fs::path filepath( infilename );

	if( fs::exists( filepath ) && fs::is_regular_file(filepath) )
	{
		boost::iostreams::mapped_file mmap(infilename.c_str(), boost::iostreams::mapped_file::readonly);
		iterator_t begin = mmap.const_data();
		iterator_t end = begin + mmap.size();

		apegrunt::parsers::Alignment_parser_FASTA_grammar< iterator_t, AlignmentT > parser;
		const bool success = boost::spirit::qi::phrase_parse(begin, end, parser, ascii::space, alignment);
		if( success )
		{
			alignment->set_id_string( filepath.stem().c_str() );
		}
	}
	return alignment;
}

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_PARSERS_HPP

