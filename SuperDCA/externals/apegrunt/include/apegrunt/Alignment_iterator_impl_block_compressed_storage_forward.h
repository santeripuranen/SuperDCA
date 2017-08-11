/** @file Alignment_iterator_impl_block_compressed_storage_forward.h
 
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

#ifndef APEGRUNT_ALIGNMENT_ITERATOR_IMPL_BLOCK_COMPRESSED_STORAGE_FORWARD_H
#define APEGRUNT_ALIGNMENT_ITERATOR_IMPL_BLOCK_COMPRESSED_STORAGE_FORWARD_H

namespace apegrunt {

namespace iterator {

template< typename StateVectorT >
class Alignment_iterator_impl_block_compressed_storage;

template< typename StateVectorT >
class Alignment_const_iterator_impl_block_compressed_storage;

//template< typename StateVectorT >
//using Alignment_const_iterator_impl_block_compressed_storage = Alignment_iterator_impl_block_compressed_storage< StateVectorT >;

} // namespace iterator

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_ITERATOR_IMPL_BLOCK_COMPRESSED_STORAGE_FORWARD_H
