/** @file Parser_commons.hpp

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

#ifndef APEGRUNT_PARSER_COMMONS_HPP
#define APEGRUNT_PARSER_COMMONS_HPP

//#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <string>
#include <iostream>
#include <memory>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/phoenix/object/construct.hpp>

namespace apegrunt {

namespace parsers {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;

void printer( const std::string& output );

/** Helper to make lazy() accept an inherited attribute
 *
 */
struct rule_with_params_impl
{
    template <typename Rule, typename ...Param>
    auto operator()(const Rule& r, const Param& ...p) const -> decltype(r(p...))
    {
        return r(p...);
    }
};
/** Helper to make lazy() accept an inherited attribute
 *
 */
static const phx::function<rule_with_params_impl> rule_with_params;

template <typename T>
struct lazy_make_shared_impl
{
    template <typename... A>
    struct result
    {
    	using type = std::shared_ptr<T>;
    };

    template <typename... A>
    typename result<A...>::type operator()(A&&... a) const
    {
        return std::make_shared<T>(std::forward<A>(a)...);
    }
};

template <typename T>
using lazy_make_shared = boost::phoenix::function<lazy_make_shared_impl<T> >;

/** Parse any quoted string -- double or single quoted.
	All single quote characters will be retained within a doubly quoted string, and vice versa
*/
template< typename IteratorT >
struct quoted_string : qi::grammar< IteratorT, std::string(), qi::locals<char>, ascii::space_type >
{
	quoted_string() : quoted_string::base_type(qstring, "Quoted string parser")
	{
		qstring %=	qi::lexeme[
						qi::omit[ ascii::char_("\"'")[qi::_a = qi::_1] ]
						>>	+( ascii::char_ - qi::lit(qi::_a) )
						>	qi::lit(qi::_a)
					]
		;
	}

	qi::rule< IteratorT, std::string(), qi::locals<char>, ascii::space_type > qstring;
};

} // namespace parsers

} // namespace apegrunt

#endif // APEGRUNT_PARSER_COMMONS_HPP

