/** @file StateVector_impl_block_compressed_alignment_storage.hpp

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

#ifndef APEGRUNT_STATEVECTOR_IMPL_BLOCK_COMPRESSED_ALIGNMENT_STORAGE_HPP
#define APEGRUNT_STATEVECTOR_IMPL_BLOCK_COMPRESSED_ALIGNMENT_STORAGE_HPP

#include <cstdlib> // for std::div

//#include <iosfwd>
#include <vector>
#include <memory> // for std::make_shared
#include <algorithm> // for std::find and std::min

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

#include "StateVector.h"
#include "StateVector_impl_base.hpp"
#include "StateVector_state_types.hpp"
#include "StateVector_iterator.h"
#include "StateVector_iterator_impl_block_compressed_alignment_storage.hpp"

#include "StateVector_parser_forward.h"
//#include "StateVector_generator_forward.h"
#include "StateVector_mutator_forward.h"

#include "State_block.hpp"
#include "Alignment_impl_block_compressed_storage.hpp"
#include "aligned_allocator.hpp"

namespace apegrunt {

template< typename StateT >
class StateVector_impl_block_compressed_alignment_storage : public StateVector_impl_base< StateVector_impl_block_compressed_alignment_storage<StateT>, StateT >
{
public:
	using state_t = StateT;
	using my_type = StateVector_impl_block_compressed_alignment_storage<state_t>;
	using base_type = StateVector_impl_base< my_type, state_t >;
	using block_type = typename StateVector<state_t>::block_type;

	enum { N=block_type::N };

	using parent_t = Alignment_impl_block_compressed_storage< my_type >;

	using block_index_t = uint16_t;
	using block_index_container_t = std::vector< block_index_t >;
	using frequencies_type = std::array< std::size_t, number_of_states<state_t>::value >;

	using allocator_t = memory::AlignedAllocator<block_type,alignof(block_type)>;
	using block_storage_t = std::vector< std::vector< block_type, allocator_t > >;
	using Block_Storage_ptr = std::shared_ptr< block_storage_t >;

	using const_iterator = typename base_type::const_iterator;
	using iterator = typename base_type::iterator ;

	using value_type = typename base_type::value_type;

	//StateVector_impl_block_compressed_alignment_storage() = delete;
	StateVector_impl_block_compressed_alignment_storage() : base_type() { }
	~StateVector_impl_block_compressed_alignment_storage() = default;

	StateVector_impl_block_compressed_alignment_storage( const my_type& other )
	: base_type( other.id_string(), other.multiplicity() ),
	  m_block_storage(other.m_block_storage),
	  m_block_indices(other.m_block_indices),
	  m_cache(other.m_cache),
	  m_frequencies(other.m_frequencies),
	  m_block_col(other.m_block_col),
	  m_pos(other.m_pos),
	  m_size(other.m_size),
	  m_dirty(other.m_dirty)
	{
	}

	StateVector_impl_block_compressed_alignment_storage( my_type&& other ) noexcept
	: base_type( other.id_string(), other.multiplicity() ),
	  m_block_storage( std::move( other.m_block_storage ) ),
	  m_block_indices( std::move( other.m_block_indices ) ),
	  m_cache( std::move( other.m_cache ) ),
	  m_frequencies( std::move( other.m_frequencies ) ),
	  m_block_col(other.m_block_col),
	  m_pos(other.m_pos),
	  m_size(other.m_size),
	  m_dirty(other.m_dirty)
	{
	}

	my_type& operator=( my_type&& other ) noexcept
	{
		this->set_id_string( std::move(other.id_string()) );
		this->set_multiplicity( other.multiplicity() );
		m_block_storage = std::move(other.m_block_storage);
		m_block_indices = std::move(other.m_block_indices);
		m_cache = other.m_cache;
		m_frequencies = other.m_frequencies;
		m_block_col = other.m_block_col;
		m_pos = other.m_pos;
		m_size = other.m_size;
		m_dirty = other.m_dirty;
		return *this;
	}
/*
	my_type& operator=( const my_type& other )
	{
		this->set_id_string( other.id_string() );
		this->set_multiplicity( other.multiplicity() );
		m_block_storage = other.m_block_storage;
		m_block_indices = other.m_block_indices;
		m_cache = other.m_cache;
		m_frequencies = other.m_frequencies;
		m_block_col = other.m_block_col;
		m_pos = other.m_pos;
		m_size = other.m_size;
		m_dirty = other.m_dirty;
		return *this;
	}
*/
	StateVector_impl_block_compressed_alignment_storage( Block_Storage_ptr& block_storage, const std::string& id_string, std::size_t size_hint=0 )
	: base_type( id_string ),
	  m_block_storage( block_storage ),
	  m_block_indices(),
	  m_cache(),
	  m_frequencies{0},
	  m_block_col(0),
	  m_pos(0),
	  m_size(0),
	  m_dirty(false)
	{
		if( size_hint > 0 ) { m_block_indices.reserve(size_hint); }
	}

	inline const_iterator cbegin() const { return const_iterator( std::make_shared<const_iterator_impl>( 0, 0, this ) ); }
    inline const_iterator cend() const { return const_iterator( std::make_shared<const_iterator_impl>( m_pos, (m_pos==0 ? m_block_indices.size() : m_block_indices.size()-1 ), this ) ); }

    inline iterator begin() { return iterator( std::make_shared<iterator_impl>( 0, 0, this ) ); }
    inline iterator end() { return iterator( std::make_shared<iterator_impl>( m_pos, (m_pos==0 ? m_block_indices.size() : m_block_indices.size()-1 ), this ) ); }

    inline value_type operator[]( std::size_t index ) const
    {
    	const auto a = std::div( index, N );
    	return this->get_block(a.quot)[a.rem];
    }

    bool operator==( const my_type& rhs ) const
    {
    	using boost::get;

    	if( this->size() == rhs.size() )
    	{
    		if( m_block_storage == rhs.m_block_storage ) // this and rhs belong to the same Alignment
    		{
    			for( auto index_pair: zip_range( m_block_indices, rhs.m_block_indices ) )
    			{
    				if( get<0>(index_pair) != get<1>(index_pair) ) { return false; } // compare pair indices
    			}
    		}
    		else
    		{
				for( std::size_t i=0; i < m_block_indices.size(); ++i )
				{
					if( this->get_block(i) != rhs.get_block(i) ) { return false; }
				}
    		}
    	}
    	else
    	{
    		return false;
    	}
    	return true;
    }

    inline block_type get_block( std::size_t index ) const
    {
		// guard against invalid block indices from user code..
		if( index < m_block_indices.size() )
		{
			return (*m_block_storage)[index][ m_block_indices[index] ]; // ..but trust that we are internally consistent
		}
		else
		{
			*Apegrunt_options::get_err_stream() << "apegrunt::StateVector: block index=" << index << " out_of_range" << std::endl;
			return block_type();
		}
    }

    inline const frequencies_type& frequencies() const { return m_frequencies; }

	inline std::size_t operator&&( const my_type& rhs ) const
	{
		std::size_t n = 0;

		const std::size_t n_indices = std::min( m_block_indices.size(), rhs.m_block_indices.size() ) -1; // "-1": process the last, potentially partially filled block separately
		if( m_block_storage == rhs.m_block_storage ) // this and rhs belong to the same Alignment
		{
			for( std::size_t i = 0; i < n_indices; ++i )
			{
				if( m_block_indices[i] == rhs.m_block_indices[i] ) { n+=N; } // compare pair indices
				else { n += count_identical( this->get_block(i), rhs.get_block(i) ); }
			}
		}
		else
		{
			for( std::size_t i = 0; i < n_indices; ++i ) // compare pair indices
			{
				n += count_identical( this->get_block(i), rhs.get_block(i) );
			}
		}
		{
			const auto my_block = this->get_block(n_indices); // last common block
			const auto rhs_block = rhs.get_block(n_indices); // last common block
			for( std::size_t i=0; i< std::min(m_pos,rhs.m_pos); ++i ) { my_block[i] == rhs_block[i] && ++n; }
		}
		return n;
	}

	inline std::size_t size() const { return m_size; }

	inline std::size_t bytesize() const { return apegrunt::bytesize(m_block_indices); }

	inline const std::type_info& type() const { return typeid(my_type); }
/*
	inline StateVector_subscript_proxy< State_holder<state_t> > subscript_proxy() const
	{
#pragma message( "WARNING: not implemented!" )
		std::cerr << "\nWARNING: apegrunt::StateVector_impl_block_compressed_alignment_storage::subscript_proxy() NOT implemented.\n" << std::endl;
		return StateVector_subscript_proxy< State_holder<state_t> >( this );
	}
*/
private:
	using iterator_impl = apegrunt::iterator::StateVector_iterator_impl_block_compressed_alignment_storage< State_holder<state_t>, StateVector_impl_block_compressed_alignment_storage<state_t> >;
	using const_iterator_impl = apegrunt::iterator::StateVector_iterator_impl_block_compressed_alignment_storage< State_holder<state_t>, StateVector_impl_block_compressed_alignment_storage<state_t> >;

	template< state_t > friend class apegrunt::iterator::StateVector_iterator_impl_block_compressed_alignment_storage;

	Block_Storage_ptr m_block_storage;
	block_index_container_t m_block_indices;
	block_type m_cache;
	std::array< std::size_t, number_of_states<state_t>::value > m_frequencies;
	std::size_t m_block_col;
	std::size_t m_pos;
	std::size_t m_size;
	bool m_dirty;

	// Parser interface
	template< typename T >
	inline void push_back( const T& state )
	//inline void push_back( const char& state )
	{
		m_cache[m_pos] = state; ++m_pos; m_dirty=true; ++m_size;
		if( N == m_pos ) { this->flush_block_buffer(); }
	}

	bool append( const std::string& state_string )
	{
		m_block_indices.reserve( m_block_indices.size() + state_string.size()/N + (state_string.size() % N == 0 ? 0 : 1) );
		for( auto state: state_string ) { this->push_back( state ); }
		this->flush_block_buffer();
		return true;
	}
	void assign( const std::string& state_string )
	{
		this->clear();
		this->append(state_string); /* this->flush_block_buffer(); */ // don't flush twice, when the implementation uses append()
	}

	//> clear internal state, with the exception of link to parent Alignment
	void clear()
	{
		m_block_indices.clear();
		m_cache.clear();
		m_frequencies.fill(0);
		m_block_col=0;
		m_pos=0;
		m_dirty=false;
		m_size=0;
	}

	inline void update_frequencies()
	{
		for( std::size_t i = 0; i < std::min(m_pos,std::size_t(N)); ++i ) { ++( m_frequencies[std::size_t(m_cache[i])] ); }
	}

	inline void flush_block_buffer()
	{
		if(m_dirty)
		{
			if( m_block_storage->size() < m_block_col+1 ) { m_block_storage->emplace_back( 1, m_cache ); /*m_block_storage->back().reserve(10);*/ m_block_indices.push_back(0); }
			else
			{
				using std::cbegin; using std::cend;
				auto& block_list = (*m_block_storage)[m_block_col];
				auto list_pos = std::find( cbegin(block_list), cend(block_list), m_cache );
				if( list_pos == cend(block_list) )
				{
					block_list.emplace_back( m_cache ); m_block_indices.push_back( block_list.size()-1 );
				}
				else
				{
					m_block_indices.push_back( std::distance( cbegin(block_list), list_pos ) );
				}
			}
			this->update_frequencies();
			if( N  == m_pos ) // test if cache is full or partially filled
			{
				m_cache.clear(); m_pos=0; ++m_block_col;
			}
			m_dirty=false;
		}
	}

	// Allow parser access to private members
	STATEVECTOR_PARSER_GRAMMAR_FRIENDS(my_type) // macro defined in #include "StateVector_parser_forward.h"

	friend class StateVector_mutator<my_type>;

	/// boost.serialization interface.
	friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(boost::serialization::base_object< base_type >(*this));
        ar & BOOST_SERIALIZATION_NVP(m_block_storage);
        ar & BOOST_SERIALIZATION_NVP(m_block_indices);
        ar & BOOST_SERIALIZATION_NVP(m_cache);
        ar & BOOST_SERIALIZATION_NVP(m_frequencies);
        ar & BOOST_SERIALIZATION_NVP(m_pos);
        ar & BOOST_SERIALIZATION_NVP(m_dirty);
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

template<> template< typename StateT >
class StateVector_mutator< StateVector_impl_block_compressed_alignment_storage< StateT > >
{
public:
	using statevector_t = StateVector_impl_block_compressed_alignment_storage< StateT >;
	StateVector_mutator( statevector_t *statevector ) : m_statevector(statevector) { }
	~StateVector_mutator() { m_statevector->flush_block_buffer(); } // flush when done

	template< typename StateT2 >
	void operator()( StateT2 state ) { m_statevector->push_back( state ); }

private:
	statevector_t *m_statevector;
};

/*
template< typename StateT >
class StateVector_subscript_proxy
{
public:
	StateVector_subscript_proxy() { }
	~StateVector_subscript_proxy() { }

	StateVector_subscript_proxy( const std::vector<StateT>* container ) : m_container(container) { }

	inline StateT operator[]( std::size_t index ) const { return (*m_container)[index]; }

private:
	const std::vector<StateT> *m_container;
};
*/
} // namespace apegrunt

#endif // APEGRUNT_STATEVECTOR_IMPL_BLOCK_COMPRESSED_ALIGNMENT_STORAGE_HPP

