/** @file plmDCA.hpp
	Top-level include for the plmDCA routine.

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
#ifndef PLMDCA_HPP
#define PLMDCA_HPP

#include <numeric> // for std::accumulate
#include <memory> // for std::shared_ptr and std::make_shared

#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
#pragma message("Compiling with TBB support")
//#include "tbb/tbb.h"
#include "tbb/parallel_reduce.h"
//#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h" // should be included by parallel_for.h
//#include "tbb/mutex.h"
#endif // SUPERDCA_NO_TBB

#include "apegrunt/Alignment.h"
#include "apegrunt/StateVector_utility.hpp"
#include "apegrunt/Alignment_utility.hpp"
#include "apegrunt/Loci.h"
#include "apegrunt/aligned_allocator.hpp"

#include "plmDCA_options.h"
#include "plmDCA_optimizers.hpp"

#include "Stopwatch.hpp"
#include "Matrix_math.hpp"
#include "plmDCA_utility.hpp"
#include "SuperDCA_commons.h"

namespace superdca {

uint64_t get_pool_size( uint64_t n_loci ) { return ipow(n_loci,2)-n_loci; }

template< typename RealT >
struct OptimizerHistory
{
	using real_t = RealT;

	OptimizerHistory( std::size_t history_size )
	{
		// reserve space for optimizer statistics log
		fval_history.resize(history_size);
		nfeval_history.resize(history_size);
	}

	std::vector<real_t> fval_history;
	std::vector<std::size_t> nfeval_history;

};

template< typename RealT, uint States >
class CouplingStorage
{
public:

	enum { Q=States };

	using real_t = RealT;
	using internal_real_t = float;
	using allocator_t = typename apegrunt::memory::AlignedAllocator<internal_real_t>;

	using raw_matrix_t = std::array< internal_real_t, Q*Q >;

	using array_view_t = Array_view< internal_real_t, Q >;
	using matrix_view_t = Array_view< array_view_t, extent<array_view_t>::value >;

	using matrix_view_array_t = Array_view< matrix_view_t >;

	CouplingStorage() : m_dim1(0), m_dim2(0), m_has_dim1_mapping(false), m_has_dim2_mapping(false) { }

	CouplingStorage( apegrunt::Loci_ptr dim1_loci, apegrunt::Loci_ptr dim2_loci )
	: m_dim1(dim1_loci->size()),
	  m_has_dim1_mapping(true),
	  m_dim2(dim2_loci->size()),
	  m_has_dim2_mapping(true)
	{
	    const uint64_t pool_size = m_dim1*m_dim2;

	    {
	    	std::size_t i=0;
	    	for( const auto locus: dim1_loci ) { m_dim1_mapping[locus] = i; ++i; }
	    }
	    {
	    	std::size_t j=0;
	    	for( const auto locus: dim2_loci ) { m_dim2_mapping[locus] = j; ++j; }
	    }

		if( plmDCA_options::verbose() )
		{
			if( plmDCA_options::norm_of_mean_scoring() )
			{
				*plmDCA_options::out_stream()
					<< "plmDCA: computation will require approximately " << apegrunt::memory_string(pool_size*sizeof(raw_matrix_t)) << " of memory\n";
			}
			else
			{
				*plmDCA_options::out_stream()
					<< "plmDCA: computation will require approximately " << apegrunt::memory_string(pool_size*sizeof(internal_real_t)) << " of memory\n";
			}
		}

		this->allocate( pool_size );
	}

	inline std::size_t get_matrix_storage_size() const { return matrix_storage.size(); }
	inline std::size_t get_coupling_storage_size() const { return coupling_storage.size(); }

	inline matrix_view_array_t get_Ji_matrices( std::size_t i ) // zero-based locus index; returns a view to a row vector
	{
		if( m_has_dim1_mapping )
		{
			i = m_dim1_mapping[i];
		}
		assert( i < m_dim1 );
		return matrix_view_array_t( matrix_storage[i*(m_dim1-1)].data(), (m_dim2-1) );
	}

	inline matrix_view_t get_Jij_matrix( std::size_t i, std::size_t j ) // zero-based locus index
	{
		if( m_has_dim1_mapping && m_has_dim2_mapping )
		{
			i = m_dim1_mapping[i];
			j = m_dim2_mapping[j];
		}
		assert(  i < m_dim1 && j < m_dim2 );
		return this->get_Ji_matrices(i)[j]; //coupling_storage[i*(m_dim1-1)+j]; // we store diagonal elements, too, even though they might never be used.
	}

	inline internal_real_t& get_Jij_score( std::size_t i, std::size_t j ) // zero-based locus index
	{
		if( m_has_dim1_mapping )
		{
			i = m_dim1_mapping[i];
			j = m_dim2_mapping[j];
		}
		assert( i < m_dim1 && j < m_dim2 );
		return coupling_storage[i*(m_dim1-1)+j]; // we store diagonal elements, too, even though they might never be used.
	}

private:
    std::vector< raw_matrix_t > matrix_storage;
    std::vector< internal_real_t, allocator_t > coupling_storage;
    std::size_t m_dim1;
    std::size_t m_dim2;

    using loci_mapping_t = std::map<std::size_t,std::size_t>;

    bool m_has_dim1_mapping;
    loci_mapping_t m_dim1_mapping;
    bool m_has_dim2_mapping;
    loci_mapping_t m_dim2_mapping;

	bool allocate( std::size_t pool_size )
	{
		// Allocate matrix memory pool

		try
		{
			if( plmDCA_options::norm_of_mean_scoring() )
			{
				matrix_storage.resize( pool_size );
			}
			else
			{
				coupling_storage.resize( pool_size );
			}
		}
		catch(...)
		{
			*plmDCA_options::err_stream() << "plmDCA error: Exception of unknown type!\n\n";
			Exit(EXIT_FAILURE);
		}
	}

};

template< typename RealT, typename StateT > //, typename OptimizerT >
class plmDCA_solver
{
public:
	using real_t = RealT;
	using state_t = StateT;
    using plmDCA_optimizer_parameters_t = plmDCA_optimizer_parameters<real_t,state_t>;

	plmDCA_solver( std::vector< apegrunt::Alignment_ptr<state_t> > alignments, std::shared_ptr< std::vector<real_t> > weights, CouplingStorage<real_t,apegrunt::number_of_states<state_t>::N>& storage, OptimizerHistory<real_t>& log, std::size_t loci_slice )
    : m_optimizer_parameters( alignments, weights ),
	  m_Jij_storage(storage),
	  m_optimizer_log(log),
	  m_optimizer_objective( m_optimizer_parameters ), // initialize objective
	  m_cputimer( plmDCA_options::verbose() ? plmDCA_options::out_stream() : nullptr ),
	  m_solution( m_optimizer_parameters.get_dimensions(), 0 ),
	  m_loci_slice( loci_slice ),
	  m_no_estimate( plmDCA_options::no_estimate() ),
	  m_no_dca( plmDCA_options::no_dca() )
	{
		auto control = cppoptlib::Criteria<real_t>();
		control.iterations = 2000;
		control.gradNorm = plmDCA_options::gradient_threshold(); // default = 1e-4; lower than 1e-3 will converge very slowly with large number of parameters
		m_optimizer.setStopCriteria( control );

		// override default optimizer parameters
		//auto& control = m_optimizer.ctrl();
		//control.iterations = 2000; // default = 100000
		//control.m = 10; // default = 10
		//control.rate = 0.005; // default = 0.00005
		//control.gradNorm = plmDCA_options::gradient_threshold(); // default = 1e-4; lower than 1e-3 will converge very slowly with large number of parameters
	}

	plmDCA_solver( plmDCA_solver<real_t,state_t>&& other )
    : m_optimizer_parameters( other.m_optimizer_parameters ),
	  m_Jij_storage( other.m_Jij_storage ),
	  m_optimizer_log( other.m_optimizer_log ),
	  m_optimizer_objective( m_optimizer_parameters ), // initialize objective
	  m_cputimer( other.m_cputimer ),
	  m_solution( std::move( other.m_solution ) ), // each instance has its own private solution vector
	  //m_optimizer( other.m_optimizer.criteria() ),
	  m_loci_slice( std::move( other.m_loci_slice ) ),
	  m_no_estimate( other.m_no_estimate ),
	  m_no_dca( other.m_no_dca )
	{
		//m_optimizer.setStopCriteria( other.m_optimizer.criteria() );
		auto control = cppoptlib::Criteria<real_t>();
		control.iterations = 2000;
		control.gradNorm = plmDCA_options::gradient_threshold(); // default = 1e-4; lower than 1e-3 will converge very slowly with large number of parameters
		m_optimizer.setStopCriteria( control );
	}
#ifndef SUPERDCA_NO_TBB
    // TBB interface (Requirements for tbb::parallel_reduce Body)
	template< typename TBBSplitT >
	plmDCA_solver( plmDCA_solver<real_t,state_t>& other, TBBSplitT s )
    : m_optimizer_parameters( other.m_optimizer_parameters ),
	  m_Jij_storage( other.m_Jij_storage ),
	  m_optimizer_log( other.m_optimizer_log ),
	  m_optimizer_objective( m_optimizer_parameters ), // initialize objective
	  m_cputimer( other.m_cputimer ),
	  m_solution( other.m_solution.size(), 0 ), // each instance has its own private solution vector
	  //m_optimizer( other.m_optimizer.criteria() ),
	  m_loci_slice( other.m_loci_slice ),
	  m_no_estimate( other.m_no_estimate ),
	  m_no_dca( other.m_no_dca )
	{
		//m_optimizer.setStopCriteria( other.m_optimizer.criteria() );
		auto control = cppoptlib::Criteria<real_t>();
		control.iterations = 2000;
		control.gradNorm = plmDCA_options::gradient_threshold(); // default = 1e-4; lower than 1e-3 will converge very slowly with large number of parameters
		m_optimizer.setStopCriteria( control );
	}

	// TBB interface (required by tbb::parallel_reduce Body)
	void join( plmDCA_solver<real_t,state_t>& rhs )
	{
	}
#endif // #ifndef SUPERDCA_NO_TBB

	template< typename RangeT >
    inline void operator()( const RangeT& index_range )
    {

		const std::size_t n_loci = m_optimizer_parameters.get_alignment()->n_loci(); // cache the number of loci

		// Map std::vector to an Eigen::Matrix -- required by CppNumericalSolvers
		Eigen::Matrix<real_t, Eigen::Dynamic, 1> solution = Eigen::Map< Eigen::Matrix<real_t, Eigen::Dynamic, 1> >( m_solution.data(), m_solution.size() );

		stopwatch::stopwatch estimatetimer;
		stopwatch::stopwatch dcatimer;

		for( const auto r: index_range )
		{
			m_optimizer_parameters.set_target_column(r);

			std::ostringstream estimate_elapsed_time;
			std::ostringstream bf_elapsed_time;
			std::ostringstream dca_statistics;
			m_cputimer.start();

			solution.setZero();

			// /* CppNumericalSolvers
			if( !m_no_dca )
			{
				dcatimer.start();
				m_optimizer.minimize( m_optimizer_objective, solution );
				dcatimer.stop();
				m_optimizer_log.fval_history[r] = m_optimizer_parameters.get_fvalue();
				m_optimizer_log.nfeval_history[r] = m_optimizer_objective.get_nfeval();
				m_optimizer_objective.reset_counters();
				auto& info = m_optimizer.criteria();
				dca_statistics
					<< "fval=" << std::scientific << m_optimizer_log.fval_history[r]
					<< " gnorm=" << info.gradNorm
					<< " nfeval=" << m_optimizer_log.nfeval_history[r]
					<< " iter=" << info.iterations
					<< " dca=" << dcatimer;
			}

			m_cputimer.stop();

			if( plmDCA_options::verbose() )
			{
				*plmDCA_options::out_stream() << "  " << r+1 << " / " << m_loci_slice << " (out of " << n_loci << ") locus=" << (*(m_optimizer_parameters.get_alignment()->get_loci_translation()))[r]+1
					<< ( m_no_dca ? "" : " " + dca_statistics.str() )
					<< ( m_no_estimate ? "" : " estimate=" + estimate_elapsed_time.str() )
					<< " total=" << m_cputimer << "\n";
			}

			// Store all solutions (parameter matrices)

			auto&& Jr_solution = m_optimizer_parameters.get_Jr_view(solution.data());

			// Either:

			// a) store full q-by-q Jij matrices
			if( plmDCA_options::norm_of_mean_scoring() )
			{
				//const auto&& solution = Jr_solution[n];
				if( m_optimizer_parameters.number_of_alignments() > 1 )
				{
					for( std::size_t n=0; n < Jr_solution.size(); ++n )
					{
						auto&& coupling_ij_matrix = m_Jij_storage.get_Jij_matrix(r,n); // Jr_solution does not store self-interaction/diagonal element, but we can access the matrix as if it does
						copy( Jr_solution, n, coupling_ij_matrix, false ); // false = do not transpose
						//copy( gauge_shift( Jr_solution, n ), coupling_ij_matrix );
					}
				}
				else
				{
					// lower-triangular element matrices
					for( std::size_t n=0; n < r; ++n )
					{
						auto&& coupling_ij_matrix = m_Jij_storage.get_Jij_matrix(r,n);
						// transpose the upper-triangular element matrices
						copy( Jr_solution, n, coupling_ij_matrix, true ); // true = transpose
					}
					// upper-triangular element matrices
					for( std::size_t n=r+1; n < Jr_solution.size(); ++n )
					{
						auto&& coupling_ij_matrix = m_Jij_storage.get_Jij_matrix(r,n);
						copy( Jr_solution, n, coupling_ij_matrix, false ); // false = do not transpose
					}
				}
			}

			// b) store the norms of Jij matrices, discarding the full q-by-q Jij matrices
			else
			{
				//const auto&& Jr_solution = m_optimizer_parameters.get_initial_Jr();

				if( m_optimizer_parameters.number_of_alignments() > 1 )
				{
					for( std::size_t n=0; n < Jr_solution.size(); ++n )
					{
						auto& coupling_ij = m_Jij_storage.get_Jij_score(r,n);
						coupling_ij = frobenius_norm( gauge_shift( Jr_solution, n ), std::size_t(state_t::GAP) );
					}
				}
				else
				{
					// lower-triangular element matrices
					for( std::size_t n=0; n < r; ++n )
					{
						auto& coupling_ij = m_Jij_storage.get_Jij_score(r,n);
						// transpose the lower-triangular element matrices
						coupling_ij = frobenius_norm( gauge_shift( Jr_solution, n, true ), std::size_t(state_t::GAP) );
					}
					// upper-triangular element matrices
					for( std::size_t n=r+1; n <	Jr_solution.size(); ++n )
					{
						auto& coupling_ij = m_Jij_storage.get_Jij_score(r,n);
						coupling_ij = frobenius_norm( gauge_shift( Jr_solution, n ), std::size_t(state_t::GAP) );
					}
				}
			}
		}
	}

	void set_no_estimate( bool flag ) { m_no_estimate = flag; }
	void set_no_dca( bool flag ) { m_no_dca = flag; }

private:
	using problem_t = plmDCA_cpu_objective_for_CppNumericalSolvers<plmDCA_optimizer_parameters_t>;

	plmDCA_optimizer_parameters_t m_optimizer_parameters;

	CouplingStorage<real_t,apegrunt::number_of_states<state_t>::N>& m_Jij_storage;

	OptimizerHistory<real_t>& m_optimizer_log;

	problem_t m_optimizer_objective;

	stopwatch::stopwatch m_cputimer; // for timing statistics

	using allocator_t = apegrunt::memory::AlignedAllocator<real_t>;
	std::vector<real_t,allocator_t> m_solution;

	// the optimizer
	//cppoptlib::LbfgsSolver<real_t> m_optimizer;
	cppoptlib::LbfgsSolver<problem_t> m_optimizer;

	const std::size_t m_loci_slice;

	bool m_no_estimate;
	bool m_no_dca;
};

template< typename RealT, typename StateT > //, typename OptimizerT >
plmDCA_solver<RealT,StateT> get_plmDCA_solver(
	std::vector< apegrunt::Alignment_ptr<StateT> > alignments,
	std::shared_ptr< std::vector<RealT> > weights,
	CouplingStorage<RealT,apegrunt::number_of_states<StateT>::N>& storage,
	OptimizerHistory<RealT>& log,
	std::size_t loci_slice
) { return plmDCA_solver<RealT,StateT>( alignments, weights, storage, log, loci_slice ); }

template< typename RealT, typename StateT >
bool run_plmDCA( std::vector< apegrunt::Alignment_ptr<StateT> >& alignments, apegrunt::Loci_ptr loci_list )
{
	using real_t = RealT;
	using state_t = StateT;

	if( alignments.size() == 0 )
	{
		*plmDCA_options::err_stream() << "plmDCA error: no input alignment(s)\n";
		return false;
	}

	stopwatch::stopwatch cputimer( plmDCA_options::verbose() ? plmDCA_options::out_stream() : nullptr ); // for timing statistics
	stopwatch::stopwatch plmdca_timer( plmDCA_options::verbose() ? plmDCA_options::out_stream() : nullptr ); // for timing statistics
	plmdca_timer.start();

	if( plmDCA_options::verbose() )
	{
		for( auto alignment: alignments )
		{
			*plmDCA_options::out_stream() << "plmDCA: input alignment " << alignment->id_string() << " has " << alignment->size() << " sequences and " << alignment->n_loci() << " loci\n";
		}
	}

	const auto n_loci = alignments.front()->n_loci();

	if( 0 == n_loci )
	{
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "plmDCA: there are no loci to analyze (need at least 2)\n";
		}
		return false;
	}

	if(	plmDCA_options::keep_n_best_couples() == 0 )
	{
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "plmDCA: will not calculate coupling parameters, since user has requested that no solutions should be stored.\n";
		}
		return true;
	}

    auto loci_range = boost::make_iterator_range( cbegin(loci_list), cend(loci_list) );

	apegrunt::Loci_ptr loci_list2;

	if( alignments.size() > 1 )
	{
		const std::size_t n_loci2 = alignments.back()->n_loci();
		std::vector<std::size_t> loci; loci.reserve( n_loci2 );
		for( std::size_t i = 0; i < n_loci2; ++i ) { loci.push_back(i); }
		loci_list2 = apegrunt::make_Loci_list( loci );
	}

	if( (!loci_list2 && loci_list->size() < 2) || (loci_list2 && loci_list2->size() == 0) )
    {
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "plmDCA: there are not enough loci for plmDCA analysis (found " << loci_list->size() << " loci that qualify, but need at least 2)\n";
		}
		return false;
    }

	// reserve space for optimizer statistics log
	OptimizerHistory<real_t> optimizer_log(n_loci);

	// initialize parameter storage
	cputimer.start();
    CouplingStorage<real_t,apegrunt::number_of_states<state_t>::N> Jij_storage( loci_list, ( alignments.size() > 1 ? loci_list2 : loci_list ) );
    //CouplingStorage<real_t,number_of_states<plmDCA_runtime_state_t>::N> Jij_storage( alignments.front()->n_loci(), loci_list->size() );

    if( plmDCA_options::verbose() )
    {
		if( plmDCA_options::norm_of_mean_scoring() )
		{
			// norm-of-mean scoring: the traditional plmDCA way of scoring couplings.
			*plmDCA_options::out_stream() << "plmDCA: storage pool capacity is " << Jij_storage.get_matrix_storage_size() << " units\n";
		}
		else
		{
			// mean-of-norms scoring: the SuperDCA way of scoring couplings. The results (coupling strengths and relative order of couplings)
			// are insignificant compared to norm-of-mean scoring in the statistically significant coupling range, but without the need
			// to store an L-by-L J-matrix consisting of full q-by-q Jij submatrices. Instead, J-matrix elements are scalar, resulting in huge
			// (factor of q^2) memory savings.
			*plmDCA_options::out_stream() << "plmDCA: storage pool capacity is " << Jij_storage.get_coupling_storage_size() << " units\n";
		}
    }
	cputimer.stop(); cputimer.print_timing_stats();

    // calculate sample weight factors
    if( (!loci_list2 && loci_list->size() > 1) || (loci_list2 && loci_list2->size() != 0) )
    {
    	std::shared_ptr< std::vector<real_t> > weights;
    	if( plmDCA_options::reweight() )
    	{
			if( plmDCA_options::verbose() )
			{
				*plmDCA_options::out_stream() << "\nplmDCA: calculate sequence weights\n";
			}
			cputimer.start();
			weights = std::make_shared< std::vector<real_t> >( calculate_weights( alignments.back() ) );
			cputimer.stop(); cputimer.print_timing_stats(); *plmDCA_options::out_stream() << "\n";
    	}
    	else
    	{
    		weights = std::make_shared< std::vector<real_t> >( alignments.back()->size(), 1.0 );
    	}

    	// output weights
		if( plmDCA_options::output_weights() )
		{
			// output weights
			auto weights_file = get_unique_ofstream( alignments.front()->id_string()+".weights" );
			auto& weights_stream = *weights_file.stream();
			weights_stream << std::scientific;
			weights_stream.precision(8);
			for( auto w: *weights ) { weights_stream << w << "\n"; }
		}

		if( plmDCA_options::no_dca() )
		{
			if( plmDCA_options::verbose() )
			{
				*plmDCA_options::out_stream() << "plmDCA: exit on users request without performing DCA\n\n";
			}
			return true;
		}

		// Perform the optimization
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "\nplmDCA: let's learn!\n";
		}

		cputimer.start();

		// refresh block accounting
		for( auto& alignment: alignments )
		{
			alignment->get_block_accounting();
		}

		// The parameter learning stage -- this is where the magic happens
		auto plmDCA_ftor = get_plmDCA_solver( alignments, weights, Jij_storage, optimizer_log, loci_list->size() );
	#ifndef SUPERDCA_NO_TBB
		tbb::parallel_reduce( tbb::blocked_range<decltype(loci_range.begin())>( loci_range.begin(), loci_range.end(), 1 ), plmDCA_ftor );
	#else
		plmDCA_ftor( loci_range );
	#endif // #ifndef SUPERDCA_NO_TBB
		cputimer.stop(); cputimer.print_timing_stats();

	// /*
		// output final coupling scores
		std::ostringstream extension;
		extension << apegrunt::Apegrunt_options::get_output_indexing_base() << "-based"; // indicate base index

		if( !plmDCA_options::no_coupling_output() )
		{

			// Ensure that we always get a unique output filename
			auto couplings_file = get_unique_ofstream( alignments.front()->id_string()+(alignments.size() > 1 ? "_scan" : "")+".SuperDCA_couplings."+extension.str()+".all" );
			//auto matrix_file = get_unique_ofstream( alignments.front()->id_string()+(alignments.size() > 1 ? "_scan" : "")+".SuperDCA_coupling_matrices."+extension.str()+".all" );

			if( couplings_file.stream()->is_open() && couplings_file.stream()->good() )
			{
				if( plmDCA_options::verbose() )
				{
					*plmDCA_options::out_stream() << "\nplmDCA: writing coupling values to file \"" << couplings_file.name() << "\"\n";
				}
				cputimer.start();

				auto index_translation_dim1 = alignments.front()->get_loci_translation();
				auto index_translation_dim2 = alignments.back()->get_loci_translation();

				const std::size_t base_index = apegrunt::Apegrunt_options::get_output_indexing_base();

				auto& couplings_out = *couplings_file.stream();
				for( auto r_itr = cbegin(loci_list); r_itr != cend(loci_list); ++r_itr )
				{
					if( plmDCA_options::norm_of_mean_scoring() )
					{
						//matrixfile.precision(6); matrixfile << std::scientific;

						couplings_out.precision(8); couplings_out << std::fixed;
						const auto r = *r_itr;
						const auto r_index = (*index_translation_dim1)[r]+base_index;

						if( alignments.size() > 1 )
						{
							for( auto n_itr = cbegin(loci_list2); n_itr != cend(loci_list2); ++n_itr )
							{
								const auto n = *n_itr;
								//{
								//	const auto&& Jij = Jij_storage.get_Jij_matrix(r,n);
								//	matrixfile << r_index << " " << (*index_translation_dim2)[n]+base_index << " " << gauge_shift(Jij) << "\n";
								//}

								const auto Jij_norm = frobenius_norm( gauge_shift( Jij_storage.get_Jij_matrix(r,n) ), std::size_t(state_t::GAP) );
								couplings_out << Jij_norm << " " << r_index << " " << (*index_translation_dim2)[n]+base_index << "\n";
							}
						}
						else
						{
							for( auto n_itr = cbegin(loci_list); n_itr != r_itr; ++n_itr )
							{
								const auto n = *n_itr;

								//{
								//	const auto&& Jij = Jij_storage.get_Jij_matrix(r,n);
								//	const auto&& Jji = Jij_storage.get_Jij_matrix(n,r);
								//	matrixfile << (gauge_shift(Jij) + gauge_shift(Jji))*.5 << "\n";
								//}

								const auto Jij_norm = frobenius_norm( gauge_shift( Jij_storage.get_Jij_matrix(r,n) ), std::size_t(state_t::GAP) );
								const auto Jji_norm = frobenius_norm( gauge_shift( Jij_storage.get_Jij_matrix(n,r) ), std::size_t(state_t::GAP) );
								const auto J_norm = frobenius_norm( (gauge_shift(Jij_storage.get_Jij_matrix(r,n)) + gauge_shift(Jij_storage.get_Jij_matrix(n,r)))*0.5, std::size_t(state_t::GAP) );

								couplings_out
									<< J_norm << " " << r_index << " " << (*index_translation_dim2)[n]+base_index
									<< " " << Jij_norm
									<< " " << Jji_norm
									<< " " << (Jij_norm+Jji_norm)*0.5
									//<< " " << optimizer_log.fval_history[n]
									//<< " " << optimizer_log.fval_history[r]
									//<< " " << optimizer_log.nfeval_history[n] + optimizer_log.nfeval_history[r]
									<< "\n";
							}
						}
					}
					else
					{
						couplings_out.precision(8); couplings_out << std::fixed;
						const auto r = *r_itr;
						const auto r_index = (*index_translation_dim1)[r]+base_index;
						if( alignments.size() > 1 )
						{
							for( auto n_itr = cbegin(loci_list2); n_itr != cend(loci_list2); ++n_itr )
							{
								const auto n = *n_itr;
								const auto Jij_norm = Jij_storage.get_Jij_score(r,n);

								couplings_out << Jij_norm << " " << r_index << " " << (*index_translation_dim2)[n]+base_index << "\n";
							}
						}
						else
						{
							for( auto n_itr = cbegin(loci_list); n_itr != r_itr; ++n_itr )
							{
								const auto n = *n_itr;
								const auto Jij_norm = Jij_storage.get_Jij_score(r,n);
								const auto Jji_norm = Jij_storage.get_Jij_score(n,r);

								couplings_out << (Jij_norm+Jji_norm)*0.5 << " " << r_index << " " << (*index_translation_dim2)[n]+base_index << "\n";
							}
						}
					}
				}
				cputimer.stop(); cputimer.print_timing_stats();
			}
		}
    }
    else
    {
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "plmDCA: nothing to do\n";
		}
    }

	if( plmDCA_options::verbose() )
	{
		*plmDCA_options::out_stream() << "\nplmDCA: analysis completed\n";
	}
	plmdca_timer.stop(); plmdca_timer.print_timing_stats();

	return true;
}

} // namespace superdca


#endif // PLMDCA_HPP
