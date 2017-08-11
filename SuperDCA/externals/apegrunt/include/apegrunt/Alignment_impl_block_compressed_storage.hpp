/** @file Alignment_impl_block_compressed_storage.hpp

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

#ifndef APEGRUNT_ALIGNMENT_IMPL_BLOCK_COMPRESSED_STORAGE_HPP
#define APEGRUNT_ALIGNMENT_IMPL_BLOCK_COMPRESSED_STORAGE_HPP

//#include <iosfwd>
#include <cmath>
#include <cfloat>
#include <memory> // for std::enable_shared_from_this
#include <algorithm> // for std::find
#include <iterator> // for std::distance
#include <mutex> // for std::mutex

#include <boost/range/adaptor/indexed.hpp>

#include "Alignment_forward.h"
#include "Alignment_impl_base.hpp"
#include "StateVector_forward.h"

#include "Alignment_parser_forward.h"
#include "Alignment_generator_forward.h"

#include "Alignment_iterator.h"
//#include "Alignment_iterator_impl_block_compressed_storage_forward.h"
#include "Alignment_iterator_impl_block_compressed_storage.hpp"

#include "State_block.hpp"
#include "Index_block.hpp"

namespace apegrunt {

template< typename T >
std::size_t bytesize( const std::vector<T>& v ) { return v.size()*sizeof(T); }

template< typename T, std::size_t VectorSize, std::size_t StateBlockSize >
class Column_frequency_vector_view_accumulator
{
public:
	using frequency_t = T;
	enum { N=VectorSize };
	enum { BlockSize=StateBlockSize };

	using my_type = Column_frequency_vector_view_accumulator<frequency_t,N,BlockSize>;

	Column_frequency_vector_view_accumulator( frequency_t* const data, std::size_t extent )
	: m_data(data), m_extent(extent)
	{ }

	~Column_frequency_vector_view_accumulator() { }

	Column_frequency_vector_view_accumulator( const my_type& other )
	: m_data(other.m_data), m_extent(other.m_extent)
	{ }

	my_type& operator=( const my_type& other )
	{
		m_data = other.m_data;
		m_extent = other.m_extent;
	}

	template< typename StateT, typename IndexContainerT >
	inline void accumulate( apegrunt::State_block<StateT,BlockSize> stateblock, const IndexContainerT& sequence_indices )
	{
		const auto n_seq = frequency_t( sequence_indices.size() );
		for( std::size_t i=0; i < m_extent; ++i )
		{
			const auto state = std::size_t( stateblock[i] );
			*(m_data+i*N+state) += n_seq;
		}
	}

	inline void setZero()
	{
		//vector_t zero;
		for( std::size_t i=0; i < m_extent; ++i )
		{
			for( std::size_t j=0; j < N; ++j )
			{
				*(m_data+i*N+j) = frequency_t(0);
			}
			//vector_view_t( m_data+i*N ) = zero();
		}
	}

private:

	frequency_t* const m_data;
	const std::size_t m_extent;
};

} // namespace apegrunt

namespace apegrunt {

// forward declarations

template< typename AlignmentT > class Alignment_factory;

template< typename StateVectorT > //=StateVector_impl_block_compressed_alignment_storage<nucleic_acid_state_t> >
class Alignment_impl_block_compressed_storage : public Alignment_impl_base< Alignment_impl_block_compressed_storage<StateVectorT>, typename StateVectorT::state_t >
{
public:
	using statevector_t = StateVectorT;
	using state_t = typename statevector_t::state_t;

	using my_type = Alignment_impl_block_compressed_storage<statevector_t>;
	using base_type = Alignment_impl_base<my_type,state_t>;
	using const_iterator = typename base_type::const_iterator;
	using iterator = typename base_type::iterator;
	using value_type = typename base_type::value_type;

	using block_type = typename StateVector<state_t>::block_type;
	enum { N=block_type::N };

	using frequency_t = typename base_type::frequency_t;
	using frequencies_t = typename base_type::frequencies_t;
	using frequencies_ptr = typename base_type::frequencies_ptr;

	using distance_matrix_t = typename base_type::distance_matrix_t;
	using distance_matrix_ptr = typename base_type::distance_matrix_ptr;

	using block_index_t = typename base_type::block_index_t;
	using block_accounting_t = typename base_type::block_accounting_t;
	using block_accounting_ptr = typename base_type::block_accounting_ptr;

	using block_storage_t = typename base_type::block_storage_t;
	using block_storage_ptr = typename base_type::block_storage_ptr;

	Alignment_impl_block_compressed_storage() : m_rows(), m_block_storage( std::make_shared<block_storage_t>() ) { }

	~Alignment_impl_block_compressed_storage() = default;

	Alignment_impl_block_compressed_storage( const my_type& other )
		: base_type( other.id_string() ), m_rows( other.m_rows ), m_block_storage( other.m_block_storage )
	{
	}

	Alignment_impl_block_compressed_storage( my_type&& other ) noexcept
		: base_type( other.id_string() ), m_rows( std::move(other.m_rows) ), m_block_storage(other.m_block_storage) //m_block_storage( std::move(other.m_block_storage) )
	{
	}

	my_type& operator=( const my_type& other )
	{
		this->set_id_string( other.id_string() );
		m_rows = other.m_rows;
		m_block_storage = other.m_block_storage;
		return *this;
	}

	my_type& operator=( my_type&& other ) noexcept
	{
		this->set_id_string( other.id_string() );
		m_rows = std::move( other.m_rows );
		m_block_storage = other.m_block_storage; // std::move( other.m_block_storage );
		return *this;
	}

	Alignment_ptr<state_t> clone() const
	{
		return make_Alignment_ptr( *this );
	}

    inline const_iterator cbegin() const { return const_iterator( std::make_shared<const_iterator_impl>( m_rows.cbegin() ) ); }
    inline const_iterator cend() const { return const_iterator( std::make_shared<const_iterator_impl>( m_rows.cend() ) ); }

    inline const_iterator begin() const { return this->cbegin(); }
    inline const_iterator end() const { return this->cend(); }

    inline iterator begin() { return iterator( std::make_shared<iterator_impl>( m_rows.begin() ) ); }
    inline iterator end() { return iterator( std::make_shared<iterator_impl>( m_rows.end() ) ); }

    inline value_type operator[]( std::size_t index ) const { return m_rows[index]; }

    iterator erase( iterator first, iterator last )
    {
    	using std::cbegin; using std::cend;
    	const auto beg = std::find( cbegin(m_rows), cend(m_rows), *first );
    	const auto end = std::find( beg, cend(m_rows), *last );
    	return iterator( std::make_shared<iterator_impl>( m_rows.erase(beg,end) ) );
    }

    inline std::size_t size() const { return m_rows.size(); }
    inline std::size_t n_loci() const { return m_rows.front()->size(); } // this may not always be safe/produce the correct result

    inline const std::type_info& type() const { return typeid(my_type); }

    inline frequencies_ptr frequencies()
    {
    	m_cache_frequencies_mutex.lock();
    	if( !m_frequencies ) { this->cache_column_frequencies(); }
    	m_cache_frequencies_mutex.unlock();
    	return m_frequencies;
    }

    inline distance_matrix_ptr distance_matrix()
    {
    	m_cache_distance_matrix_mutex.lock();
    	if( !m_distance_matrix ) { this->cache_distance_matrix(); }
    	m_cache_distance_matrix_mutex.unlock();
    	return m_distance_matrix;
    }

    inline Alignment_subscript_proxy< StateVector_ptr<state_t> > subscript_proxy() const { return Alignment_subscript_proxy< StateVector_ptr<state_t> >( &m_rows ); }

	void statistics( std::ostream *out=nullptr ) const
	{
		using std::cbegin; using std::cend;
		if( !out ) { return; }

		const std::size_t n_loci = this->n_loci();
		const std::size_t n_seqs = m_rows.size();
		const std::size_t n_block_column_groups = n_loci / N + ( n_loci % N == 0 ? 0 : 1 );
		const std::size_t indexing_overhead_mem = std::accumulate( cbegin(m_rows), cend(m_rows), std::size_t(0), [=]( std::size_t sum, const auto& seq ) { return sum += seq->bytesize(); } );
		const std::size_t dense_indexing_overhead_mem = sizeof(block_index_t)*n_block_column_groups*n_seqs;

		std::size_t n_cblocks = 0;
		std::size_t min_blocks_per_column = m_block_storage->front().size();
		std::size_t max_blocks_per_column = 0;
		std::vector<std::size_t> col_size; col_size.reserve(m_block_storage->size());
		std::map<std::size_t,std::size_t> nb_bins;
		for( const auto& block_col: *m_block_storage )
		{
			const std::size_t nb = block_col.size();
			col_size.push_back( nb );

			min_blocks_per_column = std::min(min_blocks_per_column,nb);
			max_blocks_per_column = std::max(max_blocks_per_column,nb);
			n_cblocks += nb;
		}

		std::sort( col_size.begin(), col_size.end() );
		const std::size_t n_col_median = col_size[ col_size.size()/2 ];
		const std::size_t n_col_mean = n_cblocks / n_block_column_groups;

		const std::size_t onucs = n_loci*n_seqs;
		const std::size_t cnucs = n_cblocks*N;
		const std::size_t onucs_mem = onucs*sizeof(state_t);
		const std::size_t cnucs_mem = cnucs*sizeof(state_t);

		*out << "apegrunt: alignment has " << n_seqs << " sequences and " << n_loci << " loci (" << onucs << " nucleotides in total)\n";
//		*out << "apegrunt: stateblock compression uses " << n_block_column_groups << " column groups of " << N << "-nucleotide blocks (" << sizeof(block_type) << "B per block)\n";
		*out << "apegrunt: uncompressed size = " << memory_string(onucs_mem) << "\n";
// Compressed blocks -- block-wise lists
		*out << "apegrunt: compressed size = " << memory_string(cnucs_mem+indexing_overhead_mem) << " (" << memory_string(cnucs_mem) << " for " << n_cblocks << " blocks and " << memory_string(indexing_overhead_mem) << " for indexing)\n";
		*out << "apegrunt: compression ratio = " << double(onucs_mem)/double(cnucs_mem+indexing_overhead_mem) << "\n";
		*out << "apegrunt: min/mean/median/max # of stateblocks per column = " << min_blocks_per_column << "/" << n_col_mean << "/" << n_col_median << "/" << max_blocks_per_column << "\n";
	}

	block_accounting_ptr get_block_accounting()
	{
		m_cache_block_accounting_mutex.lock();
		if( !m_block_accounting ) { this->cache_block_accounting(); }
		m_cache_block_accounting_mutex.unlock();
		return m_block_accounting;
	}

	block_storage_ptr get_block_storage() const
	{
		return m_block_storage;
	}

private:
	using iterator_impl = apegrunt::iterator::Alignment_iterator_impl_block_compressed_storage< StateVector_ptr<state_t> >;
	using const_iterator_impl = apegrunt::iterator::Alignment_const_iterator_impl_block_compressed_storage< StateVector_ptr<state_t> >;

	std::vector< StateVector_ptr<state_t> > m_rows;
	block_storage_ptr m_block_storage;
	block_accounting_ptr m_block_accounting;
	frequencies_ptr m_frequencies;
	distance_matrix_ptr m_distance_matrix;
	std::mutex m_cache_frequencies_mutex;
	std::mutex m_cache_distance_matrix_mutex;
	std::mutex m_cache_block_accounting_mutex;

	// pull in all
	void cache_block_accounting()
	{
		//using boost::get;
		using std::cbegin;
		using std::cend;
		m_block_accounting = std::make_shared<block_accounting_t>( m_block_storage->size() );

		std::size_t flat_mem = 0;
		std::size_t compr_mem = 0;

		for( std::size_t col=0; col < m_block_storage->size(); ++col )
		{
			(*m_block_accounting)[col].resize( (*m_block_storage)[col].size() );

			for( std::size_t seq=0; seq < this->size(); ++seq )
			{
				auto query_block = m_rows[seq]->get_block(col);
				auto block_itr = std::find( cbegin( (*m_block_storage)[col] ), cend( (*m_block_storage)[col] ), query_block );
				if( block_itr != cend( (*m_block_storage)[col] ) )
				{
					(*m_block_accounting)[col][ std::distance( cbegin( (*m_block_storage)[col] ), block_itr ) ].push_back( seq );
				}
			}
			for( const auto& index_container: (*m_block_accounting)[col] )
			{
				flat_mem += index_container.size()*sizeof(block_index_t);
				compr_mem += apegrunt::bytesize(index_container);
			}
		}
	}

	void cache_column_frequencies()
	{
		m_frequencies = std::make_shared<frequencies_t>( this->n_loci() );

	    const std::size_t n_loci = this->n_loci(); // number of columns in the alignment
	    const std::size_t n_loci_per_block = apegrunt::StateBlock_size;
	    const std::size_t last_block_size = n_loci % n_loci_per_block == 0 ? n_loci_per_block : n_loci % n_loci_per_block;
	    const std::size_t last_block = n_loci / n_loci_per_block + ( n_loci % n_loci_per_block == 0 ? -1 : 0 );

	    const auto& block_accounting = *(this->get_block_accounting());
		const auto& blocks = *(this->get_block_storage());

		auto& frequencies = *m_frequencies;

		for( std::size_t n_block=0; n_block < last_block+1; ++n_block )
		{
			const auto n_end = ( n_block == last_block ? last_block_size : n_loci_per_block );
			auto&& column_frequency_accumulator = apegrunt::Column_frequency_vector_view_accumulator<typename frequency_t::value_type, number_of_states<state_t>::value, apegrunt::StateBlock_size>( frequencies[n_block*n_loci_per_block].data(), n_end );
			column_frequency_accumulator.setZero();
			const auto& sequence_blocks = blocks[n_block];
			const auto& indices = block_accounting[n_block];

			for( std::size_t block_index=0; block_index < indices.size(); ++block_index )
			{
				column_frequency_accumulator.accumulate( sequence_blocks[block_index], indices[block_index] );
			}
		}
	}

	void cache_distance_matrix()
	{
		const std::size_t n_seqs = this->size();
		m_distance_matrix = std::make_shared<distance_matrix_t>( n_seqs*(n_seqs-1)/2 );

		auto& dmat = *m_distance_matrix;

		// compute matrix of pairwise sequence identities
		for( std::size_t i = 0; i < n_seqs; ++i )
		{
			const auto seq_i = (*this)[i];
			for( std::size_t j = 0; j < i; ++j )
			{
				const std::size_t n_elem = std::min( seq_i->size(), (*this)[j]->size() );
				dmat[i*(i-1)/2+j] = n_elem - ( *seq_i && *(*this)[j] ); // bitwise-AND counts the number of identical positions
			}
		}
	}

	/// Parser interface

	statevector_t* get_new_sequence( const std::string& name )
	{
		//std::cout << "Add new sequence named \"" << name << "\"" << std::endl;

		std::size_t reserve = 0;
		if( m_rows.size() > 0 ) {
			reserve = m_rows.back()->size() / N + ( m_rows.back()->size() % N == 0 ? 0 : 1 );
		}

		m_rows.emplace_back( make_StateVector_ptr<statevector_t>( m_block_storage, name, reserve ) );
		return static_cast<statevector_t*>(m_rows.back().get());
	}

	// Allow parser access to private members
	ALIGNMENT_PARSER_GRAMMAR_FRIENDS(my_type)

	friend class Alignment_factory<my_type>;

	/// Generator interface

	// Allow generator access to private members
	ALIGNMENT_GENERATOR_GRAMMAR_FRIENDS(my_type)

	/// boost.serialization interface.
	friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(boost::serialization::base_object< base_type >(*this));
        ar & BOOST_SERIALIZATION_NVP(m_rows);
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

#endif // APEGRUNT_ALIGNMENT_IMPL_BLOCK_COMPRESSED_STORAGE_HPP

