/** @file Loci_parser_grammar.hpp

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

#ifndef APEGRUNT_LOCI_PARSER_GRAMMAR_HPP
#define APEGRUNT_LOCI_PARSER_GRAMMAR_HPP

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
#include "Loci_forward.h"
#include "Loci_impl_default_storage.hpp"

namespace apegrunt {

namespace parsers {

namespace qi	= boost::spirit::qi;
namespace ascii	= boost::spirit::ascii;
namespace phx	= boost::phoenix;

template< typename IteratorT, typename LociT=apegrunt::Loci_impl_default_storage<std::size_t> >
struct Loci_parser_grammar
	: qi::grammar< IteratorT, Loci_ptr(), qi::locals< std::shared_ptr<LociT> >, ascii::space_type >
{
	using space_t = ascii::space_type;

	using loci_t = LociT;
	using loci_ptr_t = std::shared_ptr<loci_t>;

	// Explicit member function pointer signatures; used for the static cast below, in order for function
	// pointer signatures to be properly resolved when compiling with gcc against boost >1.62.0
	typedef void (loci_t::*push_back)(std::size_t locus, std::size_t base);
	typedef void (loci_t::*push_back_range)(std::size_t locus_begin, std::size_t locus_end, std::size_t base);

	explicit Loci_parser_grammar( std::size_t base_index=1 )
		: Loci_parser_grammar::base_type(start, "Loci_parser_grammar"), m_base_index(base_index)
	{
		using apegrunt::parsers::printer;

		//loci_count %= qi::_int
		//;

		single_locus = ( qi::uint_ - '-' )[ phx::bind( static_cast<push_back>(&loci_t::push_back), qi::_r1, qi::_1, m_base_index ) ] // 1 is the second argument of the bound member function; the default value for that argument is apparently not enought, but an explicit value is needed to compile
		;

		loci_range = ( qi::uint_ >> '-' >> qi::uint_ )[ phx::bind( static_cast<push_back_range>(&loci_t::push_back), qi::_r1, qi::_1, qi::_2, m_base_index ) ] // 1 is the third argument of the bound member function; the default value for that argument is apparently not enought, but an explicit value is needed to compile
		;

		loci = loci_range(qi::_r1) | single_locus(qi::_r1)
		;

		// Main node
		start =	//qi::eps[ phx::bind( &index_base, qi::_r1 )
				/* a simple "qi::_a = std::make_shared<T>()" would not be lazy,
				instead it's evaluated immediately and only once. lazy_make_shared evaluates
				lazily each time the parent parser is successfully called. */
				qi::eps[ qi::_a = apegrunt::parsers::lazy_make_shared<loci_t>()() ]
			 //>> loci_count[ qi::_b = phx::bind( &loci_t::reserve, phx::bind( &loci_ptr_t::get, qi::_a ), qi::_1 ) ] // pass a raw ptr
			 >> *( loci( phx::bind( &loci_ptr_t::get, qi::_a ) ) ) // pass a raw ptr
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

//	qi::rule<IteratorT, std::size_t(), space_t>
//		loci_count
//	;

	qi::rule<IteratorT, void(loci_t* const), space_t>
		single_locus,
		loci_range,
		loci
	;

	qi::rule< IteratorT, Loci_ptr(), qi::locals< std::shared_ptr<loci_t> >, ascii::space_type > start;

	std::size_t m_base_index;

};

} // namespace parsers

} // namespace apegrunt

#endif // APEGRUNT_LOCI_PARSER_GRAMMAR_HPP

