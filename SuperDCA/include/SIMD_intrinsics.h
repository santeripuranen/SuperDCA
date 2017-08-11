/** @file SIMD_intrinsics.h
 
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
#ifndef SUPERDCA_SIMD_INTRINSICS_H
#define SUPERDCA_SIMD_INTRINSICS_H

#ifndef NO_INTRINSICS

#pragma message("Vectorization using SIMD intrinsics is ENABLED.")

#ifdef __AVX512F__
#pragma message("Using AVX-512 intrinsics")
#include <immintrin.h>
#elif __MIC__ // Intel MIC
#pragma message("Using Intel MIC/KNC (\"Knights Corner\") intrinsics")
#include <immintrin.h>
#elif __AVX2__
#pragma message("Using AVX2 intrinsics")
#include <immintrin.h>
#elif __AVX__
#pragma message("Using AVX intrinsics")
#include <immintrin.h>
#elif __SSE3__ // SSE3 was introduced in 2004 (Intel) and 2005 (AMD); it is usually safe to assume at least SSE3 support.
#pragma message("Using SSE3 intrinsics")
#include <immintrin.h>
#elif __SSE2__ // SSE2 is the default minimum for x86_64 builds
#pragma message("Using SSE2 intrinsics")
#include <emmintrin.h> // SSE2
#elif __SSE__
#pragma message("Using SSE intrinsics")
#include <xmmintrin.h> // SSE
#else
#pragma message("Not using SIMD intrinsics. Switch on SIMD support in your compiler, if you want a vectorized build.")
#define NO_INTRINSICS
#endif

#if defined(__MIC__) || defined(__AVX2__) || defined(__AVX__) || defined(__SSE3__) || defined(__SSE2__)
#define REQUIRE_ALIGNED_MEMORY
#endif

#else
#pragma message("Vectorization using SIMD intrinsics is DISABLED.")
#endif // NO_INTRINSICS

#endif // SUPERDCA_SIMD_INTRINSICS_H
