/** @file Alignment_parser_forward.h

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

#ifndef APEGRUNT_ALIGNMENT_PARSER_FORWARD_H
#define APEGRUNT_ALIGNMENT_PARSER_FORWARD_H

namespace apegrunt {

// forward declarations
#ifdef USE_BOOST_SPIRIT_X3
#pragma message("Using boost::spirit x3")
namespace parsers {
namespace Alignment_parser_grammar {
// forward declaration of grammar template, needed for friend declaration within the Alignment class
template< typename AlignmentT >
class parse;
} // namespace Alignment_parser_grammar
} // namespace parsers
#else
// forward declaration of grammar template, needed for friend declaration within the Alignment class
namespace parsers {
template< typename IteratorT, typename AlignmentT > struct Alignment_parser_FASTA_grammar;
} // namespace parsers
#endif

#ifdef USE_BOOST_SPIRIT_X3
#define ALIGNMENT_PARSER_GRAMMAR_FRIENDS(Alignment_type) \
	template< statevector_t > friend class apegrunt::parsers::Alignment_parser_grammar::parse;
#else
#define ALIGNMENT_PARSER_GRAMMAR_FRIENDS(Alignment_type) \
	template< typename IteratorT, Alignment_type& > friend class apegrunt::parsers::Alignment_parser_FASTA_grammar;
#endif // #ifdef USE_BOOST_SPIRIT_X3

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_PARSER_FORWARD_H

