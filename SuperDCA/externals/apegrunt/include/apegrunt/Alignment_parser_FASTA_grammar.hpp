/** @file Alignment_parser_FASTA_grammar.hpp

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

#ifndef APEGRUNT_ALIGNMENT_PARSER_FASTA_GRAMMAR_HPP
#define APEGRUNT_ALIGNMENT_PARSER_FASTA_GRAMMAR_HPP

#define BOOST_SPIRIT_USE_PHOENIX_V3 1
//#define BOOST_RESULT_OF_USE_DECLTYPE
//#define BOOST_RESULT_OF_USE_DECLTYPE_WITH_TR1_FALLBACK 1

#include <iosfwd>
#include <memory> // for std::unique_ptr and std::shared_ptr
#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "Parser_commons.hpp"
#include "Alignment_forward.h"
#include "StateVector_parser_FASTA_grammar.hpp"

namespace apegrunt {

namespace parsers {

namespace qi	= boost::spirit::qi;
namespace ascii	= boost::spirit::ascii;
namespace phx	= boost::phoenix;

template< typename IteratorT, typename AlignmentT >
struct Alignment_parser_FASTA_grammar
	: qi::grammar< IteratorT, Alignment_ptr<typename AlignmentT::state_t>(), qi::locals< std::shared_ptr<AlignmentT>, typename AlignmentT::statevector_t* >, ascii::space_type >
{
	static const std::string s_fasta_tag;

	using space_t = ascii::space_type;

	using alignment_t = AlignmentT;
	using alignment_ptr_t = std::shared_ptr<alignment_t>;

	using statevector_t = typename AlignmentT::statevector_t;

	explicit Alignment_parser_FASTA_grammar( )
		: Alignment_parser_FASTA_grammar::base_type(alignment, "Alignment_parser_FASTA_grammar")
	{
		using apegrunt::parsers::printer;

		sequence_name %= ( qi::no_skip[ *( qi::char_ - '\n') >> '\n' ] ) //[phx::bind(&printer, std::string("Found a string and EOL") )]
		;

		// Main node
		alignment = /* a simple "qi::_a = std::make_shared<expression_t>()" would not be lazy,
				instead it's evaluated immediately and only once. lazy_make_shared evaluates
				lazily each time the parent parser is successfully called. */
				qi::eps[ qi::_a = apegrunt::parsers::lazy_make_shared<alignment_t>()() ]
			 >>	*( // parse zero or more sequences
				   qi::char_(s_fasta_tag)
				>> sequence_name[ qi::_b = phx::bind( &alignment_t::get_new_sequence, phx::bind( &alignment_ptr_t::get, qi::_a ), qi::_1 ) ] // pass a raw ptr
				>> FASTA_alignment_sequence( qi::_b ) // pass a raw ptr
			 )
			 >> qi::eps[ qi::_val = qi::_a ]
		;
/*
		qi::on_error<qi::fail>(start,
		   std::cerr << phx::val("Expected ") << qi::_4 << phx::val(" at offset ")
		      << (qi::_3 - qi::_1) << phx::val(" in ") << std::string(qi::_1, qi::_2)
		      << std::endl)
		;
*/
	}

	StateVector_parser_FASTA_grammar<IteratorT, statevector_t>
		FASTA_alignment_sequence
	;

	qi::rule<IteratorT, std::string(), space_t>
		sequence_name
	;

	qi::rule<IteratorT, Alignment_ptr< typename AlignmentT::state_t >(), qi::locals< alignment_ptr_t, statevector_t* >, space_t>
		alignment
	;

};

template< typename IteratorT, typename AlignmentT >
const std::string Alignment_parser_FASTA_grammar<IteratorT, AlignmentT>::s_fasta_tag = ">";

} // namespace parsers

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_PARSER_FASTA_GRAMMAR_HPP

