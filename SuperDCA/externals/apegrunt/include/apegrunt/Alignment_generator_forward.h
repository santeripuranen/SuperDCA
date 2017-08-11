/** @file Alignment_generator_forward.h

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

#ifndef APEGRUNT_ALIGNMENT_GENERATOR_FORWARD_H
#define APEGRUNT_ALIGNMENT_GENERATOR_FORWARD_H

namespace apegrunt {

// forward declarations
#ifdef USE_BOOST_SPIRIT_X3
#pragma message("Using boost::spirit x3")
namespace generators {
namespace Alignment_generator_grammar {
// forward declaration of grammar template, needed for friend declaration within the Alignment class
template< typename AlignmentT >
class generate;
} // namespace Alignment_generator_grammar
} // namespace generators
#else
// forward declaration of grammar template, needed for friend declaration within the Alignment class
namespace generators {
template< typename IteratorT, typename AlignmentT > struct Alignment_generator_FASTA_grammar;
} // namespace generators
#endif

#ifdef USE_BOOST_SPIRIT_X3
#define ALIGNMENT_GENERATOR_GRAMMAR_FRIENDS(Alignment_type) \
	template< IteratorT > friend class apegrunt::generators::Alignment_generator_grammar::parse;
#else
#define ALIGNMENT_GENERATOR_GRAMMAR_FRIENDS(Alignment_type) \
	template< typename IteratorT > friend class apegrunt::generators::Alignment_generator_FASTA_grammar;
#endif // #ifdef USE_BOOST_SPIRIT_X3

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_GENERATOR_FORWARD_H
