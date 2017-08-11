/** @file StateVector_utility.hpp

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

#ifndef APEGRUNT_STATEVECTOR_UTILITY_HPP
#define APEGRUNT_STATEVECTOR_UTILITY_HPP

#include <numeric> // for std::accumulate

#include "Apegrunt_utility.hpp"
#include "StateVector.h"

namespace apegrunt {

template< typename StateT >
inline std::size_t count_identical( const StateVector_ptr<StateT>& a, const StateVector_ptr<StateT>& b )
{
	return *a && *b;
/*
	const auto boolvec = *a & *b;
	return std::count( cbegin(boolvec), cend(boolvec), true );
*/
/*
	using boost::get;
	return std::accumulate( cbegin(zip_range( lhs, rhs )), cend(zip_range( lhs, rhs )), std::size_t(0), [=](auto sum, const auto& pair) { return get<0>(pair) == get<1>(pair) ? ++sum : sum; } ); // the number of identical loci
*/
/*
	std::size_t n_ident = 0;
	for( const auto ab: zip_range( a, b ) ) { get<0>(ab) == get<1>(ab) && ++n_ident; }

	return n_ident;
*/
}

template< typename StateT >
inline bool identical( const StateVector_ptr<StateT>& a, const StateVector_ptr<StateT>& b )
{
	// StateVectors are identical if they are of equal length and all corresponding states between the two are exactly equal
	return *a == *b;
}

template< typename StateT, typename RealT=double >
inline RealT fract_identical( const StateVector_ptr<StateT>& a, const StateVector_ptr<StateT>& b )
{
	using real_t = RealT;

	const std::size_t n_elem = std::min(a->size(), b->size());

	if( 0 == n_elem ) { return real_t(0); }

	const std::size_t n_ident = count_identical( a, b );

	return real_t( n_ident ) / real_t( n_elem );
}
/*
inline bool identical( const StateVector_ptr& a, const StateVector_ptr& b )
{
	// StateVectors are identical if they are of equal length and all corresponding states between the two are exactly equal
	return a->size() == b->size() && a->size() == n_identical(a,b);
}
*/

} // namespace apegrunt

#endif // APEGRUNT_STATEVECTOR_UTILITY_HPP

