/** @file Loci_interface.hpp
 
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
#ifndef APEGRUNT_LOCI_INTERFACE_HPP
#define APEGRUNT_LOCI_INTERFACE_HPP

#include <string>
#include <functional> // for std::hash
//#include <memory> // for std::unique_ptr and std::make_unique

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/range/iterator_range_core.hpp>

#include "Loci_forward.h"
#include "Loci_iterator.hpp"

namespace apegrunt {

// A compositable "Template Method" interface class
class Loci
{
public:
	using const_iterator = apegrunt::iterator::Loci_const_iterator;
	using iterator = apegrunt::iterator::Loci_iterator;
	using value_type = typename const_iterator::value_type;

	Loci() { }
    virtual ~Loci() { } // enable derived classes to be destructed through Loci_ptr

    const_iterator cbegin() const { return this->cbegin_impl(); }
    const_iterator cend() const { return this->cend_impl(); }

    value_type operator[]( std::size_t index ) const { return this->subscript_operator_impl( index ); }

    std::size_t size() const { return this->size_impl(); }

    void resize( std::size_t size ) { this->resize_impl(size); }

    const std::string& id_string() const { return this->id_string_impl(); }
	void set_id_string( const std::string& id_string ) { this->set_id_string_impl(id_string); }

private:
    virtual const_iterator cbegin_impl() const = 0;
    virtual const_iterator cend_impl() const = 0;

    virtual value_type subscript_operator_impl( std::size_t index ) const = 0;

    virtual std::size_t size_impl() const = 0;
    virtual void resize_impl( std::size_t size ) = 0;

    virtual const std::string& id_string_impl() const = 0;
    virtual void set_id_string_impl( const std::string& id_string ) = 0;
};

typename Loci::const_iterator begin( const Loci& loci ) { return loci.cbegin(); }
typename Loci::const_iterator end( const Loci& loci ) { return loci.cend(); }

typename Loci::const_iterator begin( const Loci_ptr& loci ) { return loci->cbegin(); }
typename Loci::const_iterator end( const Loci_ptr& loci ) { return loci->cend(); }

typename Loci::const_iterator cbegin( const Loci& loci ) { return loci.cbegin(); }
typename Loci::const_iterator cend( const Loci& loci ) { return loci.cend(); }

typename Loci::const_iterator cbegin( const Loci_ptr& loci ) { return loci->cbegin(); }
typename Loci::const_iterator cend( const Loci_ptr& loci ) { return loci->cend(); }

std::ostream& operator<< ( std::ostream& os, const Loci* locilist )
{
	for( auto i: *locilist ) { os << " " << i; }
	return os;
}

std::ostream& operator<< ( std::ostream& os, const Loci_ptr& locilist )
{
	os << locilist.get();
	return os;
}

} // namespace apegrunt

#endif // APEGRUNT_LOCI_INTERFACE_HPP
