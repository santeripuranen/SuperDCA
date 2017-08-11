/** @file Alignment_forward.h

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

#ifndef APEGRUNT_ALIGNMENT_INTERFACE_FORWARD_H
#define APEGRUNT_ALIGNMENT_INTERFACE_FORWARD_H

namespace apegrunt {

enum { StateBlock_size = 16 };
enum { SequenceGroup_blocksize = 512 };

} // namespace apegrunt

namespace apegrunt {

// forward declarations
template< typename StateT >
class Alignment;

template< typename StateT >
using Alignment_ptr = std::shared_ptr< Alignment<StateT> >;
//using Alignment_ptr = std::shared_ptr< Alignment >;

template< typename AlignmentT, typename... Args >
Alignment_ptr< typename AlignmentT::state_t > make_Alignment_ptr( Args&&... args )
{
	return std::make_shared< AlignmentT >( std::move(args...) );
	//return Alignment_ptr( new AlignmentT( std::move(args...) ) );
}

template< typename AlignmentT >
Alignment_ptr< typename AlignmentT::state_t > make_Alignment_ptr( const AlignmentT& alignment )
{
	return std::make_shared< AlignmentT >( alignment );
}

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_INTERFACE_FORWARD_H

