/** @file Vector_interface.hpp
 
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
#ifndef SUPERDCA_VECTOR_INTERFACE_HPP
#define SUPERDCA_VECTOR_INTERFACE_HPP

#include "Vector_forward.h" // consistency check
#include "Array_view_forward.h"

namespace superdca {

template< typename RealT, uint Capacity, bool View=false >
struct Vector
{
	enum { N=Capacity };
	using element_t = RealT;
	using my_type = Vector<element_t,N,false>;

	inline Vector() { for( std::size_t i=0; i < N; ++i ) { m_elem[i] = element_t(0.0); } }
 	inline Vector( Vector<RealT,Capacity>&& v ) noexcept { for( std::size_t i=0; i < N; ++i ) { m_elem[i] = v.m_elem[i]; } }
	inline Vector( const Vector<RealT,Capacity>& v ) { for( std::size_t i=0; i < N; ++i ) { m_elem[i] = v.m_elem[i]; } }

	inline my_type& operator=( Vector<RealT,Capacity>&& v ) { for( std::size_t i=0; i < N; ++i ) { m_elem[i] = v.m_elem[i]; return *this; } }
	inline my_type& operator=( const Vector<RealT,Capacity>& v ) { for( std::size_t i=0; i < N; ++i ) { m_elem[i] = v.m_elem[i]; return *this; } }

	element_t* data() { return m_elem; }
	const element_t* data() const { return m_elem; }

	inline element_t& operator[]( uint i ) { return m_elem[i]; }
	inline const element_t& operator[]( uint i ) const { return m_elem[i]; }

	template< bool ViewFlag >
	inline my_type& operator+=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { m_elem[i] += v[i]; return *this; } }
	template< bool ViewFlag >
	inline my_type& operator-=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { m_elem[i] -= v[i]; return *this; } }
	template< bool ViewFlag >
	inline my_type& operator*=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { m_elem[i] *= v[i]; return *this; } }
	template< bool ViewFlag >
	inline my_type& operator/=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { m_elem[i] /= v[i]; return *this; } }

	union
	{
		element_t m_elem[N];
	};
};

template< typename RealT, uint Capacity, bool View >
std::ostream& operator<< ( std::ostream& os, const Vector<RealT,Capacity,View>& v )
{
	os << "[";
	for( std::size_t i=0; i < Capacity; ++i )
	{
		os << " " << v[i];
	}
	os << " ]";
	return os;
}

template< typename RealT, uint Capacity >
struct Vector<RealT,Capacity,true>
{
	enum { N=Capacity };
	using element_t = RealT;
	using my_type = Vector<element_t,N,true>;

	inline Vector( element_t* p ) : m_p(p) { }
	// generic Vector
	inline my_type& operator=( Vector<RealT,Capacity>&& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) = v.m_elem[i]; return *this; } }
	inline my_type& operator=( const Vector<RealT,Capacity>& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) = v.m_elem[i]; return *this; } }
	// my_type
	inline my_type& operator=( my_type&& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) = v.m_elem[i]; return *this; } }
	inline my_type& operator=( const my_type& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) = v.m_elem[i]; return *this; } }

	element_t* data() { return m_p; }
	const element_t* data() const { return m_p; }

	inline element_t& operator[]( uint i ) { return *(m_p+i); }
	inline const element_t& operator[]( uint i ) const { return *(m_p+i); }

	template< bool ViewFlag >
	inline my_type& operator+=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) += v[i]; return *this; } }
	template< bool ViewFlag >
	inline my_type& operator-=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) -= v[i]; return *this; } }
	template< bool ViewFlag >
	inline my_type& operator*=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) *= v[i]; return *this; } }
	template< bool ViewFlag >
	inline my_type& operator/=( const Vector<RealT,Capacity,ViewFlag>& v ) { for( std::size_t i=0; i < N; ++i ) { *(m_p+i) /= v[i]; return *this; } }

	element_t* const m_p;
};

} // namespace superdca

#include "SIMD_intrinsics.h"

namespace superdca {

#ifndef NO_INTRINSICS

#ifdef __AVX__
template<>
struct alignas(32) Vector<double,4,false>
{
	enum { N=4 };
	using element_t = double;
	using simd_t = __m256d;
	using my_type = Vector<element_t,N,false>;

	inline Vector() : m_vec( _mm256_setzero_pd() ) { }
 	inline Vector( Vector<element_t,N>&& v ) noexcept : m_vec(v()) { }
	inline Vector( const Vector<element_t,N>& v ) : m_vec(v()) { }
	inline Vector( simd_t v ) : m_vec(v) { }
	inline Vector( double e ) : m_vec( _mm256_set1_pd(e) ) { }
	inline Vector( double *e ) : m_vec( _mm256_broadcast_sd(e) ) { }

	inline my_type& operator=( Vector<element_t,N>&& v ) { m_vec = v(); return *this; }
	inline my_type& operator=( const Vector<element_t,N>& v ) { m_vec = v(); return *this; }

	inline my_type& operator=( simd_t v ) { m_vec = v; return *this; }

	element_t* data() { return m_elem; }
	const element_t* data() const { return m_elem; }

	simd_t operator()() { return m_vec; }
	simd_t operator()() const { return m_vec; }

	inline element_t& operator[]( uint i ) { return m_elem[i]; }
	inline const element_t& operator[]( uint i ) const { return m_elem[i]; }

	template< bool ViewFlag >
	inline my_type& operator+=( const Vector<element_t,N,ViewFlag>& v ) { m_vec += v(); return *this; }
	template< bool ViewFlag >
	inline my_type& operator-=( const Vector<element_t,N,ViewFlag>& v ) { m_vec -= v(); return *this; }
	template< bool ViewFlag >
	inline my_type& operator*=( const Vector<element_t,N,ViewFlag>& v ) { m_vec *= v(); return *this; }
	template< bool ViewFlag >
	inline my_type& operator/=( const Vector<element_t,N,ViewFlag>& v ) { m_vec /= v(); return *this; }

	inline my_type& operator+=( const simd_t v ) { m_vec += v; return *this; }
	inline my_type& operator-=( const simd_t v ) { m_vec -= v; return *this; }
	inline my_type& operator*=( const simd_t v ) { m_vec *= v; return *this; }
	inline my_type& operator/=( const simd_t v ) { m_vec /= v; return *this; }

	inline my_type& operator+=( element_t e ) { m_vec += _mm256_set1_pd(e); return *this; }
	inline my_type& operator-=( element_t e ) { m_vec -= _mm256_set1_pd(e); return *this; }
	inline my_type& operator*=( element_t e ) { m_vec *= _mm256_set1_pd(e); return *this; }
	inline my_type& operator/=( element_t e ) { m_vec /= _mm256_set1_pd(e); return *this; }

	union
	{
		element_t m_elem[N];
		simd_t m_vec;
	};
};

template<>
struct Vector<double,4,true>
{
	enum { N=4 };
	using element_t = double;
	using simd_t = __m256d;
	using my_type = Vector<element_t,N,true>;

	inline Vector( element_t* p ) : m_p(p) { }
	// generic
	inline my_type& operator=( Vector<element_t,N>&& v ) { this->store( v() ); return *this; }
	inline my_type& operator=( const Vector<element_t,N>& v ) { this->store( v() ); return *this; }
	// my_type
	inline my_type& operator=( my_type&& v ) { this->store( v() ); return *this; }
	inline my_type& operator=( const my_type& v ) { this->store( v() ); return *this; }

	inline my_type& operator=( simd_t v ) { this->store( v ); return *this; }

	element_t* data() { return m_p; }
	const element_t* data() const { return m_p; }

	simd_t operator()() { return _mm256_load_pd(m_p); }
	simd_t operator()() const { return _mm256_load_pd(m_p); }

	inline element_t& operator[]( uint i ) { return *(m_p+i); }
	inline const element_t& operator[]( uint i ) const { return *(m_p+i); }

	void inline store( simd_t vec ) { _mm256_store_pd( m_p, vec ); }

	template< bool ViewFlag >
	inline my_type& operator+=( const Vector<element_t,N,ViewFlag>& v ) { this->store( _mm256_add_pd( (*this)(), v() ) ); return *this; }
	template< bool ViewFlag >
	inline my_type& operator-=( const Vector<element_t,N,ViewFlag>& v ) { this->store( _mm256_sub_pd( (*this)(), v() ) ); return *this; }
	template< bool ViewFlag >
	inline my_type& operator*=( const Vector<element_t,N,ViewFlag>& v ) { this->store( _mm256_mul_pd( (*this)(), v() ) ); return *this; }
	template< bool ViewFlag >
	inline my_type& operator/=( const Vector<element_t,N,ViewFlag>& v ) { this->store( _mm256_div_pd( (*this)(), v() ) ); return *this; }

	inline my_type& operator+=( const simd_t v ) { this->store( _mm256_add_pd( (*this)(), v ) ); return *this; }
	inline my_type& operator-=( const simd_t v ) { this->store( _mm256_sub_pd( (*this)(), v ) ); return *this; }
	inline my_type& operator*=( const simd_t v ) { this->store( _mm256_mul_pd( (*this)(), v ) ); return *this; }
	inline my_type& operator/=( const simd_t v ) { this->store( _mm256_div_pd( (*this)(), v ) ); return *this; }

	inline my_type& operator+=( element_t e ) { this->store( _mm256_add_pd( (*this)(), _mm256_set1_pd(e) ) ); return *this; }
	inline my_type& operator-=( element_t e ) { this->store( _mm256_sub_pd( (*this)(), _mm256_set1_pd(e) ) ); return *this; }
	inline my_type& operator*=( element_t e ) { this->store( _mm256_mul_pd( (*this)(), _mm256_set1_pd(e) ) ); return *this; }
	inline my_type& operator/=( element_t e ) { this->store( _mm256_div_pd( (*this)(), _mm256_set1_pd(e) ) ); return *this; }

	element_t* const m_p;
};

#endif // __AVX__

#endif // #ifndef NO_INTRINSICS

} // namespace superdca

#endif // SUPERDCA_VECTOR_INTERFACE_HPP
