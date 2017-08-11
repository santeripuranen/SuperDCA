/** @file StateVector_parser_FASTA_grammar.hpp

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

#ifndef APEGRUNT_STATEVECTOR_PARSER_FASTA_GRAMMAR_HPP
#define APEGRUNT_STATEVECTOR_PARSER_FASTA_GRAMMAR_HPP

#define BOOST_SPIRIT_USE_PHOENIX_V3 1
//#define BOOST_RESULT_OF_USE_DECLTYPE
//#define BOOST_RESULT_OF_USE_DECLTYPE_WITH_TR1_FALLBACK 1

#include <iosfwd>
#include <memory> // for std::unique_ptr and std::shared_ptr
#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "Parser_commons.hpp"
#include "StateVector_forward.h"

namespace apegrunt {

namespace parsers {

namespace qi	= boost::spirit::qi;
namespace ascii	= boost::spirit::ascii;
namespace phx	= boost::phoenix;

template< typename IteratorT, typename StateVectorT >
struct StateVector_parser_FASTA_grammar
	//: qi::grammar< IteratorT, void(StateVectorT* const), ascii::space_type >
	: qi::grammar< IteratorT, void(StateVectorT*), ascii::space_type >
{
	static const char s_fasta_tag;

	using space_t = ascii::space_type;
	using statevector_t = StateVectorT;

	explicit StateVector_parser_FASTA_grammar( )
		: StateVector_parser_FASTA_grammar::base_type(sequence, "StateVector_parser_FASTA_grammar")
	{
		state_string %= *( qi::char_ - qi::char_(s_fasta_tag) ) // buffer input into a std::string
		//state_string = *( ( qi::char_ - qi::char_(s_fasta_tag) )[ phx::bind( &statevector_t::push_back, qi::_r1, qi::_1 ) ] )// buffer input into a std::string
		;
		// Main node
		sequence = state_string[ phx::bind( &statevector_t::assign, qi::_r1, std::move(qi::_1) ) ] // transfer cached string to the state container
		//sequence = state_string(qi::_r1)[ phx::bind( &statevector_t::flush_cache, qi::_r1 ) ] // flush the StateVector input cache
		;
	}

	qi::rule<IteratorT, void(statevector_t* const), space_t> sequence;
	qi::rule<IteratorT, std::string(), space_t> state_string;
	//qi::rule<IteratorT, void(statevector_t* const), space_t> state_string;

};

template< typename IteratorT, typename StateVectorT >
const char StateVector_parser_FASTA_grammar<IteratorT, StateVectorT>::s_fasta_tag = '>';
/*
template< typename IteratorT, typename StateVectorT >
qi::rule<IteratorT, void(StateVectorT* const), ascii::space_type> StateVector_parser_FASTA_grammar<IteratorT, StateVectorT>::state_string = *( qi::char_ - qi::char_(StateVector_parser_FASTA_grammar<IteratorT, StateVectorT>::s_fasta_tag) )[ phx::bind( &StateVectorT::push_back, qi::_r1, qi::_1 ) ];

template< typename IteratorT, typename StateVectorT >
qi::rule<IteratorT, void(StateVectorT* const), ascii::space_type> StateVector_parser_FASTA_grammar<IteratorT, StateVectorT>::sequence = StateVector_parser_FASTA_grammar<IteratorT, StateVectorT>::state_string(qi::_r1)[ phx::bind( &StateVectorT::flush_cache, qi::_r1 ) ];
*/
} // namespace parsers

} // namespace apegrunt

#endif // APEGRUNT_STATEVECTOR_PARSER_FASTA_GRAMMAR_HPP

