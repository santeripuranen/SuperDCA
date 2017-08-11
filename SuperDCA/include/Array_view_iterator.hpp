/** @file Array_view_iterator.hpp
 
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
#ifndef SUPERDCA_ARRAY_VIEW_ITERATOR_HPP
#define SUPERDCA_ARRAY_VIEW_ITERATOR_HPP

#include <iterator>
#include <type_traits>

#include "Array_view_iterator_forward.h"
#include "Math_utility.hpp"
#include "Array_view_forward.h"
#include "Array_view.hpp"

namespace superdca {

template< typename T >
class Array_view_iterator : std::iterator< std::forward_iterator_tag, Array_view_iterator<T> >
{
public:
	Array_view_iterator( typename scalar_type<T>::type *data ) : m_pos(data) { }
	~Array_view_iterator() { }

	using my_type = Array_view_iterator<T>;

	using value_type = T; //almost always T
	using reference = value_type; // almost always T& or const T&; but in this case a reference would be to a temporary Array_view object
    using pointer = value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	inline reference operator*() { return value_type(m_pos); }
	inline reference operator*() const { return value_type(m_pos); }

	inline my_type& operator++() { this->advance(); return *this; }
	inline my_type operator++(int) { value_type tmp(*this); this->advance(); return tmp; }

	inline bool operator==( const my_type& other ) const { return m_pos == other.m_pos; }
	inline bool operator!=( const my_type& other ) const { return m_pos != other.m_pos; }

private:
	typename scalar_type<value_type>::type *m_pos;

	inline void advance(std::size_t i=1) { m_pos+=recursive_extent<T>::value*i; }

};

} // namespace superdca

#endif // SUPERDCA_ARRAY_VIEW_ITERATOR_HPP
