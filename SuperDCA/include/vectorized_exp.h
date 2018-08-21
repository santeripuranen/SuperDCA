/** @file vectorized_exp.hpp
 
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
#ifndef SUPERDCA_VECTORIZED_EXP_HPP
#define SUPERDCA_VECTORIZED_EXP_HPP

#ifndef SUPERDCA_NO_VECMATHLIB
#include "vecmathlib.h"
#endif

namespace superdca {

#ifndef NO_INTRINSICS
#ifdef __AVX__

#ifndef SUPERDCA_NO_VECMATHLIB
inline __m256d exp( __m256d a ) { return vecmathlib::mathfuncs< vecmathlib::realvec<double,4> >::vml_exp( a ).v; }
inline __m256d abs( __m256d a ) { return vecmathlib::mathfuncs< vecmathlib::realvec<double,4> >::vml_fabs( a ).v; }
#else
// We're in a pickle
#endif // #ifndef SUPERDCA_NO_VECMATHLIB

#endif // __AVX__

#endif // #ifndef NO_INTRINSICS

} // namespace superdca

#endif // SUPERDCA_VECTORIZED_EXP_HPP
