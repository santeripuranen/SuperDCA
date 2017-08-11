/** @file Alignment_iterator_impl_block_compressed_storage.hpp
 
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

#ifndef APEGRUNT_ALIGNMENT_ITERATOR_IMPL_BLOCK_COMPRESSED_STORAGE_HPP
#define APEGRUNT_ALIGNMENT_ITERATOR_IMPL_BLOCK_COMPRESSED_STORAGE_HPP

#include <cstdint>
#include <functional> // for std::hash
#include <iterator>
//#include <memory> // for std::unique_ptr and std::make_unique

#include "Alignment_iterator.h"
#include "Alignment_iterator_impl_base.hpp"
#include "Alignment_impl_block_compressed_storage.hpp"

namespace apegrunt {

namespace iterator {

template< typename StateVectorT >
class Alignment_iterator_impl_block_compressed_storage : public Alignment_iterator_impl_base< Alignment_iterator_impl_block_compressed_storage<StateVectorT>, StateVectorT >
{
public:
	using statevector_t = StateVectorT;
	using container_t = std::vector< statevector_t >;

	using value_type = typename container_t::value_type ;
	using reference = value_type&; // almost always T& or const T&
    using pointer = value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using base_t = Alignment_iterator_impl_base< Alignment_iterator_impl_block_compressed_storage<statevector_t>, statevector_t >;
	using my_type = Alignment_iterator_impl_block_compressed_storage<statevector_t>;
	using iterator = typename base_t::iterator;

	Alignment_iterator_impl_block_compressed_storage() { }
    ~Alignment_iterator_impl_block_compressed_storage() override { } // enable objects of derived classes be destructed through StateVector_iterator*

    Alignment_iterator_impl_block_compressed_storage( typename container_t::iterator&& pos ) : m_position( std::move(pos) ) { }

	inline iterator& operator++() { ++m_position; return *this; }
    //inline iterator operator++(int) { my_type temp(m_position); ++m_position; return temp; }
	inline iterator& operator+=( std::size_t n ) { m_position+=n; return *this; }
    inline reference operator*() { return *m_position; }
    inline pointer operator->() { return m_position.operator->(); }
    inline bool operator==( const my_type& rhs ) const { return m_position == rhs.m_position; }
    inline bool operator<( const my_type& rhs ) const { return m_position < rhs.m_position; }

    inline const std::type_info& type() const { return typeid(my_type); }
    inline std::shared_ptr<my_type> clone() const { auto pos = m_position; return std::make_shared<my_type>( std::move(pos) ); }

private:
    typename container_t::iterator m_position;
};

template< typename StateVectorT >
class Alignment_const_iterator_impl_block_compressed_storage : public Alignment_const_iterator_impl_base< Alignment_const_iterator_impl_block_compressed_storage<StateVectorT>, StateVectorT >
{
public:
	using statevector_t = StateVectorT;
	using container_t = std::vector< statevector_t >;

	using value_type = typename container_t::value_type;
	using reference = const value_type&; // almost always T& or const T&
    using pointer = const value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using base_t = Alignment_const_iterator_impl_base< Alignment_const_iterator_impl_block_compressed_storage<statevector_t>, statevector_t >;
	using my_type = Alignment_const_iterator_impl_block_compressed_storage<statevector_t>;
	//using iterator = typename base_t::iterator;
	using const_iterator = typename base_t::const_iterator;

	Alignment_const_iterator_impl_block_compressed_storage() { }
    ~Alignment_const_iterator_impl_block_compressed_storage() override { } // enable objects of derived classes be destructed through StateVector_iterator*

    Alignment_const_iterator_impl_block_compressed_storage( typename container_t::const_iterator&& pos ) : m_position( std::move(pos) ) { }

	inline const_iterator& operator++() { ++m_position; return *this; }
    //inline const_iterator operator++(int) { my_type temp(m_position); ++m_position; return temp; }
	inline const_iterator& operator+=( std::size_t n ) { m_position+=n; return *this; }
    inline reference operator*() const { return *m_position; }
    inline pointer operator->() const { return m_position.operator->(); }
    inline bool operator==( const my_type& rhs ) const { return m_position == rhs.m_position; }
    inline bool operator<( const my_type& rhs ) const { return m_position < rhs.m_position; }

    inline const std::type_info& type() const { return typeid(my_type); }
    inline std::shared_ptr<my_type> clone() const { auto pos = m_position; return std::make_shared<my_type>( std::move(pos) ); }

private:
    typename container_t::const_iterator m_position;
};

} // namespace iterator

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_ITERATOR_IMPL_BLOCK_COMPRESSED_STORAGE_HPP
