/** @file Vector_operations.hpp
 
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
#ifndef SUPERDCA_VECTOR_OPERATIONS_HPP
#define SUPERDCA_VECTOR_OPERATIONS_HPP

#include <algorithm> // for std::min, std::max
#include <array>

#ifndef SUPERDCA_NO_VECMATHLIB
#include "vecmathlib.h"
#endif

#include "Vector_forward.h"
#include "Vector_interface.hpp"
#include "Math_utility.hpp"

namespace superdca {

#ifndef NO_INTRINSICS
#ifdef __AVX__
inline double sum( __m256d a )
{
	using vec_t = Vector<double,4,false>;
	using simd_t = __m256d;
	const simd_t b( _mm256_permute2f128_pd( a, a, 0b0001 ) );
	const simd_t c( _mm256_add_pd( a, b ) );
	const vec_t d( _mm256_hadd_pd( c, c ) );

	return typename vec_t::element_t( d.m_elem[0] );
}

template< uint Exponent >
inline __m256d pow( __m256d x ) { __m256d result = x; for( std::size_t i=1; i<Exponent; ++i ) { result = result * x; } return result; }
template<>
inline __m256d pow<2>( __m256d x ) { return x*x; }

#ifndef SUPERDCA_NO_VECMATHLIB
inline __m256d exp( __m256d a )
{
	return vecmathlib::mathfuncs< vecmathlib::realvec<double,4> >::vml_exp( a ).v;
}
#else
/*
template< bool View >
inline Vector<double,4,View> exp( typename Vector<double,4,View>::simd_t a )
{
	return vecmathlib::mathfuncs< vecmathlib::realvec<double,4> >::vml_exp( a ).v;
}
*/
#endif // #ifndef SUPERDCA_NO_VECMATHLIB

#endif // __AVX__

#endif // #ifndef NO_INTRINSICS

} // namespace superdca

#endif // SUPERDCA_VECTOR_OPERATIONS_HPP
