/** @file StateVector_iterator_impl_base.hpp
 
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

#ifndef APEGRUNT_STATEVECTOR_ITERATOR_IMPL_BASE_HPP
#define APEGRUNT_STATEVECTOR_ITERATOR_IMPL_BASE_HPP

//#include <iterator>
//#include <memory> // for std::unique_ptr and std::make_unique

//#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/nvp.hpp>
//#include <boost/range/iterator_range_core.hpp>

//#include <boost/iterator.hpp>

#include "StateVector_iterator_interface.hpp"
#include "StateVector_state_types.hpp"

namespace apegrunt {

namespace iterator {
template< typename DerivedT, typename StateT >
class StateVector_iterator_impl_base : public StateVector_iterator_impl< StateT >
{
public:
	using value_type = StateT;
	//using value_type = typename DerivedT::value_type;
	using reference = value_type; // almost always T& or const T&
    using pointer = const value_type*; //almost always T* or const T*
	//using iterator_category = std::random_access_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t; //almost always ptrdiff_t

	using my_type = StateVector_iterator_impl_base<DerivedT,StateT>;

	using interface_t = StateVector_iterator_impl< StateT >;
	using iterator = interface_t;
	using const_iterator = interface_t;

	StateVector_iterator_impl_base() = default;
    virtual ~StateVector_iterator_impl_base() = default;

private:
    using base_t = StateVector_iterator_impl< StateT >;
    using derived_type = DerivedT;
	using cast_t = derived_type* const;
	using const_cast_t = const derived_type* const;
	using ref_t = derived_type&;
	using const_ref_t = const derived_type&;

	const_iterator& pre_increment_operator_impl() override { return ++(static_cast<ref_t>(*this)); }
    //const_iterator post_increment_operator_impl() override { return (*static_cast<cast_t>(this))++; }
    const_iterator& addition_assignment_operator_impl( std::size_t n ) override { return static_cast<ref_t>(*this) += n; }
    reference dereference_operator_impl() const override { /*std::cout << "StateVector_iterator_impl_base::dereference_operator_impl()" << std::endl;*/ return static_cast<const_ref_t>(*this).operator*(); }
    pointer pointer_member_access_operator_impl() const override { return static_cast<const_ref_t>(*this).operator->(); }
	bool eq_operator_impl( const interface_t& rhs ) const override { assert( static_cast<const_ref_t>(*this).type() == rhs.type() ); return static_cast<const_ref_t>(*this) == static_cast<const_ref_t>(rhs); }
    bool lt_operator_impl( const interface_t& rhs ) const override { assert( static_cast<const_ref_t>(*this).type() == rhs.type() ); return static_cast<const_ref_t>(*this) < static_cast<const_ref_t>(rhs); }

    const std::type_info& type_impl() const override { return static_cast<const_ref_t>(*this).type(); }
    std::shared_ptr<base_t> clone_impl() const override { return static_cast<const_ref_t>(*this).clone(); }
};

} // namespace iterator

} // namespace apegrunt

#endif // APEGRUNT_STATEVECTOR_ITERATOR_IMPL_BASE_HPP
