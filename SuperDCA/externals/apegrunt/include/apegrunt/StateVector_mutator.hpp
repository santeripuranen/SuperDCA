/** @file StateVector_mutator.hpp
 
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

#ifndef APEGRUNT_STATEVECTOR_MUTATOR_HPP
#define APEGRUNT_STATEVECTOR_MUTATOR_HPP

#include "StateVector_mutator_forward.h"
//#include "StateVector_interface.hpp"

namespace apegrunt {

template< typename StateVectorT >
class StateVector_mutator
{
public:
	StateVector_mutator( StateVectorT *statevector ) : m_statevector(statevector) { }
	~StateVector_mutator() { }

	template< typename StateT >
	void operator()( StateT state ) { m_statevector->push_back( state ); }

private:
	StateVectorT *m_statevector;
};

} // namespace apegrunt

#endif // APEGRUNT_STATEVECTOR_MUTATOR_HPP
