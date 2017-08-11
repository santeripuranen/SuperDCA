/** @file Coupling_matrix_view.hpp
 
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
#ifndef SUPERDCA_COUPLING_MATRIX_VIEW_HPP
#define SUPERDCA_COUPLING_MATRIX_VIEW_HPP

#include <iterator>
#include <type_traits>

//#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/nvp.hpp>
//#include <boost/range/iterator_range_core.hpp>

#include "Math_utility.hpp"
#include "Array_view_forward.h"
#include "Vector.h"

namespace superdca {

template< typename T, std::size_t SquareMatrixSize, typename StateT, std::size_t Size >
class Coupling_matrix_view_iterator
{
public:
	using real_t = T;
	using state_t = StateT;
	enum { N=SquareMatrixSize };
	enum { BlockSize=Size };

	using my_type = Coupling_matrix_view_iterator<real_t,N,state_t,BlockSize>;

	Coupling_matrix_view_iterator( real_t* const data, std::size_t extent, const apegrunt::State_block<state_t,BlockSize>& stateblock )
	: m_stateblock(stateblock), m_pos(0), m_data(data), m_extent(extent)
	{ }

	~Coupling_matrix_view_iterator() { }

	Coupling_matrix_view_iterator( const my_type& other )
	: m_stateblock(other.m_stateblock), m_pos(other.m_pos), m_data(other.m_data), m_extent(other.m_extent)
	{ }

	my_type& operator=( const my_type& other )
	{
		m_stateblock = other.m_stateblock;
		m_pos = other.m_pos;
		m_data = other.m_data;
		m_extent = other.m_extent;
	}

	// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
	inline real_t* operator*() { return m_data+(std::size_t(m_stateblock[m_pos])*m_extent+m_pos)*N; }
	inline const real_t* operator*() const { return m_data+(std::size_t(m_stateblock[m_pos])*m_extent+m_pos)*N; }

	inline my_type& operator++() { ++m_pos; return *this; }

	inline void reset() { m_pos=0; }

private:
	const apegrunt::State_block<state_t,BlockSize> m_stateblock;
	std::size_t m_pos;
	real_t* const m_data;
	const std::size_t m_extent;
};

template< typename T, std::size_t SquareMatrixSize, std::size_t Size >
class Coupling_matrix_view_accumulator
{
public:
	using real_t = T;
	enum { N=SquareMatrixSize };
	enum { BlockSize=Size };

	using vector_t = Vector<real_t,N>;
	using vector_view_t = Vector<real_t,N,true>;

	using my_type = Coupling_matrix_view_accumulator<real_t,N,BlockSize>;

	Coupling_matrix_view_accumulator( real_t* const data, std::size_t extent )
	: m_data(data), m_extent(extent)
	{ }

	~Coupling_matrix_view_accumulator() { }

	Coupling_matrix_view_accumulator( const my_type& other )
	: m_data(other.m_data), m_extent(other.m_extent)
	{ }

	my_type& operator=( const my_type& other )
	{
		m_data = other.m_data;
		m_extent = other.m_extent;
	}

	template< typename StateT >
	inline vector_t accumulate( apegrunt::State_block<StateT,BlockSize> stateblock ) const
	{
		vector_t thesum;
		for( std::size_t i=0; i < m_extent; ++i )
		{
			// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
			thesum += vector_view_t( m_data+(std::size_t(stateblock[i])*m_extent+i)*N );
		}
		return thesum;
	}

	template< typename StateT >
	inline vector_t accumulate( apegrunt::State_block<StateT,BlockSize> stateblock, std::size_t exclude ) const
	{
		vector_t thesum;
		for( std::size_t i=0; i < m_extent; ++i )
		{
			// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
			if( i != exclude ) { thesum += vector_view_t( m_data+(std::size_t(stateblock[i])*m_extent+i)*N ); }
		}
		return thesum;
	}

	template< typename StateT >
	inline void add_to_matrix_rows( const apegrunt::State_block<StateT,BlockSize>& stateblock, const vector_view_t& v )
	{
		for( std::size_t i=0; i < m_extent; ++i )
		{
			// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
			vector_view_t( m_data+(std::size_t(stateblock[i])*m_extent+i)*N ) += v;
		}
	}

	template< typename StateT >
	inline void add_to_matrix_rows( const apegrunt::State_block<StateT,BlockSize>& stateblock, const vector_view_t& v, std::size_t exclude )
	{
		for( std::size_t i=0; i < m_extent; ++i )
		{
			// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
			if( i != exclude ) { vector_view_t( m_data+(std::size_t(stateblock[i])*m_extent+i)*N ) += v; }
		}
	}
/*
	template< typename StateT >
	inline vector_t operator()( Compressed_state_block<StateT,BlockSize> stateblock ) const
	{
		//std::cout << stateblock << std::endl;
		vector_t thesum;
		for( std::size_t i=0, extent=0, k=0; i < stateblock.m_pos && extent < m_extent; ++i, extent += std::size_t(stateblock.m_states[i].n) )
		//for( std::size_t i=0, k=0; i < stateblock.m_pos && k < m_extent; ++i )
		{
			const auto state = std::size_t( stateblock.m_states[i].state );
			const auto data = m_data+(state*m_extent)*N;
			const auto n = k+std::size_t(stateblock.m_states[i].n);
			//std::cout << "state=" << state << ":" << std::size_t(stateblock.m_states[i].n) << " ";
			for( ; k < n; ++k )
			{
				// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
				//thesum += vector_view_t( m_data+(state*m_extent+j)*N );
				thesum += vector_view_t( data+k*N );
			}
		}
		//std::cout << std::endl;
		return thesum;
	}
*/
private:
	real_t* const m_data;
	const std::size_t m_extent;
};

template< typename T, std::size_t SquareMatrixSize, std::size_t Size, typename RefStatesT, typename WeightsT >
class Pair_frequency_matrix_view_accumulator
{
public:
	using real_t = T;
	enum { N=SquareMatrixSize };
	enum { BlockSize=Size };

	using vector_t = Vector<real_t,N>;
	using vector_view_t = Vector<real_t,N,true>;

	using my_type = Pair_frequency_matrix_view_accumulator<real_t,N,BlockSize,RefStatesT,WeightsT>;

	using reference_states_t = RefStatesT;
	using weights_t = WeightsT;

	Pair_frequency_matrix_view_accumulator( real_t* const data, std::size_t extent, reference_states_t& refstates, weights_t& weights )
	: m_data(data), m_extent(extent), m_refstates(refstates), m_weights(weights)
	{ }

	~Pair_frequency_matrix_view_accumulator() { }

	Pair_frequency_matrix_view_accumulator( const my_type& other )
	: m_data(other.m_data), m_extent(other.m_extent), m_refstates(other.m_refstates), m_weights(other.m_weights)
	{ }

	my_type& operator=( const my_type& other )
	{
		m_data = other.m_data;
		m_extent = other.m_extent;
		m_refstates = other.m_refstates;
		m_weights = other.m_weights;
	}

//	template< typename StateT, typename IndexT >
//	inline void accumulate( apegrunt::State_block<StateT,BlockSize> stateblock, const std::vector<IndexT>& sequence_indices )
	template< typename StateT, typename IndexContainerT >
	inline void accumulate( apegrunt::State_block<StateT,BlockSize> stateblock, const IndexContainerT& sequence_indices )
	{
		for( const auto si: sequence_indices )
		{
			const auto r_state = std::size_t( m_refstates[si] );
			const auto weight = m_weights[si];
			for( std::size_t i=0; i < m_extent; ++i )
			{
				const auto state = std::size_t( stateblock[i] );
				// weighted pair-sums in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
				//*(m_data+(state*m_extent+i)*N+r_state) += weight;
				*(m_data+(state*m_extent+r_state)*N+i) += weight;
			}
		}
	}

//	template< typename StateT, typename IndexT >
//	inline void accumulate( apegrunt::State_block<StateT,BlockSize> stateblock, const std::vector<IndexT>& sequence_indices, std::size_t exclude )
	template< typename StateT, typename IndexContainerT >
	inline void accumulate( apegrunt::State_block<StateT,BlockSize> stateblock, const IndexContainerT& sequence_indices, std::size_t exclude )
	{
		for( auto si: sequence_indices )
		{
			const auto r_state = std::size_t( m_refstates[si] );
			const auto weight = m_weights[si];
			for( std::size_t i=0; i < m_extent; ++i )
			{
				if( i != exclude )
				{
					const auto state = std::size_t( stateblock[i] );
					// weighted pair-sums in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
					//*(m_data+(state*m_extent+i)*N+r_state) += weight;
					*(m_data+(state*m_extent+r_state)*N+i) += weight;
				}
			}
		}
	}

	//template< typename StateT, typename IndexT >
	inline void lgamma_sum( real_t *result, real_t alpha ) const
	{
		using boost::math::lgamma;
		for( std::size_t i=0; i < N; ++i )
		{
			for( std::size_t seq=0; seq < m_extent; ++seq )
			{
				for( std::size_t j=0; j < N; ++j )
				{
					// weighted pair-sums in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
					*(result+seq) += lgamma( *(m_data+(i*m_extent+seq)*N+j) + alpha );
				}
			}
		}
	}

	inline void setZero()
	{
		vector_t zero;
		for( std::size_t state=0; state < N; ++state )
		{
			for( std::size_t i=0; i < m_extent; ++i )
			{
				// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
				vector_view_t( m_data+(std::size_t(state)*m_extent+i)*N ) = zero();
			}
		}
	}

private:

	real_t* const m_data;
	const std::size_t m_extent;
	const reference_states_t& m_refstates;
	const weights_t& m_weights;
};

template< typename T, std::size_t SquareMatrixSize, std::size_t StateBlockSize=apegrunt::StateBlock_size >
class Coupling_matrix_view
{
public:
	using real_t = T;
	enum { N=SquareMatrixSize };
	enum { BlockSize=StateBlockSize };

	using my_type = Coupling_matrix_view<real_t,N>;

	Coupling_matrix_view( real_t* const data, std::size_t extent ) : m_data(data), m_extent(extent)
	{
	}
	~Coupling_matrix_view() { }

	inline real_t* data() { return m_data; }
	inline const real_t* data() const { return m_data; }
	inline std::size_t size() const { return m_extent; }

	inline real_t* get_global( std::size_t matrix, std::size_t row )
	{
		const std::size_t bn = matrix / BlockSize;
		const std::size_t current_block_size = ( m_extent-(matrix+1) < m_extent % BlockSize ? m_extent % BlockSize : BlockSize );

		//	std::cout << "extent=" << m_extent << " matrix=" << matrix << " bn=" << bn << " current_block_size=" << current_block_size << " matrix_within_block=" << matrix-bn*BlockSize << " row=" << row << std::endl;
		//std::cout << "* ";
		return this->get_view_for_block( bn, current_block_size )( matrix-bn*BlockSize, row );
		//return m_data + (matrix/BlockSize)*BlockSize*N*N + (row*N+matrix%BlockSize)*N;
	}
	inline const real_t* get_global( std::size_t matrix, std::size_t row ) const
	{
		const std::size_t bn = matrix / BlockSize;
		const std::size_t current_block_size = ( m_extent-(matrix+1) < m_extent % BlockSize ? m_extent % BlockSize : BlockSize );
		//std::cout << "* ";
		return this->get_view_for_block( bn, current_block_size )( matrix-bn*BlockSize, row );
		//return m_data + (matrix/BlockSize)*BlockSize*N*N + (row*N+matrix%BlockSize)*N;
	}
/*
	// Js in blocks of BlockSize, ordered as [0,1,2... 0,1,2... 0,1,2... 0,1,2...]
	inline real_t* operator()( std::size_t matrix, std::size_t row ) { return m_data+(matrix*N+row)*N; }
	inline const real_t* operator()( std::size_t matrix, std::size_t row ) const { return m_data+(matrix*N+row)*N; }
*/
// /*
	// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
	inline real_t* operator()( std::size_t matrix, std::size_t row ) { /*std::cout << "Matrix_view offset=" << (row*m_extent+matrix)*N << "\n";*/ return m_data+(row*m_extent+matrix)*N; }
	inline const real_t* operator()( std::size_t matrix, std::size_t row ) const { return m_data+(row*m_extent+matrix)*N; }
//*/
	inline my_type get_view_for_block( std::size_t bn, std::size_t block_size=BlockSize )
	{
		const auto data = m_data+bn*block_size*N*N;
		//__builtin_prefetch((const void*)(data),0,3);
		return my_type( data, block_size );
	}

	template< typename StateT >
	inline Coupling_matrix_view_iterator<real_t,N,StateT,BlockSize> get_iterator_for_block( std::size_t bn, std::size_t block_size, const apegrunt::State_block<StateT,BlockSize>& stateblock )
	{
		real_t *const data = m_data+bn*block_size*N*N;
		return Coupling_matrix_view_iterator<real_t,N,StateT,BlockSize>( data, block_size, stateblock );
	}

	inline Coupling_matrix_view_accumulator<real_t,N,BlockSize> get_accumulator_for_block( std::size_t bn, std::size_t block_size )
	{
		real_t *const data = m_data+bn*block_size*N*N;
		return Coupling_matrix_view_accumulator<real_t,N,BlockSize>( data, block_size );
	}

	template< typename RefStatesT, typename WeightsT >
	inline Pair_frequency_matrix_view_accumulator<real_t,N,BlockSize,RefStatesT,WeightsT> get_pair_frequency_accumulator_for_block( std::size_t bn, std::size_t block_size, RefStatesT& refstates, WeightsT& weights )
	{
		real_t *const data = m_data+bn*block_size*N*N;
		return Pair_frequency_matrix_view_accumulator<real_t,N,BlockSize,RefStatesT,WeightsT>( data, block_size, refstates, weights );
	}

private:
	real_t* const m_data;
	const std::size_t m_extent;
};

template< typename RealT, std::size_t SquareMatrixSize >
std::size_t negative_count( const std::array< std::array<RealT,SquareMatrixSize>, SquareMatrixSize >& mat )
{
	std::size_t np=0;
	for( std::size_t row=0; row < SquareMatrixSize; ++row )
	{
		for( std::size_t col=0; col < SquareMatrixSize; ++col )
		{
			mat[row][col] < 0 && ++np;
		}
	}
	return np;
}

template< typename RealT, std::size_t SquareMatrixSize >
std::array< std::array<RealT,SquareMatrixSize>, SquareMatrixSize > convert( Coupling_matrix_view<RealT,SquareMatrixSize>& Js, std::size_t n )
{
	std::array< std::array<RealT,SquareMatrixSize>, SquareMatrixSize > mat{{0}};
	for( std::size_t row=0; row < SquareMatrixSize; ++row )
	{
		Vector<RealT,SquareMatrixSize,true>( mat[row].data() ) = Vector<RealT,SquareMatrixSize,true>( Js.get_global(n,row) );
	}
	return mat;
}

template< typename RealT, std::size_t SquareMatrixSize, typename MatrixViewT >
void copy( Coupling_matrix_view<RealT,SquareMatrixSize>& Js, std::size_t n, MatrixViewT&& mat, bool transpose=false )
{
	//std::cout << "copy" << std::endl;
	//std::array< std::array<RealT,SquareMatrixSize>, SquareMatrixSize > mat{{0}};
	if( transpose )
	{
		for( std::size_t row=0; row < SquareMatrixSize; ++row )
		{
			const auto&& source_row = Vector<RealT,SquareMatrixSize,true>( Js.get_global(n,row) );
			for( std::size_t col=0; col < SquareMatrixSize; ++col )
			{
				*(mat[col].data()+row) = source_row[col];
			}
		}
	}
	else
	{
		for( std::size_t row=0; row < SquareMatrixSize; ++row )
		{
			const auto&& source_row = Vector<RealT,SquareMatrixSize,true>( Js.get_global(n,row) );
			auto dest_row = mat[row].data();
			for( std::size_t col=0; col < SquareMatrixSize; ++col )
			{
				*(dest_row+col) = source_row[col];
			}
			// source and destination are not guaranteed to be properly aligned memory
			//Vector<RealT,SquareMatrixSize,true>( mat[row].data() ) = Vector<RealT,SquareMatrixSize,true>( Js.get_global(n,row) );
		}
	}
}
/*
template< typename RealT, std::size_t SquareMatrixSize >
std::array< std::array<RealT,SquareMatrixSize>, SquareMatrixSize > convert( const Coupling_matrix_view<RealT,SquareMatrixSize>& Js, std::size_t n )
{
	std::array< std::array<RealT,SquareMatrixSize>, SquareMatrixSize > mat;
	for( std::size_t row=0; row < SquareMatrixSize; ++row )
	{
		Vector<RealT,SquareMatrixSize,true>( mat[row].data() ) = Vector<RealT,SquareMatrixSize,true>( Js(n,row) );
	}
	return mat;
}
*/
} // namespace superdca

#endif // SUPERDCA_COUPLING_MATRIX_VIEW_HPP
