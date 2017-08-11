/** @file StateVector_forward.h

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

#ifndef APEGRUNT_STATEVECTOR_FORWARD_H
#define APEGRUNT_STATEVECTOR_FORWARD_H

namespace apegrunt {

// forward declarations
template< typename StateT >
class StateVector;

template< typename StateT >
using StateVector_ptr = std::shared_ptr< StateVector< StateT > >;
//using StateVector_ptr = std::shared_ptr< StateVector >;

/*
template< typename StateVectorT, typename... Args >
StateVector_ptr< typename StateVectorT::state_t > make_StateVector_ptr( Args&&... args )
{
	return std::make_shared< StateVectorT >( std::move(args...) );
	//return StateVector_ptr( new StateVectorT( args... ) );
}
*/
template< typename StateVectorT, typename... Args >
StateVector_ptr< typename StateVectorT::state_t > make_StateVector_ptr( Args&... args )
{
	return std::make_shared< StateVectorT >( args... );
}

template< typename StateVectorT >
StateVector_ptr< typename StateVectorT::state_t > make_StateVector_ptr( const StateVectorT& state_vector )
{
	return std::make_shared< StateVectorT >( state_vector );
}

} // namespace apegrunt

#endif // APEGRUNT_STATEVECTOR_FORWARD_H

