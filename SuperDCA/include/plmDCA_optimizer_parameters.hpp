/** @file plmDCA_optimizer_parameters.hpp

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

#ifndef SUPERDCA_PLMDCA_OPTIMIZER_PARAMETERS_HPP
#define SUPERDCA_PLMDCA_OPTIMIZER_PARAMETERS_HPP

#include <algorithm>
#include <cstring> // for std::memset

#include <boost/math/special_functions/gamma.hpp>

#include "apegrunt/Apegrunt_utility.hpp"
#include "apegrunt/aligned_allocator.hpp"

#include "Array_view.hpp"
#include "Coupling_matrix_view.hpp"

namespace superdca {

template< typename StateT, typename RealT=double >
std::vector<RealT> calculate_weights( apegrunt::Alignment_ptr<StateT> alignment )
{
	using real_t = RealT;

	auto ali = alignment->subscript_proxy();

	const real_t threshold = plmDCA_options::reweighting_threshold();
	const std::size_t n_seqs = alignment->size();

	std::vector<std::size_t> int_weights; int_weights.reserve( n_seqs );
	for( const auto& seq_ptr: *alignment ) { int_weights.push_back( seq_ptr->multiplicity() ); }

	real_t min_identity = 1.0;
	real_t max_identity = 0.0;

	//std::cout << "Get distance matrix" << std::endl;
	const auto dmat = *(alignment->distance_matrix());
	//std::cout << "  done" << std::endl;

	std::size_t nonidentical_pairs = 0;
	for( std::size_t i = 0; i < n_seqs; ++i )
	{
		const auto seq_i = ali[i]; // (*alignment)[i];
		for( std::size_t j = 0; j < i; ++j )
		{
			//std::cout << "seqs \"" << seq_i->id_string()  << "\" and \"" << (*alignment)[j]->id_string() << "\" are " << fract_identical( seq_i, (*alignment)[j] )*100. << "% identical" << std::endl;
			//if( fract_identical( seq_i, (*alignment)[j] ) >= threshold )
			const std::size_t n_elem = std::min( seq_i->size(), ali[j]->size() );
			const auto identity = real_t( n_elem - dmat[ i*(i-1)/2+j ]) / real_t(n_elem);
			min_identity = std::min( min_identity, identity );
			max_identity = std::max( max_identity, identity );

			if( identity >= threshold )
			{
				//int_weights[i] += (*alignment)[j]->multiplicity();
				int_weights[i] += ali[j]->multiplicity();
				int_weights[j] += seq_i->multiplicity();
			}
			else
			{
				++nonidentical_pairs;
			}
		}
	}

	if( plmDCA_options::verbose() )
	{
		*plmDCA_options::out_stream()
			<< "plmDCA: sequence identity min=" << min_identity << " max=" << max_identity << " reweighting threshold=" << threshold << "\n"
			<< "plmDCA: # of non-identical pairs=" << nonidentical_pairs << "\n";
	}

	std::vector<real_t> weights; weights.reserve(n_seqs);

	using boost::get;
	for( auto&& int_weights_and_seq : zip_range(int_weights,*alignment) ) { weights.push_back( get<1>(int_weights_and_seq)->multiplicity() / real_t( get<0>(int_weights_and_seq) ) ); } // 1/weight * multiplicity

	return weights;
}

template< typename RealT, typename StateT >
class plmDCA_optimizer_parameters
{
public:
	using real_t = RealT;
	using state_t = StateT;
	enum { N=apegrunt::number_of_states<state_t>::value };

	using frequencies_type = std::array< real_t, apegrunt::number_of_states<state_t>::value >;

	using allocator_t = apegrunt::memory::AlignedAllocator< std::array<real_t,N> >;

	using logpots_t = std::vector< std::array<real_t,N>, allocator_t >;
	using nodebels_t = logpots_t;

	using coupling_matrix_view_t = Coupling_matrix_view<real_t,N>;

	using array_view_t = Array_view< real_t, N >;
	using matrix_view_t = Array_view< array_view_t, extent<array_view_t>::value >;

	using matrix_view_array_t = Array_view< matrix_view_t >;
	using array_view_array_t = Array_view< array_view_t >;

	using weights_t = std::shared_ptr< std::vector<real_t> >;
	using frequencies_t = std::shared_ptr< std::vector<frequencies_type> >;

	plmDCA_optimizer_parameters() { }
	~plmDCA_optimizer_parameters() { }

	plmDCA_optimizer_parameters( std::vector< apegrunt::Alignment_ptr<state_t> > alignments, weights_t weights )
	: m_alignments(alignments),
	  m_weights(weights),
	  m_multiplicities(),
	  m_frequencies(),
	  m_nloci(alignments.back()->n_loci()),
	  m_current_column(0),
	  m_logPots(weights->size(),{0}),
	  m_nodeBels(weights->size(),{0}),
	  m_rstates(),
	  m_solution(nullptr),
	  m_gradient(nullptr),
	  m_fvalue(0),
	  m_B_eff(0),
	  m_lambda_J(0),
	  m_lambda_h(0)
	{
		this->init();
		this->cache_rstates();
		//std::cout << std::scientific; for( auto w: m_weights ) { std::cout << " " << w; } std::cout << std::endl;
	}

	plmDCA_optimizer_parameters( plmDCA_optimizer_parameters<real_t,state_t>& other )
	: m_alignments(other.m_alignments),
	  m_weights(other.m_weights),
	  m_multiplicities(other.m_multiplicities),
	  m_frequencies(other.m_frequencies),
	  m_nloci(other.m_nloci),
	  m_current_column(other.m_current_column),
	  m_logPots(other.m_weights->size(),{0}),
	  m_nodeBels(other.m_weights->size(),{0}),
	  m_rstates(),
	  m_solution(nullptr),
	  m_gradient(nullptr),
	  m_fvalue(other.m_fvalue),
	  m_B_eff(other.m_B_eff),
	  m_lambda_J(other.m_lambda_J),
	  m_lambda_h(other.m_lambda_h)
	{
		this->cache_rstates();
		//std::cout << std::scientific; for( auto w: m_weights ) { std::cout << " " << w; } std::cout << std::endl;
	}

	std::size_t get_nloci() const { return m_nloci; }

	std::size_t get_dimensions() const { return this->Jr_size() + this->hr_size(); }

	void set_target_column( std::size_t i ) { m_current_column = i; this->cache_rstates(); }
	std::size_t get_target_column() const { return m_current_column; }

	void set_solution( const real_t *solution )
	{
		if( std::size_t(solution) % 32 != 0 )
		{
			std::cout << "solution storage at " << solution << " is " << ( std::size_t(solution) % 32 == 0 ? "" : "NOT" ) << " 32B aligned" << std::endl;
		}
//		if( solution != m_solution ) { std::cout << "switch solution from addr=" << m_solution << " to addr=" << solution << std::endl; }
		m_solution = const_cast<real_t*>(solution);
	}
	real_t* get_solution() { return m_solution; }

	void set_fvalue( real_t fval ) { m_fvalue=fval; }
	real_t get_fvalue() const { return m_fvalue; }

	void set_gradient( real_t *gradient, bool clear=false )
	{
		if( std::size_t(gradient) % 32 != 0 )
		{
			std::cout << "gradient storage at " << gradient << " is " << ( std::size_t(gradient) % 32 == 0 ? "" : "NOT" ) << " 32B aligned" << std::endl;
		}
		m_gradient = gradient;
		if( clear && m_gradient )
		{
//			std::cout << "clear gradient at=" << m_gradient << " size=" << this->get_dimensions() << std::endl;
			// set gradient vector to zero
			//std::fill( m_gradient, m_gradient+this->get_dimensions(), real_t(0.0) );
			std::memset( m_gradient, 0, this->get_dimensions()*sizeof(real_t) );
		}
	}
	real_t* get_gradient() { return m_gradient; }

	//> SuperDCA::plmDCA interface
	apegrunt::Alignment_ptr<state_t> get_alignment() { return m_alignments.back(); }
//	const std::vector< std::array<std::size_t,N> >& get_allele_occurrence() const { return m_allele_occurrence; }

	std::size_t number_of_alignments() const { return m_alignments.size(); }

	//matrix_view_array_t get_Jr() { return matrix_view_array_t( m_solution, this->n_Jr() ); }
	coupling_matrix_view_t get_Jr_view() { return coupling_matrix_view_t( m_solution, this->n_Jr() ); }
	array_view_t get_hr_view() { return array_view_t( m_solution+this->Jr_size(), this->hr_size() ); }

	//matrix_view_array_t get_Jr( real_t* solution ) { return matrix_view_array_t( solution, this->n_Jr() ); }
	coupling_matrix_view_t get_Jr_view( real_t* const solution ) { return coupling_matrix_view_t( solution, this->n_Jr() ); }
	array_view_t get_hr_view( real_t *solution ) { return array_view_t( solution+this->Jr_size(), this->hr_size() ); }

	void set_weights( weights_t weights ) { m_weights=weights; }
	weights_t& get_weights() { return m_weights; }
	weights_t& get_multiplicities() { return m_multiplicities; }

	//void set_lambda_J( real_t lambda ) { m_lambda_J = lambda; }
	real_t get_lambda_J() const { return m_lambda_J; }

	//void set_lambda_h( real_t lambda ) { m_lambda_h = lambda; }
	real_t get_lambda_h() const { return m_lambda_h; }

	// output parameters
	//matrix_view_array_t get_grad_Jr() { return matrix_view_array_t( m_gradient, this->n_Jr() ); }
	coupling_matrix_view_t get_grad_Jr_view() { return coupling_matrix_view_t( m_gradient, this->n_Jr() ); }
	array_view_t get_grad_hr_view() { return array_view_t( m_gradient+this->Jr_size(), this->hr_size() ); }

	//matrix_view_array_t get_grad_Jr( real_t* gradient ) { return matrix_view_array_t( gradient, this->n_Jr() ); }
	coupling_matrix_view_t get_grad_Jr_view( real_t* gradient ) { return coupling_matrix_view_t( gradient, this->n_Jr() ); }
	array_view_t get_grad_hr_view( real_t* gradient ) { return array_view_t( gradient+this->Jr_size(), this->hr_size() ); }

	std::size_t Jr_size() const { return this->n_Jr()*extent<matrix_view_t>::value*N; }
	std::size_t hr_size() const { return N; }
	std::size_t n_Jr() const { return m_nloci; }

	logpots_t& get_logPots() { return m_logPots; }
	nodebels_t& get_nodeBels() { return m_nodeBels; }

	const std::vector<std::size_t>& get_rstates() const { return m_rstates; }
	const frequencies_t get_frequencies() const { return m_frequencies; }

private:

	void init()
	{
		using apegrunt::cbegin;
		using apegrunt::cend;
		m_B_eff = std::accumulate( cbegin(m_weights), cend(m_weights), 0.0 );

		real_t auto_lambda = ( m_B_eff > 500.0 ) ? 0.1 : 1.0 - ((1.0-0.1) * m_B_eff / 500.0);

		if( plmDCA_options::lambda_J() < 0.0 )
		{
			plmDCA_options::set_lambda_J( auto_lambda/2.0 );
			// Automatic specification of regularization strength based on B_eff. B_eff>500 means the standard regularization 0.1 is used, while B_eff<=500 means a higher regularization is chosen.
			m_lambda_J = auto_lambda * m_B_eff/2.0; // Divide by 2 to keep the size of the coupling regularization equivalent to symmetric variant of plmDCA.
		}
		else
		{
			m_lambda_J = plmDCA_options::lambda_J() * m_B_eff;
		}

		if( plmDCA_options::lambda_h() < 0.0 )
		{
			plmDCA_options::set_lambda_h( auto_lambda );
			m_lambda_h = auto_lambda * m_B_eff;
		}
		else
		{
			m_lambda_h = plmDCA_options::lambda_h() * m_B_eff;
		}

		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream()
				<< "plmDCA: L=" << this->get_alignment()->n_loci() << " n=" << this->get_alignment()->size() << " n(effective)=" << m_B_eff << "\n"
				<< "plmDCA: lambda_J=" << plmDCA_options::lambda_J() << " lambda_h=" << plmDCA_options::lambda_h() << "\n";
		}

		this->cache_frequencies();
		this->cache_multiplicities();
	}

	void cache_rstates()
	{
		const auto& alignment = m_alignments.front();

		const auto n_seqs = alignment->size();
		if( m_rstates.size() < n_seqs ) { m_rstates.resize( n_seqs ); }
		const auto&& ali = alignment->subscript_proxy();
		for( std::size_t i = 0; i < n_seqs; ++i )
		{
			m_rstates[i] = std::size_t( (*ali[i])[m_current_column] );
		}
	}

	void cache_multiplicities()
	{
		const auto& alignment = this->get_alignment();

		m_multiplicities = std::make_shared< std::vector<real_t> >();
		auto& multis = *(m_multiplicities.get());
		const auto n_seqs = alignment->size();
		const auto n_effseqs = alignment->effective_size();
		multis.reserve( n_seqs );

		for( auto seq: alignment )
		{
			multis.push_back( real_t( seq->multiplicity() ) / real_t(n_effseqs) );
		}
	}

	void cache_frequencies()
	{
		const auto& alignment = this->get_alignment();

		m_frequencies = std::make_shared< std::vector<frequencies_type> >();
		auto& freqs = *(m_frequencies.get());
		const auto n_seqs = alignment->size();
		const auto n_effseqs = alignment->effective_size();
		freqs.reserve( n_seqs );

		frequencies_type seqfreq{0};

		//const auto&& alignment = m_alignment->subscript_proxy();
		for( auto seq: alignment )
		{
			//const real_t weight = 1.0 / real_t( seq->size() * n_effseqs );
			const real_t normalize = 1.0 / real_t( seq->size() );
			const auto& fq = seq->frequencies();
			for( std::size_t i=0; i < N; ++i ) { seqfreq[i] = ( real_t(fq[i]) * normalize ); }
			freqs.push_back( seqfreq );
		}
	}

	//> SuperDCA::plmDCA
	//std::vector< std::array<std::size_t,N> > m_allele_occurrence;
	std::vector< apegrunt::Alignment_ptr<state_t> > m_alignments;
	weights_t m_weights;
	weights_t m_multiplicities;
	frequencies_t m_frequencies;
	const std::size_t m_nloci;
	std::size_t m_current_column;

	logpots_t m_logPots;
	nodebels_t m_nodeBels;
	std::vector<std::size_t> m_rstates;

	//> Optimizer interface
	//std::vector<real_t> m_solution;
	real_t *m_solution;
	real_t *m_gradient;
	real_t m_fvalue;

	real_t m_B_eff;
	real_t m_lambda_J;
	real_t m_lambda_h;

	std::vector<real_t> m_logw_sums;
};

} // namespace superdca

#endif // SUPERDCA_PLMDCA_OPTIMIZER_PARAMETERS_HPP

