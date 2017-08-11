/** @file Loci_impl_base.hpp

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

#ifndef APEGRUNT_LOCI_IMPL_BASE_HPP
#define APEGRUNT_LOCI_IMPL_BASE_HPP

#include "Loci.h"

namespace apegrunt {

template< typename LociT >
class Loci_impl_base : public Loci
{
public:
	using const_iterator = Loci::const_iterator;
	using iterator = Loci::iterator;
	using value_type = typename const_iterator::value_type;

	Loci_impl_base() { }
	~Loci_impl_base() { }

	Loci_impl_base( const std::string& id_string ) : m_id_string(id_string) { }

	const std::string& id_string() const { return m_id_string; }
	void set_id_string( const std::string& id_string ) { m_id_string = id_string; }

private:
	using derived_type = LociT;
	using cast_t = derived_type* const;
	using const_cast_t = const derived_type* const;

	std::string m_id_string;

	virtual const_iterator cbegin_impl() const { return static_cast<const_cast_t>(this)->cbegin(); }
	virtual const_iterator cend_impl() const { return static_cast<const_cast_t>(this)->cend(); }

	iterator begin_impl() { return static_cast<cast_t>(this)->begin(); }
	iterator end_impl() { return static_cast<cast_t>(this)->end(); }

    virtual value_type subscript_operator_impl( std::size_t index ) const { return (*static_cast<const_cast_t>(this))[index]; }

    virtual std::size_t size_impl() const { return static_cast<const_cast_t>(this)->size(); }
    virtual void resize_impl( std::size_t size ) { static_cast<cast_t>(this)->resize(size); }

    virtual const std::string& id_string_impl() const { return static_cast<const_cast_t>(this)->id_string(); }
    virtual void set_id_string_impl( const std::string& id_string ) { static_cast<cast_t>(this)->set_id_string(id_string); }

    friend class boost::serialization::access;
	/// boost.serialization interface.
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & BOOST_SERIALIZATION_NVP(m_id_string);
    }
};

} // namespace apegrunt

#endif // APEGRUNT_LOCI_IMPL_BASE_HPP

