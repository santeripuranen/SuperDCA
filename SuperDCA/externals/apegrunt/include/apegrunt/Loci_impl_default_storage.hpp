/** @file Loci_impl_default_storage.hpp

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

#ifndef APEGRUNT_LOCI_IMPL_DEFAULT_STORAGE_HPP
#define APEGRUNT_LOCI_IMPL_DEFAULT_STORAGE_HPP

//#include <iosfwd>
#include <vector>
#include <memory> // for std::enable_shared_from_this

//#include <boost/fusion/adapted/adt/adapt_adt.hpp>
//#include <boost/fusion/include/adapt_adt.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

#include "Loci.h"
#include "Loci_impl_base.hpp"
//#include "Loci_iterator_interface.hpp"

namespace apegrunt {

// forward declarations
#ifdef USE_BOOST_SPIRIT_X3
#pragma message("Using boost::spirit x3")
namespace parsers {
namespace Loci_parser_grammar {
// forward declaration of grammar template, needed for friend declaration within the Loci class
template< typename LociT >
class parse;
} // namespace Loci_parser_grammar
} // namespace parsers
#else
// forward declaration of grammar template, needed for friend declaration within the Loci class
namespace parsers {
template< typename IteratorT, typename LociT >
struct Loci_parser_grammar;
} // namespace parsers
#endif

template< typename LociT > class Loci_mutator;

namespace generators {
// forward declaration of grammar template, needed for friend declaration within the Loci class
template< typename IteratorT >
struct Loci_generator_grammar;
} // namespace generators

template< typename T=std::size_t >
class Loci_impl_default_storage : public Loci_impl_base< Loci_impl_default_storage<T> >
{
public:
	using locus_t = T;
	using my_type = Loci_impl_default_storage<locus_t>;
	using base_type = Loci_impl_base< my_type >;
	using const_iterator = typename base_type::const_iterator;
	using iterator = typename base_type::iterator;
	using value_type = typename base_type::value_type;

	Loci_impl_default_storage() : base_type() { }
	~Loci_impl_default_storage() { }

	Loci_impl_default_storage( const my_type& other )
		: base_type( other.id_string() ), m_storage( other.m_storage )
	{
	}

	Loci_impl_default_storage( my_type&& other ) noexcept
		: base_type( std::move(other.id_string()) ), m_storage( std::move( other.m_storage ) )
	{
	}

	using container_t = std::vector<value_type>;

	Loci_impl_default_storage( container_t&& storage ) noexcept
		: base_type(), m_storage( std::move(storage) )
	{
	}

	my_type& operator=( const my_type& other )
	{
		this->set_id_string( other.id_string() );
		m_storage = other.m_storage;
		return *this;
	}

	Loci_impl_default_storage( const std::string& id_string, std::size_t size_hint=0 ) : base_type( id_string )
	{
		if( 0 != size_hint ) { m_storage.reserve(size_hint); }
	}

    const_iterator cbegin() const { return m_storage.cbegin(); }
    const_iterator cend() const { return m_storage.cend(); }

    iterator begin() { return m_storage.begin(); }
    iterator end() { return m_storage.end(); }

    value_type operator[]( std::size_t index ) const { return m_storage[index]; }

	std::size_t size() const { return m_storage.size(); }
	void resize( std::size_t size )
	{
		if( m_storage.size() < size )
		{
			m_storage.reserve(size);
/*
			for( std::size_t i=m_storage.size(); i<size; ++i )
			{
				m_storage.push_back(false);
			}
*/
		}
	}

private:
	container_t m_storage;

	// Parser interface
	void push_back( std::size_t locus, std::size_t base=1 )
	{
		//std::cout << "insert single(" << locus << "/" << base << ")" << std::endl;
/*
		const std::size_t locus_ob = locus-base+1;
		this->resize(locus_ob);
		m_storage[locus_ob-1] = true;
*/
		m_storage.push_back(locus-base);
	}

	// Parser interface
	void push_back( std::size_t locus_begin, std::size_t locus_end, std::size_t base=1 )
	{
		//std::cout << "insert range" << std::endl;
/*
		const std::size_t locus_ob_begin = locus_begin-base+1;
		const std::size_t locus_ob_end = locus_end-base+1;
		this->resize(locus_ob_end);
		for( std::size_t i=locus_ob_begin-1; i<locus_ob_end; ++i )
		{
			m_storage[i] = true;
		}
*/
		for( std::size_t i = locus_begin; i < locus_end+1; ++i )
		{
			m_storage.push_back( i-base );
		}
	}

	//void append( const std::string& loci ) { for( auto state: state_string ) { m_storage.emplace_back( state ); } }
	//void assign( const std::string& loci ) { m_storage.assign( state_string.begin(), state_string.end() ); }

	// Allow parser access to private members
#ifdef USE_BOOST_SPIRIT_X3
	template< my_type > friend class apegrunt::parsers::Loci_parser_grammar::parse;
#else
	template< typename IteratorT, my_type& > friend class apegrunt::parsers::Loci_parser_grammar;
#endif
	friend class Loci_mutator<my_type>;

	/// boost.serialization interface.
	friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(boost::serialization::base_object< base_type >(*this));
        ar & BOOST_SERIALIZATION_NVP(m_storage);
    }

	/// Helper class for memory management thru std::shared_ptr
	class deleter
	{
	public:
		void operator()( my_type* p )
		{
			delete p;
		}
	};
	friend class deleter;

};

} // namespace apegrunt
/*
BOOST_FUSION_ADAPT_ADT(
    apegrunt::StateVector_dense_storage<apegrunt::nucleic_acid_state_t>,
    (obj.get_last_state(), obj.push_back(val))
)
*/

#endif // APEGRUNT_LOCI_IMPL_DEFAULT_STORAGE_HPP

