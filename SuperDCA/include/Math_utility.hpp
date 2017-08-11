/** @file Math_utility.hpp
 
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
#ifndef SUPERDCA_MATH_UTILITY_HPP
#define SUPERDCA_MATH_UTILITY_HPP

#include <type_traits>

namespace superdca {

template< typename T >
struct scalar_type { using type = T; }; // default; works for POD types
/*
template<>
struct scalar_type< float > { using type = float; };

template<>
struct scalar_type< double > { using type = double; };
*/
template< typename T >
struct rank : std::integral_constant< std::size_t, 0 > { };

template< typename T, unsigned N = 0 >
struct extent : std::integral_constant< std::size_t, 0 > { };

template<>
struct extent< float > : std::integral_constant< std::size_t, 1 > { };

template<>
struct extent< double > : std::integral_constant< std::size_t, 1 > { };

template< typename T >
struct recursive_extent : std::integral_constant< std::size_t, 1 > { };
/*
template<>
struct recursive_extent<float> : std::integral_constant< std::size_t, 1 > { };

template<>
struct recursive_extent<double> : std::integral_constant< std::size_t, 1 > { };
*/
template< typename T >
struct size_of : std::integral_constant< std::size_t, 0 > { };

template<>
struct size_of< float > : std::integral_constant< std::size_t, sizeof(float) > { };

template<>
struct size_of< double > : std::integral_constant< std::size_t, sizeof(double) > { };

/*
// integer pow
int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1) { result *= base; }
        exp >>= 1;
        base *= base;
    }

    return result;
}
*/
template< typename IntegerT >
IntegerT ipow(IntegerT base, int exp)
{
	IntegerT result = 1;
    while (exp)
    {
        if (exp & 1) { result *= base; }
        exp >>= 1;
        base *= base;
    }

    return result;
}

} // namespace superdca

#endif // SUPERDCA_MATH_UTILITY_HPP
