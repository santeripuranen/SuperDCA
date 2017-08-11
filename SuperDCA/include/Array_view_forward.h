/** @file Array_view_forward.h
 
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
#ifndef SUPERDCA_ARRAY_VIEW_FORWARD_H
#define SUPERDCA_ARRAY_VIEW_FORWARD_H

#include "Math_utility.hpp"

namespace superdca {

template< typename T, std::size_t Extent=0, std::size_t Rank=rank<T>::value+1 >
class Array_view;

template< typename T, std::size_t Extent >
struct rank< Array_view<T, Extent> > : std::integral_constant< std::size_t, rank<T>::value+1 > { };

template<> template< typename T, std::size_t Extent >
struct extent< Array_view<T, Extent>, 0 > : std::integral_constant< std::size_t, Extent > { };

template<> template< typename T, std::size_t Extent, unsigned N >
struct extent< Array_view<T, Extent>, N > : std::integral_constant< std::size_t, extent<T,N-1>::value > { };

template<> template< typename T, std::size_t Extent >
struct scalar_type< Array_view<T,Extent> > { using type = typename scalar_type<T>::type; };
/*
template<> template< typename T, std::size_t Extent >
struct recursive_extent< Array_view<T,Extent> > : std::integral_constant<
  	  std::size_t,
	  extent< Array_view<T,Extent> >::value
	  *
	  extent< Array_view<T,Extent>, rank< Array_view<T,Extent> >::value-1 >::value
> { };
*/
template<> template< typename T, std::size_t Extent >
struct recursive_extent< Array_view<T,Extent> > : std::integral_constant<
  	  std::size_t,
	  extent< Array_view<T,Extent> >::value
	  *
	  recursive_extent< T >::value
> {
	enum { current_level=extent< Array_view<T,Extent> >::value };
	enum { sub_level=recursive_extent< T >::value };
};

} // namespace superdca

#endif // SUPERDCA_ARRAY_VIEW_FORWARD_H
