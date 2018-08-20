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

#include "apegrunt/Apegrunt_IO_misc.hpp"
#include "apegrunt/Apegrunt_utility.hpp"
#include "apegrunt/Alignment.h"
#include "apegrunt/StateVector_utility.hpp"
#include "apegrunt/Alignment_utility.hpp"
#include "apegrunt/Alignment_StateVector_weights.hpp"
#include "apegrunt/Loci.h"

#include "misc/Stopwatch.hpp"
#include "misc/Matrix_math.hpp"
#include "misc/CouplingStorage.hpp"
#include "misc/Gauges.hpp"

#include "accumulators/distribution_std.hpp"
#include "accumulators/distribution_bincount.hpp"
#include "accumulators/distribution_ordered.hpp"
#include "accumulators/distribution_cumulative.hpp"
//#include "accumulators/distribution_generator_svg.hpp"
#include "accumulators/distribution_generator_csv.hpp"

#include "plmDCA_options.h"
#include "plmDCA_optimizers.hpp"

namespace superdca {

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

template< typename RealT, typename StateT > //, typename OptimizerT >
class plmDCA_solver
{
public:
	using real_t = RealT;
	using state_t = StateT;
    using plmDCA_optimizer_parameters_t = plmDCA_optimizer_parameters<real_t,state_t>;

	plmDCA_solver( std::vector< apegrunt::Alignment_ptr<state_t> > alignments, std::shared_ptr< std::vector<real_t> > weights, apegrunt::CouplingStorage<real_t,apegrunt::number_of_states<state_t>::N>& storage, OptimizerHistory<real_t>& log, std::size_t loci_slice )
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
	}

	plmDCA_solver( plmDCA_solver<real_t,state_t>&& other )
    : m_optimizer_parameters( other.m_optimizer_parameters ),
	  m_Jij_storage( other.m_Jij_storage ),
	  m_optimizer_log( other.m_optimizer_log ),
	  m_optimizer_objective( m_optimizer_parameters ), // initialize objective
	  m_cputimer( other.m_cputimer ),
	  m_solution( std::move( other.m_solution ) ), // each instance has its own private solution vector
	  m_loci_slice( std::move( other.m_loci_slice ) ),
	  m_no_estimate( other.m_no_estimate ),
	  m_no_dca( other.m_no_dca )
	{
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
// /*
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
// */
/*
			{
				//apegrunt::get_weighted_covariance_matrix<plmDCA_optimizer_parameters_t,real_t>( m_optimizer_parameters );
				apegrunt::get_weighted_covariance_matrix( m_optimizer_parameters, solution.data() );
			}
*/
			m_cputimer.stop();

			if( plmDCA_options::verbose() )
			{
				// buffer output in a ss before committing it to the ostream,
				// in order to keep output clean when run in multi-threaded mode.
				std::ostringstream oss;
				oss << "  " << r+1 << " / " << m_loci_slice << " (out of " << n_loci << ") locus=" << (*(m_optimizer_parameters.get_alignment()->get_loci_translation()))[r]+1
					<< ( m_no_dca ? "" : " " + dca_statistics.str() )
					<< ( m_no_estimate ? "" : " estimate=" + estimate_elapsed_time.str() )
					<< " total=" << m_cputimer << "\n";
				*plmDCA_options::out_stream() << oss.str();
			}

			if( !plmDCA_options::no_coupling_output() )
			{
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
							//copy( apegrunt::ising_gauge( Jr_solution, n ), coupling_ij_matrix );
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
							coupling_ij = apegrunt::frobenius_norm( apegrunt::ising_gauge( Jr_solution, n ), std::size_t(state_t::GAP) );
						}
					}
					else
					{
						// lower-triangular element matrices
						for( std::size_t n=0; n < r; ++n )
						{
							auto& coupling_ij = m_Jij_storage.get_Jij_score(r,n);
							// transpose the lower-triangular element matrices; we don't really need to do this, do we?
							coupling_ij = apegrunt::frobenius_norm( apegrunt::ising_gauge( Jr_solution, n, true ), std::size_t(state_t::GAP) );
						}
						// upper-triangular element matrices
						for( std::size_t n=r+1; n <	Jr_solution.size(); ++n )
						{
							auto& coupling_ij = m_Jij_storage.get_Jij_score(r,n);
							coupling_ij = apegrunt::frobenius_norm( apegrunt::ising_gauge( Jr_solution, n ), std::size_t(state_t::GAP) );
						}
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

	apegrunt::CouplingStorage<real_t,apegrunt::number_of_states<state_t>::N>& m_Jij_storage;

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
	apegrunt::CouplingStorage<RealT,apegrunt::number_of_states<StateT>::N>& storage,
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
			*plmDCA_options::out_stream() << "plmDCA: input alignment \"" << alignment->id_string() << "\" has " << alignment->size() << " sequences and " << alignment->n_loci() << " loci\n";
		}
		*plmDCA_options::out_stream() << std::endl;
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
		loci_list2 = apegrunt::make_Loci_list( std::move(loci) );
	}

	// check that we actually have meaningful work to do
	if( (!loci_list2 && loci_list->size() < 2) || (loci_list2 && loci_list2->size() == 0) )
    {
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "plmDCA: there are not enough loci for plmDCA analysis (found " << loci_list->size() << " loci that qualify, but need at least 2)\n";
		}
		return false;
    }
	else //if( (!loci_list2 && loci_list->size() > 1) || (loci_list2 && loci_list2->size() != 0) )
    {
		// prompt user with an estimate of how much memory we will attempt to allocate
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "plmDCA: "
			<< apegrunt::CouplingStorage<real_t,apegrunt::number_of_states<state_t>::N>::memory_estimate_string( loci_list->size(), ( alignments.size() > 1 ? loci_list2->size() : loci_list->size() ), plmDCA_options::norm_of_mean_scoring() )
			<< std::endl;
		}

		// check if user actually wants to run DCA

		if( plmDCA_options::no_dca() )
		{
			if( plmDCA_options::verbose() )
			{
				*plmDCA_options::out_stream() << "plmDCA: exit without performing DCA\n\n";
			}
			return true;
		}

		cputimer.start();
		// allocate memory for parameter storage
		apegrunt::CouplingStorage<real_t,apegrunt::number_of_states<state_t>::N> Jij_storage( loci_list, ( alignments.size() > 1 ? loci_list2 : loci_list ) );

		if( !plmDCA_options::no_coupling_output() )
		{
			if( plmDCA_options::norm_of_mean_scoring() )
			{
				Jij_storage.allocate_matrix_pool();
			}
			else
			{
				Jij_storage.allocate_scalar_pool();
			}
		}

		if( plmDCA_options::verbose() )
		{
			if( plmDCA_options::norm_of_mean_scoring() )
			{
				// norm-of-mean scoring: the traditional plmDCA way of scoring couplings.
				*plmDCA_options::out_stream() << "plmDCA: storage pool capacity is " << Jij_storage.get_matrix_storage_size() << " units\n";
			}
			else
			{
				// mean-of-norms scoring: the SuperDCA way of scoring couplings. The change in results (coupling strengths and relative order
				// of couplings) are insignificant compared to norm-of-mean scoring in the statistically significant coupling range, but without
				// the need to store an L-by-L J-matrix consisting of full q-by-q Jij submatrices. Instead, J-matrix elements are scalar, resulting
				// in huge (factor of q^2) memory savings.
				*plmDCA_options::out_stream() << "plmDCA: storage pool capacity is " << Jij_storage.get_coupling_storage_size() << " units\n";
			}
		}
		cputimer.stop();
		if( plmDCA_options::verbose() ) { cputimer.print_timing_stats(); *plmDCA_options::out_stream() << "\n"; }

		// reserve space for optimizer statistics log
		OptimizerHistory<real_t> optimizer_log(n_loci);

		// Perform the parameter inference

		cputimer.start();

		// refresh block accounting
		for( auto& alignment: alignments )
		{
			alignment->get_block_accounting();
		}

		auto weights = std::make_shared< decltype(apegrunt::get_weights( alignments.front() )) >( apegrunt::get_weights( alignments.front() ) );

		// The parameter learning stage -- this is where the magic happens
		auto plmDCA_ftor = get_plmDCA_solver( alignments, weights, Jij_storage, optimizer_log, loci_list->size() );
		if( plmDCA_options::verbose() )
		{
			*plmDCA_options::out_stream() << "plmDCA: let's learn!\n"; plmDCA_options::out_stream()->flush();
		}
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

		namespace acc = boost::accumulators;

		acc::accumulator_set<real_t, acc::stats<acc::tag::std(acc::from_distribution),acc::tag::distribution_bincount> > coupling_distribution(acc::tag::distribution::binwidth=0.0001);
		acc::accumulator_set<real_t, acc::stats<acc::tag::std(acc::from_distribution),acc::tag::distribution_bincount> > matrix_element_distribution(acc::tag::distribution::binwidth=0.0001);

		if( !plmDCA_options::no_coupling_output() )
		{

			// Ensure that we always get a unique output filename
			auto couplings_file = apegrunt::get_unique_ofstream( alignments.front()->id_string()+"."+apegrunt::size_string(alignments.front())+(alignments.size() > 1 ? ".scan" : "")+".SuperDCA_couplings."+extension.str()+".all" );
			apegrunt::sink_ptr matrix_file;

			if( plmDCA_options::norm_of_mean_scoring() && plmDCA_options::output_parameter_matrices() )
			{
				matrix_file = apegrunt::get_unique_ofstream( alignments.front()->id_string()+"."+apegrunt::size_string(alignments.front())+(alignments.size() > 1 ? ".scan" : "")+".SuperDCA_coupling_matrices."+extension.str()+".all" );
			}

			if( couplings_file->stream()->is_open() && couplings_file->stream()->good() )
			{
				if( plmDCA_options::verbose() )
				{
					*plmDCA_options::out_stream() << "\nplmDCA: writing coupling values to file \"" << couplings_file->name() << "\"\n";
				}
				cputimer.start();

				auto index_translation_dim1 = alignments.front()->get_loci_translation();
				auto index_translation_dim2 = alignments.back()->get_loci_translation();

				const std::size_t base_index = apegrunt::Apegrunt_options::get_output_indexing_base();

				auto& couplings_out = *couplings_file->stream();

				if( plmDCA_options::norm_of_mean_scoring() )
				{
					for( auto r_itr = cbegin(loci_list); r_itr != cend(loci_list); ++r_itr )
					{
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
								//	matrix_out << r_index << " " << (*index_translation_dim2)[n]+base_index << " " << apegrunt::ising_gauge(Jij) << "\n";
								//}

								const auto Jij_norm = apegrunt::frobenius_norm( apegrunt::ising_gauge( Jij_storage.get_Jij_matrix(r,n) ), std::size_t(state_t::GAP) );
								couplings_out << Jij_norm << " " << r_index << " " << (*index_translation_dim2)[n]+base_index << "\n";
							}
						}
						else
						{
							for( auto n_itr = cbegin(loci_list); n_itr != r_itr; ++n_itr )
							{
								using apegrunt::operator+;
								using apegrunt::operator*;
								using apegrunt::operator<<;

								const auto n = *n_itr;

								//const auto&& Jij = Jij_storage.get_Jij_matrix(r,n);
								//const auto&& Jji = Jij_storage.get_Jij_matrix(n,r);

								const auto&& Jij = apegrunt::ising_gauge( apegrunt::extract<2,2>( Jij_storage.get_Jij_matrix(r,n) )() );
								const auto&& Jji = apegrunt::ising_gauge( apegrunt::extract<2,2>( Jij_storage.get_Jij_matrix(n,r) )() );

								const auto&& Jmatrix = ( Jij + Jji )*.5;

								matrix_element_distribution << Jmatrix;

								// matrix output
								if( plmDCA_options::output_parameter_matrices() )
								{
									auto& matrix_out = *matrix_file->stream();
									matrix_out.precision(6); matrix_out << std::scientific;
									//using apegrunt::operator<<;
									matrix_out << Jmatrix << "\n";
								}

								const auto Jij_norm = apegrunt::frobenius_norm( Jij, std::size_t(state_t::GAP) );
								const auto Jji_norm = apegrunt::frobenius_norm( Jji, std::size_t(state_t::GAP) );
								const auto J_norm = apegrunt::frobenius_norm( Jmatrix );
								/*
								const auto Jij_norm = apegrunt::frobenius_norm( Jij_storage.get_Jij_matrix(r,n), std::size_t(state_t::GAP) );
								const auto Jji_norm = apegrunt::frobenius_norm( Jij_storage.get_Jij_matrix(n,r), std::size_t(state_t::GAP) );
								const auto J_norm = apegrunt::frobenius_norm( (Jij_storage.get_Jij_matrix(r,n) + Jij_storage.get_Jij_matrix(n,r))*0.5, std::size_t(state_t::GAP) );
								*/
								const auto J = (Jij_norm+Jji_norm)*0.5;

								coupling_distribution(J_norm);
								//coupling_accumulator(J);

								couplings_out
									<< J_norm << " " << r_index << " " << (*index_translation_dim2)[n]+base_index
									<< " " << Jij_norm
									<< " " << Jji_norm
									<< " " << J
									//<< " " << optimizer_log.fval_history[n]
									//<< " " << optimizer_log.fval_history[r]
									//<< " " << optimizer_log.nfeval_history[n] + optimizer_log.nfeval_history[r]
									<< "\n";
							}
						}
					}
				}
				else
				{
					for( auto r_itr = cbegin(loci_list); r_itr != cend(loci_list); ++r_itr )
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
								using apegrunt::operator*;

								const auto n = *n_itr;
								const auto Jij_norm = Jij_storage.get_Jij_score(r,n);
								const auto Jji_norm = Jij_storage.get_Jij_score(n,r);

								const auto J = (Jij_norm+Jji_norm)*0.5;

								coupling_distribution(J);

								couplings_out << J << " " << r_index << " " << (*index_translation_dim2)[n]+base_index << "\n";
							}
						}
					}
				}
				cputimer.stop(); cputimer.print_timing_stats();

				cputimer.start();

				// create an svg of the coupling value distribution
				//auto svg_file = apegrunt::get_unique_ofstream( alignments.front()->id_string()+"."+apegrunt::size_string(alignments.front())+".SuperDCA_coupling_distribution.svg" );
				//*svg_file.stream() << apegrunt::accumulators::svg(apegrunt::accumulators::distribution_ordered(coupling_distribution)) << "\n";
				//*svg_file->stream() << apegrunt::accumulators::svg(acc::distribution(coupling_distribution)) << "\n";

				// create an csv file of the coupling value distribution
				auto csv_file = apegrunt::get_unique_ofstream( alignments.front()->id_string()+"."+apegrunt::size_string(alignments.front())+".SuperDCA_coupling_distribution.csv" );
				//*csv_file.stream() << apegrunt::accumulators::csv(apegrunt::accumulators::distribution_ordered(coupling_distribution));
				*csv_file->stream() << apegrunt::accumulators::csv(acc::distribution(coupling_distribution));

				// create an csv file of the cumulative coupling value distribution
				//auto ccsv_file = apegrunt::get_unique_ofstream( alignments.front()->id_string()+"."+apegrunt::size_string(alignments.front())+".SuperDCA_cumulative_coupling_distribution.csv" );
				//*ccsv_file->stream() << apegrunt::accumulators::csv(apegrunt::accumulators::distribution_cumulative(coupling_distribution));

				// output statistics of the coupling value distribution
				plmDCA_options::out_stream()->precision(6); *plmDCA_options::out_stream() << std::scientific;
				*plmDCA_options::out_stream() << "plmDCA: coupling value distribution mean=" << boost::accumulators::distribution_mean(coupling_distribution)
							<< " std=" << boost::accumulators::distribution_std(coupling_distribution) << "\n";
				if( plmDCA_options::norm_of_mean_scoring() )
				{
					// create an csv file of the coupling matrix element value distribution
					auto mat_csv_file = apegrunt::get_unique_ofstream( alignments.front()->id_string()+"."+apegrunt::size_string(alignments.front())+".SuperDCA_coupling_matrix_element_distribution.csv" );
					*mat_csv_file->stream() << apegrunt::accumulators::csv(acc::distribution(matrix_element_distribution));

					*plmDCA_options::out_stream() << "plmDCA: matrix element mean=" << boost::accumulators::distribution_mean(matrix_element_distribution)
							<< " std=" << boost::accumulators::distribution_std(matrix_element_distribution) << "\n";
				}
				cputimer.stop(); cputimer.print_timing_stats();
			}
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
