/** @file Alignment_iterator_interface.hpp
 
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

#ifndef APEGRUNT_ALIGNMENT_ITERATOR_INTERFACE_HPP
#define APEGRUNT_ALIGNMENT_ITERATOR_INTERFACE_HPP

#include <cstdint>
#include <iterator>

#include <boost/iterator.hpp>

#include "Alignment_iterator_forward.h"
#include "Alignment_forward.h"
#include "StateVector_forward.h"

namespace apegrunt {

namespace iterator {

// iterator

template< typename StateVectorT >
class Alignment_iterator_impl
{
public:
	using value_type = StateVectorT;
	using reference = value_type&; // almost always T& or const T&
    using pointer = value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using my_type = Alignment_iterator_impl<value_type>;

	Alignment_iterator_impl() { }
    virtual ~Alignment_iterator_impl() { } // enable objects of derived classes be destructed through Alignment_iterator*

    inline my_type& operator++() { return this->pre_increment_operator_impl(); }
    //inline my_type operator++(int) { return this->post_increment_operator_impl(); }
    inline my_type& operator+=( std::size_t n ) { return this->addition_assignment_operator_impl( n ); }
    inline reference operator*() { return this->dereference_operator_impl(); }
    inline pointer operator->() { return this->pointer_member_access_operator_impl(); }
    inline bool operator==( const my_type& rhs ) const { return this->eq_operator_impl( rhs ); }
    inline bool operator<( const my_type& rhs ) const { return this->lt_operator_impl( rhs ); }

    inline const std::type_info& type() const { return this->type_impl(); }
    inline std::shared_ptr<my_type> clone() const { return this->clone_impl(); }

private:
    virtual my_type& pre_increment_operator_impl() = 0;
    //virtual my_type post_increment_operator_impl() = 0;
    virtual my_type& addition_assignment_operator_impl( std::size_t n ) = 0;
    virtual reference dereference_operator_impl() = 0;
    virtual pointer pointer_member_access_operator_impl() = 0;
    virtual bool eq_operator_impl( const my_type& rhs ) const = 0;
    virtual bool lt_operator_impl( const my_type& rhs ) const = 0;

    virtual const std::type_info& type_impl() const = 0;
    virtual std::shared_ptr<my_type> clone_impl() const = 0;
};

template< typename StateVectorT >
inline bool operator!=( const Alignment_iterator_impl<StateVectorT>& lhs, const Alignment_iterator_impl<StateVectorT>& rhs ) { return !(lhs == rhs); }
template< typename StateVectorT >
inline bool operator>( const Alignment_iterator_impl<StateVectorT>& lhs, const Alignment_iterator_impl<StateVectorT>& rhs ) { return (rhs < lhs); }
template< typename StateVectorT >
inline bool operator<=( const Alignment_iterator_impl<StateVectorT>& lhs, const Alignment_iterator_impl<StateVectorT>& rhs ) { return !(lhs > rhs); }
template< typename StateVectorT >
inline bool operator>=( const Alignment_iterator_impl<StateVectorT>& lhs, const Alignment_iterator_impl<StateVectorT>& rhs ) { return !(lhs < rhs); }

template< typename StateT >
class Alignment_iterator // : std::iterator<std::forward_iterator_tag,Alignment>
{
public:
	using state_t = StateT;
	using value_type = StateVector_ptr<state_t>;
	using reference = value_type&; // almost always T& or const T&
    using pointer = value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using my_type = Alignment_iterator;
	using iterator = my_type;

	~Alignment_iterator() { }

	Alignment_iterator( const my_type& other ) : m_impl( other.m_impl->clone() ) { }

	iterator& operator=( const my_type& rhs ) { m_impl = rhs.m_impl->clone(); return *this; }

	template< typename ImplT >
	Alignment_iterator( std::shared_ptr<ImplT>&& impl ) : m_impl( std::move(impl) ) { }

	inline iterator& operator++() { m_impl->operator++(); return *this; }
//    inline const_iterator operator++(int) { (*m_impl)++; return *this; }
    inline iterator& operator+=( std::size_t n ) { (*m_impl) += n; return *this; }
    inline reference operator*() { return m_impl->operator*(); }
    inline pointer operator->() { return m_impl->operator->(); }

    inline bool operator==( const my_type& rhs ) const { return *m_impl == *(rhs.m_impl); }
    inline bool operator<( const my_type& rhs ) const { return *m_impl < *(rhs.m_impl); }

private:
    std::shared_ptr< apegrunt::iterator::Alignment_iterator_impl<value_type> > m_impl;
};

template< typename StateT> inline bool operator!=( const Alignment_iterator<StateT>& lhs, const Alignment_iterator<StateT>& rhs ) { return !(lhs == rhs); }
template< typename StateT> inline bool operator> ( const Alignment_iterator<StateT>& lhs, const Alignment_iterator<StateT>& rhs ) { return (rhs < lhs); }
template< typename StateT> inline bool operator<=( const Alignment_iterator<StateT>& lhs, const Alignment_iterator<StateT>& rhs ) { return !(lhs > rhs); }
template< typename StateT> inline bool operator>=( const Alignment_iterator<StateT>& lhs, const Alignment_iterator<StateT>& rhs ) { return !(lhs < rhs); }

// const iterator

template< typename StateVectorT >
class Alignment_const_iterator_impl
{
public:
	using value_type = StateVectorT;
	using reference = const value_type&; // almost always T& or const T&
    using pointer = const value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using my_type = Alignment_const_iterator_impl<value_type>;

	Alignment_const_iterator_impl() { }
    virtual ~Alignment_const_iterator_impl() { } // enable objects of derived classes be destructed through Alignment_iterator*

    inline my_type& operator++() { return this->pre_increment_operator_impl(); }
    //inline my_type operator++(int) { return this->post_increment_operator_impl(); }
    inline my_type& operator+=( std::size_t n ) { return this->addition_assignment_operator_impl( n ); }
    inline reference operator*() const { return this->dereference_operator_impl(); }
    inline pointer operator->() const { return this->pointer_member_access_operator_impl(); }
    inline bool operator==( const my_type& rhs ) const { return this->eq_operator_impl( rhs ); }
    inline bool operator<( const my_type& rhs ) const { return this->lt_operator_impl( rhs ); }

    inline const std::type_info& type() const { return this->type_impl(); }
    inline std::shared_ptr<my_type> clone() const { return this->clone_impl(); }

private:
    virtual my_type& pre_increment_operator_impl() = 0;
    //virtual my_type post_increment_operator_impl() = 0;
    virtual my_type& addition_assignment_operator_impl( std::size_t n ) = 0;
    virtual reference dereference_operator_impl() const = 0;
    virtual pointer pointer_member_access_operator_impl() const = 0;
    virtual bool eq_operator_impl( const my_type& rhs ) const = 0;
    virtual bool lt_operator_impl( const my_type& rhs ) const = 0;

    virtual const std::type_info& type_impl() const = 0;
    virtual std::shared_ptr<my_type> clone_impl() const = 0;
};

template< typename StateVectorT >
inline bool operator!=( const Alignment_const_iterator_impl<StateVectorT>& lhs, const Alignment_const_iterator_impl<StateVectorT>& rhs ) { return !(lhs == rhs); }
template< typename StateVectorT >
inline bool operator>( const Alignment_const_iterator_impl<StateVectorT>& lhs, const Alignment_const_iterator_impl<StateVectorT>& rhs ) { return (rhs < lhs); }
template< typename StateVectorT >
inline bool operator<=( const Alignment_const_iterator_impl<StateVectorT>& lhs, const Alignment_const_iterator_impl<StateVectorT>& rhs ) { return !(lhs > rhs); }
template< typename StateVectorT >
inline bool operator>=( const Alignment_const_iterator_impl<StateVectorT>& lhs, const Alignment_const_iterator_impl<StateVectorT>& rhs ) { return !(lhs < rhs); }

template< typename StateT >
class Alignment_const_iterator //: std::iterator<std::forward_iterator_tag,Alignment>
{
public:
	using state_t = StateT;
	using value_type = StateVector_ptr<state_t>;
	using reference = const value_type&; // almost always T& or const T&
    using pointer = const value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using my_type = Alignment_const_iterator<state_t>;
	using const_iterator = my_type;

	~Alignment_const_iterator() { }

	Alignment_const_iterator( const my_type& other ) : m_impl( other.m_impl->clone() ) { }

	const_iterator& operator=( const my_type& rhs ) { m_impl = rhs.m_impl->clone(); return *this; }

	template< typename ImplT >
	Alignment_const_iterator( std::shared_ptr<ImplT>&& impl ) : m_impl( std::move(impl) ) { }

	inline const_iterator& operator++() { m_impl->operator++(); return *this; }
//    inline const_iterator operator++(int) { (*m_impl)++; return *this; }
    inline const_iterator& operator+=( std::size_t n ) { (*m_impl) += n; return *this; }
    inline reference operator*() const { return m_impl->operator*(); }
    inline pointer operator->() const { return m_impl->operator->(); }

    inline bool operator==( const my_type& rhs ) const { return *m_impl == *(rhs.m_impl); }
    inline bool operator<( const my_type& rhs ) const { return *m_impl < *(rhs.m_impl); }

private:
    std::shared_ptr< apegrunt::iterator::Alignment_const_iterator_impl<value_type> > m_impl;
};

template< typename StateT > inline bool operator!=( const Alignment_const_iterator<StateT>& lhs, const Alignment_const_iterator<StateT>& rhs ) { return !(lhs == rhs); }
template< typename StateT > inline bool operator> ( const Alignment_const_iterator<StateT>& lhs, const Alignment_const_iterator<StateT>& rhs ) { return (rhs < lhs); }
template< typename StateT > inline bool operator<=( const Alignment_const_iterator<StateT>& lhs, const Alignment_const_iterator<StateT>& rhs ) { return !(lhs > rhs); }
template< typename StateT > inline bool operator>=( const Alignment_const_iterator<StateT>& lhs, const Alignment_const_iterator<StateT>& rhs ) { return !(lhs < rhs); }

} // namespace iterator

} // namespace apegrunt

#endif // APEGRUNT_ALIGNMENT_ITERATOR_INTERFACE_HPP
