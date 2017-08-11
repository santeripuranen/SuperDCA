/** @file StateVector_iterator_impl_block_compressed_alignment_storage.hpp
 
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

#ifndef APEGRUNT_STATEVECTOR_ITERATOR_IMPL_BLOCK_COMPRESSED_ALIGNMENT_STORAGE_HPP
#define APEGRUNT_STATEVECTOR_ITERATOR_IMPL_BLOCK_COMPRESSED_ALIGNMENT_STORAGE_HPP

#include <cstdint>
#include <functional> // for std::hash
#include <iterator>
//#include <memory> // for std::unique_ptr and std::make_unique

#include "StateVector_iterator.h"
#include "StateVector_iterator_impl_base.hpp"
#include "StateVector_impl_block_compressed_alignment_storage.hpp"
//#include "StateVector_impl_block_compressed_alignment_storage_forward.h"

namespace apegrunt {

namespace iterator {

template< typename StateT, typename ContainerT >
class StateVector_iterator_impl_block_compressed_alignment_storage : public StateVector_iterator_impl_base< StateVector_iterator_impl_block_compressed_alignment_storage<StateT,ContainerT>, StateT >
{
public:
	using state_t = StateT;
	using index_container_t = typename ContainerT::block_index_container_t;
	using block_type = typename ContainerT::block_type;

	using value_type = state_t;
	using reference = value_type; // almost always T& or const T&
    using pointer = const value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using base_t = StateVector_iterator_impl_base< StateVector_iterator_impl_block_compressed_alignment_storage<state_t,ContainerT>, state_t >;
	using my_type = StateVector_iterator_impl_block_compressed_alignment_storage<state_t,ContainerT>;
	using iterator = typename base_t::iterator;
	using const_iterator = typename base_t::const_iterator;

    using pos_t = uint8_t;

	StateVector_iterator_impl_block_compressed_alignment_storage() = delete;
    ~StateVector_iterator_impl_block_compressed_alignment_storage() override = default; // enable objects of derived classes be destructed through StateVector_iterator*

	StateVector_iterator_impl_block_compressed_alignment_storage( my_type&& other )
    : m_cache( std::move(other.m_cache) ),
	  m_pos( std::move(other.m_pos) ),
	  m_container( std::move(other.m_container) ),
	  m_block_col( std::move(other.m_block_col) )
    {
		//std::cout << "move_constructor" << std::endl;
    }

	my_type& operator=( const my_type& rhs )
	{
		//std::cout << "assignment" << std::endl;
		m_cache = rhs.m_cache;
		m_pos = rhs.m_pos;
		m_container = rhs.m_container;
		m_block_col = rhs.m_block_col;
	}

    //StateVector_iterator_impl_block_compressed_alignment_storage( uint8_t pos, typename index_container_t::const_iterator&& index_itr, const ContainerT *container )
    StateVector_iterator_impl_block_compressed_alignment_storage( pos_t pos, std::size_t col, const ContainerT *container )
    //StateVector_iterator_impl_block_compressed_alignment_storage( uint8_t pos, typename index_container_t::const_iterator&& index_itr, StateVector_impl_block_compressed_alignment_storage<state_t> *container )
    : m_pos( pos ), m_container( container ), m_block_col( col )
    {
    	//std::cout << "constructor: m_pos=" << std::size_t(m_pos) << " *m_block_index_itr=" << *m_block_index_itr << " m_container->id_string()=" << m_container->id_string(); std::cout.flush();
    	this->load_into_cache(m_block_col,m_pos);
    	//std::cout << " block=\"" << m_cache << "\"" << std::endl;
    }

	inline const_iterator& operator++() { ++m_pos; if( m_pos == block_type::N ) { ++m_block_col; this->load_into_cache(m_block_col); } return *this; }
    //inline const_iterator operator++(int) { my_type temp(m_position); ++m_position; return temp; }
	//inline const_iterator& operator+=( std::size_t n ) { m_position+=n; return *this; }
    inline reference operator*() const
    {
    	//std::cout << "m_block_index_itr=" << *m_block_index_itr << " &=" << &*m_block_index_itr << " m_pos=" << std::size_t(m_pos) << std::endl;
    	return m_cache[m_pos];
    }
    inline pointer operator->() const { return &(m_cache[m_pos]); }
    inline bool operator==( const my_type& rhs ) const
    {
    	//std::cout << "op==(): " << m_block_col << " v " << rhs.m_block_col << " (itr==" << ( m_block_col == rhs.m_block_col ? "TRUE" : "FALSE" ) << "), " << std::size_t(m_pos) << " v " << std::size_t(rhs.m_pos) << std::endl;
    	return m_block_col == rhs.m_block_col && m_pos == rhs.m_pos;
    }
    inline bool operator<( const my_type& rhs ) const
    {
    	if( m_block_col < rhs.m_block_col ) { return true; }
    	else if( m_block_col == rhs.m_block_col && m_pos < rhs.m_pos ) { return true; }
    	else { return false; }
    }

    inline const std::type_info& type() const { return typeid(my_type); }
    inline std::shared_ptr<my_type> clone() const { return std::make_shared<my_type>( m_pos, m_block_col, m_container ); }

private:
    block_type m_cache;
    pos_t m_pos;
    const ContainerT *m_container;
    std::size_t m_block_col;

    inline void load_into_cache( std::size_t index, pos_t pos=0 )
    {
    	//std::cout << m_cache << " replaced by block(" << index << "): "; std::cout.flush();
    	m_cache = m_container->get_block(index); m_pos=pos;
     	//std::cout << "load" << ( std::size_t(&m_cache) % 32 == 0 ? "" : "u") << "(" << index << ")=" << m_cache << std::endl;
    	//std::cout << m_cache << std::endl;
    }
};

} // namespace iterator

} // namespace apegrunt

#endif // APEGRUNT_STATEVECTOR_ITERATOR_IMPL_BLOCK_COMPRESSED_ALIGNMENT_STORAGE_HPP
