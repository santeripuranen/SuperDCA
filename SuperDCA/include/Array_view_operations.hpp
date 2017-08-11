/** @file Array_view_operations.hpp
 
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
#ifndef SUPERDCA_ARRAY_VIEW_OPERATIONS_HPP
#define SUPERDCA_ARRAY_VIEW_OPERATIONS_HPP

#include <cstring> // for std::memcpy

#include "Math_utility.hpp"
#include "Array_view_forward.h"
#include "Array_view_iterator_forward.h"
#include "Array_view.hpp"

namespace superdca {

template< typename RealT, std::size_t Extent >
void copy( const Array_view< Array_view< RealT, Extent >, Extent >& source, Array_view< Array_view< RealT, Extent >, Extent >& dest, bool transpose=false )
{
	if( transpose )
	{
		for( std::size_t i=0; i < Extent; ++i )
		{
			for( std::size_t j=0; j < Extent; ++j )
			{
				dest[i][j] = source[j][i];
			}
		}
	}
	else
	{
		std::memcpy( dest.data(), source.data(), Extent*Extent );
/*
		for( std::size_t i=0; i < Extent; ++i )
		{
			for( std::size_t j=0; j < Extent; ++j )
			{
				dest[i][j] = source[i][j];
			}
		}
*/
	}
}
/*
template< typename RealT, std::size_t Extent >
Array_view< RealT, Extent > operator+( const Array_view< RealT, Extent >& lhs, const Array_view< RealT, Extent >& rhs )
*/
/*
template<>
void copy( const Array_view< Array_view< double, 4 >, 4 >& source, Array_view< Array_view< double, 4 >, 4 >& dest, bool transpose=false )
{
	if( transpose )
	{
		for( std::size_t i=0; i < 4; ++i )
		{
			for( std::size_t j=0; j < 4; ++j )
			{
				dest[i][j] = source[j][i];
			}
		}
	}
	else
	{

	}
}
*/
} // namespace superdca

#endif // SUPERDCA_ARRAY_VIEW_OPERATIONS_HPP
