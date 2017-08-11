/** @file State_block.hpp

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

#ifndef APEGRUNT_STATE_BLOCK_HPP
#define APEGRUNT_STATE_BLOCK_HPP

#include <cctype>
#include <functional> // for std::hash

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

#include "StateVector_state_types.hpp"

namespace apegrunt {

template< typename MaskT, uint N, typename StencilT >
constexpr MaskT create_clear( StencilT stencil ) { MaskT mask=0; uint n=0; while( n < N ) { mask = mask | ( MaskT(stencil) << n*8*sizeof(StencilT) ); ++n; } return mask; }

template< typename StateT, uint Size >
//struct alignas(sizeof(void*)) State_block
struct alignas(Size) State_block
{
	using state_t = StateT;
	enum { N=Size };
	enum { CLEAR=uint8_t(state_t::state_t::GAP) };

	using row_t = std::array<state_t,N>;
	using my_type = State_block< StateT, N >;

	row_t m_states;

	State_block() { this->clear(); }
	~State_block() { } // "= default;" won't work here, since the union type makes it non-trivial

	State_block( my_type&& other ) noexcept
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = std::move(other.m_states[i]); } // let the compiler do the unrolling
	}

	State_block( const my_type& other )
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = other.m_states[i]; } // let the compiler do the unrolling
	}

	my_type& operator=( my_type&& other ) noexcept
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = std::move(other.m_states[i]); } // let the compiler do the unrolling
		return *this;
	}

	my_type& operator=( const my_type& other )
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = other.m_states[i]; } // let the compiler do the unrolling
		return *this;
	}

	inline state_t& operator[]( std::size_t pos ) { return m_states[pos]; }
	inline const state_t& operator[]( std::size_t pos ) const { return m_states[pos]; }

	inline void clear() { for( std::size_t i=0; i<N; ++i ) { m_states[i] = CLEAR; } }

	inline bool operator==( const my_type& rhs ) const
	{
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] != rhs.m_states[i] ) { return false; }
		}
		return true;
	}

	inline bool operator<( const my_type& rhs ) const
	{
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] < rhs.m_states[i] ) { return true; }
		}
		return false;
	}
};
/*
template< typename StateT, uint Size >
struct hash< State_block< StateT, Size > >
{
	using my_type = hash< State_block< StateT, Size > >;

	static std::size_t operator()( const my_type& block )
	{
		std::string state_string(Size,char(StateT::state_t::GAP));
		for( std::size_t i=0; i < Size; ++i )
		{
			state_string[i] = char(block[i]);
		}
		return hash<std::string>()(state_string);
	}
};
*/
template< typename StateT >
struct State_block<StateT,8>
{
	using state_t = StateT;
	enum { N=8 };
	enum { CLEAR=create_clear<uint64_t,sizeof(uint64_t)>(uint8_t(state_t::state_t::GAP)) };

	using my_type = State_block< StateT, N >;

	union {
		state_t m_states[N];
		uint64_t m_block;
	};

	State_block( my_type&& other ) noexcept : m_block( std::move(other.m_block) ) { }
	State_block( const my_type& other ) : m_block( other.m_block ) { }

	my_type& operator=( my_type&& other ) noexcept { m_block = std::move(other.m_block); return *this; }
	my_type& operator=( const my_type& other ) { m_block = other.m_block; return *this; }

	inline state_t& operator[]( std::size_t pos ) { return m_states[pos]; }
	inline const state_t& operator[]( std::size_t pos ) const { return m_states[pos]; }
	State_block() : m_block{CLEAR} { }
	~State_block() { } // "= default" won't work here, since the union type apparently makes it non-trivial

	inline void clear() { m_block=CLEAR; }

	inline bool operator==( const my_type& rhs ) const
	{
		return m_block == rhs.m_block;
	}

	inline bool operator<( const my_type& rhs ) const
	{
		return m_block < rhs.m_block;
	}
};
// /*
// */
template< typename StateT, uint Size >
inline bool operator!=( const State_block<StateT,Size>& lhs, const State_block<StateT,Size>& rhs ) { return !(lhs == rhs); }

template< typename StateT, uint Size >
inline bool operator> ( const State_block<StateT,Size>& lhs, const State_block<StateT,Size>& rhs ) { return (rhs < lhs); }

template< typename StateT, uint Size >
inline bool operator<=( const State_block<StateT,Size>& lhs, const State_block<StateT,Size>& rhs ) { return !(lhs > rhs); }

template< typename StateT, uint Size >
inline bool operator>=( const State_block<StateT,Size>& lhs, const State_block<StateT,Size>& rhs ) { return !(lhs < rhs); }

template< typename StateT, uint Size >
std::ostream& operator<< ( std::ostream& os, const State_block<StateT,Size>& block )
{
	for( std::size_t i=0; i < Size; ++i )
	{
		os << char(block[i]);
	}
	return os;
}

template< typename StateT, uint Size >
std::size_t count_identical( State_block<StateT,Size> lhs, State_block<StateT,Size> rhs ) { std::size_t n=0; for( std::size_t i=0; i<Size; ++i ) { lhs[i] == rhs[i] && ++n; } return n; }

} // namespace apegrunt

#include "SIMD_intrinsics.h"

namespace apegrunt {

#ifndef NO_INTRINSICS

#ifdef __SSE2__
template< typename StateT >
struct alignas(16) State_block<StateT,16>
{
	using state_t = StateT;
	using simd_t = __m128i;
	enum { N=16 };
	enum { Nblock=N/sizeof(uint64_t) };
	enum { CLEAR=create_clear<uint64_t,sizeof(uint64_t)>(uint8_t(state_t::state_t::GAP)) };

	using my_type = State_block< state_t, N >;

	union {
		state_t m_states[N];
		uint64_t m_block[Nblock];
	};

	State_block() { this->clear(); }
	~State_block() { } // "= default;" won't work here, since the union type makes it non-trivial

	State_block( my_type&& other ) noexcept { this->store( other() ); }
	State_block( const my_type& other ) { this->store( other() ); }
	my_type& operator=( my_type&& other ) noexcept { this->store( other() ); return *this; }
	my_type& operator=( const my_type& other ) { this->store( other() ); return *this; }

	inline simd_t operator()() { return this->load(); }
	inline const simd_t operator()() const { return this->load(); }

	inline void store( simd_t vec ) { _mm_store_si128( (simd_t*)m_states, vec ); }

	inline simd_t load() { return _mm_load_si128( (simd_t*)m_states ); }
	inline const simd_t load() const { return _mm_load_si128( (simd_t*)m_states ); }

	inline state_t& operator[]( std::size_t pos ) { return m_states[pos]; }
	inline const state_t& operator[]( std::size_t pos ) const { return m_states[pos]; }

	inline void clear() { this->store( _mm_set1_epi8( char(state_t::state_t::GAP) ) ); }

	inline bool operator==( const my_type& rhs ) const
	{
		return m_block[0] == rhs.m_block[0] && m_block[1] == rhs.m_block[1];
	}

	inline bool operator<( const my_type& rhs ) const
	{
		//return m_block[0] < rhs.m_block[0] && m_block[1] < rhs.m_block[1];
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] < rhs.m_states[i] ) { return true; }
		}
		return false;
	}
};
#endif // __SSE2__

#ifdef __AVX__
template< typename StateT >
struct alignas(32) State_block<StateT,32>
//struct State_block<StateT,32>
{
	using state_t = StateT;
	using simd_t = __m256i;
	enum { N=32 };
	enum { Nblock=N/sizeof(uint64_t) };
	enum { CLEAR=create_clear<uint64_t,sizeof(uint64_t)>(uint8_t(state_t::state_t::GAP)) };

	using my_type = State_block< state_t, N >;

	//union alignas(32) {
	union {
		state_t m_states[N];
		uint64_t m_block[Nblock];
	};

	State_block() { this->store( _mm256_set1_epi8( char(state_t::state_t::GAP) ) ); } // std::cout << this << " 32B-aligned: " << ( std::size_t(&m_vec) % 32 == 0 ? "true" : "FALSE") << std::endl;  }
	~State_block() { } // "= default;" won't work here, since the union type makes it non-trivial

	State_block( my_type&& other ) noexcept { this->store( other() ); }
	State_block( const my_type& other ) { this->store( other() ); }
	my_type& operator=( my_type&& other ) noexcept { this->store( other() ); return *this; }
	my_type& operator=( const my_type& other ) { this->store( other() ); return *this; }

	inline simd_t operator()() { return this->load(); }
	inline const simd_t operator()() const { return this->load(); }

	inline void store( simd_t vec )
	{
		_mm256_storeu_si256( (simd_t*)m_states, vec );
		//std::cout << this << " 32B-aligned: " << ( std::size_t(m_states) % 32 == 0 ? "true" : "FALSE") << std::endl;
	}

	//inline simd_t load() { return _mm256_loadu_si256( (simd_t*)m_states ); }
	//inline const simd_t load() const { return _mm256_loadu_si256( (simd_t*)m_states ); }
	inline simd_t load() { return _mm256_lddqu_si256( (simd_t*)m_states ); } // may perform faster than _mm256_loadu_si256 when crossing cacheline boundary
	inline const simd_t load() const { return _mm256_lddqu_si256( (simd_t*)m_states ); } // may perform faster than _mm256_loadu_si256 when crossing cacheline boundary

	inline state_t& operator[]( std::size_t pos ) { return m_states[pos]; }
	inline const state_t& operator[]( std::size_t pos ) const { return m_states[pos]; }

	inline void clear() { this->store( _mm256_set1_epi8( char(state_t::state_t::GAP) ) ); }

	inline bool operator==( const my_type& rhs ) const
	{
		return	m_block[0] == rhs.m_block[0] && m_block[1] == rhs.m_block[1] && m_block[2] == rhs.m_block[2] && m_block[3] == rhs.m_block[3];
	}

	inline bool operator<( const my_type& rhs ) const
	{
		//return m_block[0] < rhs.m_block[0] && m_block[1] < rhs.m_block[1] && m_block[2] < rhs.m_block[2] && m_block[3] < rhs.m_block[3];
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] < rhs.m_states[i] ) { return true; }
		}
		return false;
	}
};

/*
template< typename StateT >
struct alignas(32) State_block<StateT,64>
{
	using state_t = StateT;
	using simd_t = __m256i;
	enum { N=64 };
	enum { Nblock=N/sizeof(uint64_t) };
	enum { CLEAR=create_clear<uint64_t,sizeof(uint64_t)>(uint8_t(state_t::state_t::GAP)) };

	using my_type = State_block< state_t, N >;

	union {
		state_t m_states[N];
		uint64_t m_block[Nblock];
	};

	State_block() { this->store( _mm256_set1_epi8( char(state_t::state_t::GAP) ) ); } // std::cout << this << " 32B-aligned: " << ( std::size_t(&m_vec) % 32 == 0 ? "true" : "FALSE") << std::endl;  }
	~State_block() { } // "= default;" won't work here, since the union type makes it non-trivial

	State_block( my_type&& other ) noexcept { this->store( other() ); }
	State_block( const my_type& other ) { this->store( other() ); }
	my_type& operator=( my_type&& other ) noexcept { this->store( other() ); return *this; }
	my_type& operator=( const my_type& other ) { this->store( other() ); return *this; }

	inline simd_t operator()() { return this->load(); }
	inline const simd_t operator()() const { return this->load(); }

	inline void store( simd_t vec )
	{
		_mm256_storeu_si256( (simd_t*)m_states, vec );
		//std::cout << this << " 32B-aligned: " << ( std::size_t(m_states) % 32 == 0 ? "true" : "FALSE") << std::endl;
	}

	//inline simd_t load() { return _mm256_loadu_si256( (simd_t*)m_states ); }
	//inline const simd_t load() const { return _mm256_loadu_si256( (simd_t*)m_states ); }
	inline simd_t load() { return _mm256_lddqu_si256( (simd_t*)m_states ); } // may perform faster than _mm256_loadu_si256 when crossing cacheline boundary
	inline const simd_t load() const { return _mm256_lddqu_si256( (simd_t*)m_states ); } // may perform faster than _mm256_loadu_si256 when crossing cacheline boundary

	inline state_t& operator[]( std::size_t pos ) { return m_states[pos]; }
	inline const state_t& operator[]( std::size_t pos ) const { return m_states[pos]; }

	inline void clear() { this->store( _mm256_set1_epi8( char(state_t::state_t::GAP) ) ); }

	inline bool operator==( const my_type& rhs ) const
	{
		// _mm256_cmpeq_epi8( (*this)(), rhs() );
		return	m_block[0] == rhs.m_block[0] && m_block[1] == rhs.m_block[1] && m_block[2] == rhs.m_block[2] && m_block[3] == rhs.m_block[3];
		//return _mm256_testc_si256( (*this)(), rhs() );
	}

	inline bool operator<( const my_type& rhs ) const
	{
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] < rhs.m_states[i] ) { return true; }
		}
		return false;
	}
};
*/

#endif // __AVX__

#else // #ifndef NO_INTRINSICS

template< typename StateT >
struct State_block<StateT,16>
{
	using state_t = StateT;
	enum { N=16 };
	enum { Nblock=N/sizeof(uint64_t) };
	enum { CLEAR=create_clear<uint64_t,sizeof(uint64_t)>(uint8_t(state_t::state_t::GAP)) };

	using my_type = State_block< StateT, N >;

	union {
		state_t m_states[N];
		uint64_t m_block[Nblock];
	};

	State_block() : m_block{CLEAR} { } // { this->clear(); }
	~State_block() { } // "= default;" won't work here, since the union type makes it non-trivial

	State_block( my_type&& other ) noexcept
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = std::move(other.m_block[i]); } // let the compiler do the unrolling
	}

	State_block( const my_type& other )
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = other.m_block[i]; } // let the compiler do the unrolling
	}

	my_type& operator=( my_type&& other ) noexcept
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = std::move(other.m_block[i]); } // let the compiler do the unrolling
		return *this;
	}

	my_type& operator=( const my_type& other )
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = other.m_block[i]; } // let the compiler do the unrolling
		return *this;
	}

	inline state_t& operator[]( std::size_t pos ) { return m_states[pos]; }
	inline const state_t& operator[]( std::size_t pos ) const { return m_states[pos]; }

	inline void clear() { for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = CLEAR; } }

	inline bool operator==( const my_type& rhs ) const
	{
		//std::cout << "lhs=" << *this << ( m_block[0] == rhs.m_block[0] && m_block[1] == rhs.m_block[1] ? " == " : " != " ) << "rhs=" << rhs << std::endl;
		return m_block[0] == rhs.m_block[0] && m_block[1] == rhs.m_block[1];
	}

	inline bool operator<( const my_type& rhs ) const
	{
		//return m_block[0] < rhs.m_block[0] && m_block[1] < rhs.m_block[1];
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] < rhs.m_states[i] ) { return true; }
		}
		return false;
	}
};

template< typename StateT >
struct State_block<StateT,32>
{
	using state_t = StateT;
	enum { N=32 };
	enum { Nblock=N/sizeof(uint64_t) };
	enum { CLEAR=create_clear<uint64_t,sizeof(uint64_t)>(uint8_t(state_t::state_t::GAP)) };
	using my_type = State_block< state_t, N >;
	using row_t = std::array< state_t, N >;

	union {
		row_t m_states;
		uint64_t m_block[Nblock];
	};

	State_block() : m_block{CLEAR} { } //{ m_block[0] = CLEAR; m_block[1] = CLEAR; m_block[2] = CLEAR; m_block[3] = CLEAR; }
	~State_block() { } // "= default;" won't work here, since the union type makes it non-trivial

	State_block( my_type&& other ) noexcept
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = std::move(other.m_block[i]); } // let the compiler do the unrolling
		//m_block[0] = std::move(other.m_block[0]); m_block[1] = std::move(other.m_block[1]); m_block[2] = std::move(other.m_block[2]); m_block[3] = std::move(other.m_block[3]);
	}

	State_block( const my_type& other )
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = other.m_block[i]; } // let the compiler do the unrolling
		//m_block[0] = other.m_block[0]; m_block[1] = other.m_block[1]; m_block[2] = other.m_block[2]; m_block[3] = other.m_block[3];
	}

	my_type& operator=( my_type&& other ) noexcept
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = std::move(other.m_block[i]); } // let the compiler do the unrolling
		//m_block[0] = std::move(other.m_block[0]); m_block[1] = std::move(other.m_block[1]);	m_block[2] = std::move(other.m_block[2]); m_block[3] = std::move(other.m_block[3]);
		return *this;
	}

	my_type& operator=( const my_type& other )
	{
		for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] = other.m_block[i]; } // let the compiler do the unrolling
		//m_block[0] = other.m_block[0]; m_block[1] = other.m_block[1]; m_block[2] = other.m_block[2]; m_block[3] = other.m_block[3];
		return *this;
	}

	inline state_t& operator[]( std::size_t pos ) { return m_states[pos]; }
	inline const state_t& operator[]( std::size_t pos ) const { return m_states[pos]; }

	inline void clear() { for( std::size_t i=0; i < Nblock; ++i ) { m_block[i] =CLEAR; } }

	inline bool operator==( const my_type& rhs ) const
	{
		//std::cout << "compare" << std::endl;
		//std::cout << "lhs=" << *this << ( m_block[0] == rhs.m_block[0] && m_block[1] == rhs.m_block[1] ? " == " : " != " ) << "rhs=" << rhs << std::endl;
		return	m_block[0] == rhs.m_block[0] && m_block[1] == rhs.m_block[1] &&	m_block[2] == rhs.m_block[2] &&	m_block[3] == rhs.m_block[3];
	}

	inline bool operator<( const my_type& rhs ) const
	{
		//return m_block[0] < rhs.m_block[0] && m_block[1] < rhs.m_block[1] && m_block[2] < rhs.m_block[2] && m_block[3] < rhs.m_block[3];
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] < rhs.m_states[i] ) { return true; }
		}
		return false;
	}
};

#endif // #ifndef NO_INTRINSICS
// /*

template< typename StateT, std::size_t Size >
//struct alignas(sizeof(void*)) Compressed_state_block
struct alignas(Size*2) Compressed_state_block
{
	using state_t = StateT;
	enum { N=Size*2 };
	enum { CLEAR=uint16_t(0) };

	struct state_and_count
	{
		state_and_count() : block(CLEAR) { }
		~state_and_count() { }

		state_and_count( const state_and_count& other ) : block(other.block) { }
		state_and_count( state_and_count&& other ) noexcept : block(std::move(other.block)) { }

		state_and_count& operator=( const state_and_count& other ) { block = other.block; return *this; }
		state_and_count& operator=( state_and_count&& other ) noexcept { block = std::move(other.block); return *this; }

		union {
			struct {
				state_t state;
				uint8_t n;
			};
			uint16_t block;
		};
	};

	using row_t = std::array<state_and_count,N>;
	using my_type = Compressed_state_block<state_t,N>;

	row_t m_states;
	std::size_t m_pos;

	Compressed_state_block() { this->clear(); }
	~Compressed_state_block() { } // "= default;" won't work here, since the union type makes it non-trivial

	Compressed_state_block( my_type&& other ) noexcept : m_pos(other.m_pos)
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = std::move(other.m_states[i]); } // let the compiler do the unrolling
	}

	Compressed_state_block( const my_type& other ) : m_pos(other.m_pos)
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = other.m_states[i]; } // let the compiler do the unrolling
	}

	Compressed_state_block( const State_block<state_t,Size>& stateblock ) : m_pos(0)
	{
		m_states[m_pos].state = stateblock[0];
		for( std::size_t i=0; i < Size; ++i )
		{
			if( m_states[m_pos].state != stateblock[i] )
			{
				++m_pos;
				m_states[m_pos].state = stateblock[i];
			}
			++(m_states[m_pos].n);
		}
		++m_pos;
		//std::cout << "Compressed_state_block capacity=" << Size << " used=" << m_pos << (m_pos > Size/2 ? " *" : "") << std::endl;
	}

	my_type& operator=( my_type&& other ) noexcept
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = std::move(other.m_states[i]); } // let the compiler do the unrolling
		m_pos = other.m_pos;
		return *this;
	}

	my_type& operator=( const my_type& other )
	{
		for( std::size_t i=0; i < N; ++i ) { m_states[i] = other.m_states[i]; } // let the compiler do the unrolling
		m_pos = other.m_pos;
		return *this;
	}

	inline state_t& operator[]( std::size_t pos ) { std::size_t i=0; std::size_t intpos=0; while( intpos < pos && i < m_pos ) { intpos += m_states[i].n; ++i; } return m_states[i].state; }
	inline const state_t& operator[]( std::size_t pos ) const { std::size_t i=0; std::size_t intpos=0; while( intpos < pos && i < m_pos ) { intpos += m_states[i].n; ++i; } return m_states[i].state; }

	inline void clear() { for( std::size_t i=0; i<N; ++i ) { m_states[i] = CLEAR; } m_pos=0; }

	inline bool operator==( const my_type& rhs ) const
	{
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] != rhs.m_states[i] ) { return false; }
		}
		return true;
	}

	inline bool operator<( const my_type& rhs ) const
	{
		for( std::size_t i=0; i < N; ++i )
		{
			if( m_states[i] < rhs.m_states[i] ) { return true; }
		}
		return false;
	}
};

template< typename StateT, uint Size >
inline bool operator!=( const Compressed_state_block<StateT,Size>& lhs, const Compressed_state_block<StateT,Size>& rhs ) { return !(lhs == rhs); }

template< typename StateT, uint Size >
inline bool operator> ( const Compressed_state_block<StateT,Size>& lhs, const Compressed_state_block<StateT,Size>& rhs ) { return (rhs < lhs); }

template< typename StateT, uint Size >
inline bool operator<=( const Compressed_state_block<StateT,Size>& lhs, const Compressed_state_block<StateT,Size>& rhs ) { return !(lhs > rhs); }

template< typename StateT, uint Size >
inline bool operator>=( const Compressed_state_block<StateT,Size>& lhs, const Compressed_state_block<StateT,Size>& rhs ) { return !(lhs < rhs); }

template< typename StateT, std::size_t Size >
std::ostream& operator<< ( std::ostream& os, const Compressed_state_block<StateT,Size>& block )
{
	os << "[";
	for( std::size_t i=0; i < block.m_pos; ++i )
	{
		os << " " << char(block.m_states[i].state) << ":" << std::size_t(block.m_states[i].n);
	}
	os << " ]";
	return os;
}

// */

} // namespace apegrunt

#endif // APEGRUNT_STATE_BLOCK_HPP

