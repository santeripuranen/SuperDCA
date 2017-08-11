/** @file Loci_iterator.hpp
 
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
#ifndef APEGRUNT_LOCI_ITERATOR_HPP
#define APEGRUNT_LOCI_ITERATOR_HPP

#include <string>
#include <functional> // for std::hash
#include <memory> // for std::unique_ptr and std::make_unique

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/range/iterator_range_core.hpp>

#include "Loci_forward.h"

namespace apegrunt {

namespace iterator {

using Loci_const_iterator = std::vector<std::size_t>::const_iterator;
using Loci_iterator = std::vector<std::size_t>::iterator;

/*
// A compositable "Template Method" interface class
class Alignment_iterator
{
public:
	Alignment_iterator() { }
    virtual ~Alignment_iterator() { } // enable derived classes to be destructed through Alignment_iterator_ptr

    //Alignment_iterator_ptr clone() const { return this->clone_impl(); } // clone the object, leaving the callee valid and unmodified.
    //Alignment_iterator_ptr move() { return this->move_impl(); } // implement C++11 move semantics; Moving an instance will invalidate the callee.

    //const std::string& id_string() const { return this->m_get_id_string(); }

    Alignment_iterator begin() const
    {
		return this->begin_impl();
	}

    Alignment_iterator end() const
	{
		return this->end_impl();
	}

private:

    //virtual Alignment_iterator_ptr clone_impl() const = 0;
    //virtual Alignment_iterator_ptr move_impl() const = 0;

    //virtual const std::string& m_get_id_string() const = 0;

    virtual Alignment_iterator begin_impl() const {} // = 0;
    virtual Alignment_iterator end_impl() const {} // = 0;
};
*/
} // namespace iterator

} // namespace apegrunt

#endif // APEGRUNT_LOCI_ITERATOR_HPP
