/** @file Matrix_kernel_access_order.hpp

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

#ifndef SUPERDCA_MATRIX_KERNEL_ACCESS_ORDER_HPP
#define SUPERDCA_MATRIX_KERNEL_ACCESS_ORDER_HPP

namespace superdca {

// s(i,j) = vector i of N states : [ s(0,0), s(0,1), ..., s(0,BlockSize-1), s(1,0), s(1,0), ..., s(1,BlockSize-1), ...,s(N,0), s(N,1), ..., s(N,BlockSize-1) ]
template< std::size_t VectorSize > struct STATES_AccessOrder_tag {
	enum { N=VectorSize };
	static std::size_t ptr_increment( std::size_t state, std::size_t block_pos, std::size_t block_extent=0 )
	{
		return (state*block_extent+block_pos)*VectorSize; // [0,0,0,0,...,1,1,1,1,...,2,2,2,2...]
	}
};

// s(i,j) = vector i of N states : [ s(0,0), s(1,0), ..., s(N,0), s(0,1), s(1,1), ..., s(N,1), ..., s(0,N), s(1,N), ..., s(N,BlockSize-1) ]
template< std::size_t VectorSize > struct MATRICES_AccessOrder_tag {
	enum { N=VectorSize };
	static std::size_t ptr_increment( std::size_t state, std::size_t block_pos, std::size_t block_extent=0 )
	{
		return (state+block_pos*VectorSize)*VectorSize; // [0,1,2,3,0,1,2,3,0,1,2,3,...]
	}
};

} // namespace superdca

#endif // SUPERDCA_MATRIX_KERNEL_ACCESS_ORDER_HPP

