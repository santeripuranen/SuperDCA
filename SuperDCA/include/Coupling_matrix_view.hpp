/** @file Coupling_matrix_view.hpp
 
	Copyright (c) 2016-2018 Santeri Puranen.

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

#include "Math_utility.hpp"
#include "Array_view_forward.h"
#include "Vector.h"

namespace superdca {

template< typename AccessOrder, std::size_t Size, typename RealT >
class Coupling_matrix_view_accumulator
{
public:
	using real_t = RealT;
	enum { N=AccessOrder::N };
	enum { BlockSize=Size };

	using vector_t = Vector<real_t,N>;
	using vector_view_t = Vector<real_t,N,true>;

	using my_type = Coupling_matrix_view_accumulator<AccessOrder,BlockSize,real_t>;

	Coupling_matrix_view_accumulator( real_t* const data, std::size_t extent )
	: m_data(data), m_extent(extent)
	{ }

	~Coupling_matrix_view_accumulator() { }

	Coupling_matrix_view_accumulator( const my_type& other )
	: m_data(other.m_data), m_extent(other.m_extent) //, m_mask(other.m_mask)
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
			const auto state = std::size_t(stateblock[i]);
			thesum += vector_view_t( m_data + AccessOrder::ptr_increment(state,i,m_extent) );
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
			if( i != exclude )
			{
				const auto state = std::size_t(stateblock[i]);
				thesum += vector_view_t( m_data + AccessOrder::ptr_increment(state,i,m_extent) );
			}
		}
		return thesum;
	}

	template< typename StateT >
	inline void add_to_matrix_rows( const apegrunt::State_block<StateT,BlockSize>& stateblock, const vector_view_t& v )
	{
		for( std::size_t i=0; i < m_extent; ++i )
		{
			// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
			const auto state = std::size_t(stateblock[i]);
			vector_view_t( m_data + AccessOrder::ptr_increment(state,i,m_extent) ) += v;
		}
	}

	template< typename StateT >
	inline void add_to_matrix_rows( const apegrunt::State_block<StateT,BlockSize>& stateblock, const vector_view_t& v, std::size_t exclude )
	{
		for( std::size_t i=0; i < m_extent; ++i )
		{
			// Js in blocks of BlockSize, ordered as [0,0,0,0,1,1,1,1,2,2,2,2...]
			if( i != exclude )
			{
				const auto state = std::size_t(stateblock[i]);
				vector_view_t( m_data + AccessOrder::ptr_increment(state,i,m_extent) ) += v;
			}
		}
	}

private:
	real_t* const m_data;
	const std::size_t m_extent;
};

template< typename AccessOrder, std::size_t StateBlockSize, typename RealT >
class Coupling_matrix_view
{
public:
	using real_t = RealT;
	enum { N=AccessOrder::N };
	enum { BlockSize=StateBlockSize };

	using my_type = Coupling_matrix_view<AccessOrder,BlockSize,real_t>;

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
		const std::size_t current_block_size = ( (bn+1)*BlockSize > m_extent ? m_extent % BlockSize : BlockSize );

		return this->get_view_for_block( bn, current_block_size )( matrix-bn*BlockSize, row );
	}
	inline const real_t* get_global( std::size_t matrix, std::size_t row ) const
	{
		const std::size_t bn = matrix / BlockSize;
		const std::size_t current_block_size = ( (bn+1)*BlockSize > m_extent ? m_extent % BlockSize : BlockSize );
		return this->get_view_for_block( bn, current_block_size )( matrix-bn*BlockSize, row );
	}

	// Js in blocks of BlockSize, ordered as defined by AccessOrder
	inline real_t* operator()( std::size_t matrix, std::size_t row ) { return m_data + AccessOrder::ptr_increment(row,matrix,m_extent); }

	inline const real_t* operator()( std::size_t matrix, std::size_t row ) const { return m_data + AccessOrder::ptr_increment(row,matrix,m_extent); }

	inline my_type get_view_for_block( std::size_t bn, std::size_t block_size=BlockSize )
	{
		const auto data = m_data+bn*BlockSize*N*N;
		return my_type( data, block_size );
	}

	inline auto get_accumulator_for_block( std::size_t bn, std::size_t block_size )
	{
		using accumulator_kernel_t = Coupling_matrix_view_accumulator<AccessOrder,BlockSize,real_t>;
		real_t *const data = m_data+bn*BlockSize*N*N;

		return accumulator_kernel_t( data, block_size );
	}

private:
	real_t* const m_data;
	const std::size_t m_extent;
};

template< typename AccessOrder, std::size_t BlockSize, typename RealT >
std::array< std::array<RealT,AccessOrder::N>, AccessOrder::N > convert( Coupling_matrix_view<AccessOrder,BlockSize,RealT>& Js, std::size_t n )
{
	std::array< std::array<RealT,AccessOrder::N>, AccessOrder::N > mat{{0}};
	for( std::size_t row=0; row < AccessOrder::N; ++row )
	{
		Vector<RealT,AccessOrder::N,true>( mat[row].data() ) = Vector<RealT,AccessOrder::N,true>( Js.get_global(n,row) );
	}
	return mat;
}

template< typename AccessOrder, std::size_t BlockSize, typename RealT, typename MatrixViewT >
void copy( Coupling_matrix_view<AccessOrder,BlockSize,RealT>& Js, std::size_t n, MatrixViewT&& mat, bool transpose=false )
{
	if( transpose )
	{
		for( std::size_t row=0; row < AccessOrder::N; ++row )
		{
			const auto&& source_row = Vector<RealT,AccessOrder::N,true>( Js.get_global(n,row) );
			for( std::size_t col=0; col < AccessOrder::N; ++col )
			{
				*(mat[col].data()+row) = source_row[col];
			}
		}
	}
	else
	{
		for( std::size_t row=0; row < AccessOrder::N; ++row )
		{
			const auto&& source_row = Vector<RealT,AccessOrder::N,true>( Js.get_global(n,row) );
			auto dest_row = mat[row].data();
			for( std::size_t col=0; col < AccessOrder::N; ++col )
			{
				*(dest_row+col) = source_row[col];
			}
		}
	}
}

} // namespace superdca

#endif // SUPERDCA_COUPLING_MATRIX_VIEW_HPP
