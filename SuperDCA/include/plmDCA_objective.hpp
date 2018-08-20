/** @file plmDCA_objective.hpp

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

#ifndef SUPERDCA_PLMDCA_OBJECTIVE_HPP
#define SUPERDCA_PLMDCA_OBJECTIVE_HPP

#include <cmath>

#include "apegrunt/Alignment.h"
#include "apegrunt/Apegrunt_utility.hpp"
#include "apegrunt/StateVector_interface.hpp"

#include "misc/Vector.h"

#include "plmDCA_optimizer_parameters.hpp"

namespace superdca
{

template< typename ParametersT, typename HPRealT=typename ParametersT::real_t >
void plmDCA_objective_fval_and_gradient( ParametersT& parameters )
{
	enum { N=ParametersT::N };

	using real_t = typename ParametersT::real_t;
	using vector_t = apegrunt::Vector<real_t,N>;
	using vector_view_t = apegrunt::Vector<real_t,N,true>;

	// input parameters
	auto alignment = parameters.get_alignment();
	auto&& h_r = parameters.get_hr_view();
	auto&& J_r = parameters.get_Jr_view();
	auto& weights = *(parameters.get_weights());
	const auto r = parameters.get_target_column(); // index of current/target column
	const auto lambda_h = real_t( parameters.get_lambda_h() );
	const auto lambda_J = real_t( parameters.get_lambda_J() );

	const auto& block_accounting = *(alignment->get_block_accounting());
	const auto& blocks = *(alignment->get_block_storage());

	// output parameters
	auto&& grad_hr = parameters.get_grad_hr_view();
	auto&& grad_Jr = parameters.get_grad_Jr_view();

    const std::size_t n_loci = alignment->n_loci(); // number of columns in the alignment
    const std::size_t n_loci_per_block = apegrunt::StateBlock_size;
    const std::size_t last_block_size = apegrunt::get_last_block_size(n_loci-1);
    const std::size_t last_block = apegrunt::get_last_block_index(n_loci-1);
	const std::size_t r_block = ( parameters.number_of_alignments() > 1 ? last_block+1 : r / n_loci_per_block );

    //std::cout << "last_block_size=" << last_block_size << " last_block=" << last_block << "\n";

    // for internal use
    real_t fval{0.0}; // function value

    auto& logPots = parameters.get_logPots();
    auto& nodeBels = parameters.get_nodeBels();

    //vector_t element_mask; element_mask[0]=1.0; element_mask[1]=1.0;

    //> Function value:

    // The following nested loops will traverse through all alignment
    // elements, except the ones in column 'r'.

    // Loop over all sequences

	// Initialize logPot with the parameter estimates for column 'r'.
	// The values are either initial estimates supplied by the user or
	// estimates produced by the optimizer.

    for( auto& logPot: logPots ) { vector_view_t( logPot.data() ) = vector_view_t( h_r.data() ); } //() * element_mask(); }

    // accumulate logPots from input parameter estimates
    {
		for( std::size_t n_block=0; n_block < last_block+1; ++n_block )
		{
			const auto n_end = ( n_block == last_block ? last_block_size : n_loci_per_block );
			auto&& Jr_block_acc = J_r.get_accumulator_for_block( n_block, n_end );
			const auto& sequence_blocks = blocks[n_block];

			if( r_block != n_block )
			{
				for( std::size_t block_index=0; block_index < block_accounting[n_block].size(); ++block_index )
				{
					const auto logPot = Jr_block_acc.accumulate( sequence_blocks[block_index] );
#ifndef NDEBUG
					std::cout.precision(12);
					std::cout << "logPot=" << logPot << " nblock=" << n_block << " i=" << block_index << std::endl;
#endif // NDEBUG
					for( auto i : block_accounting[n_block][block_index] )
					{
						vector_view_t( logPots[i].data() ) += logPot;
					}
				}
			}
			else
			{
				const auto r_local = r % n_loci_per_block;

				for( std::size_t block_index=0; block_index < block_accounting[n_block].size(); ++block_index )
				{
					const auto logPot = Jr_block_acc.accumulate( sequence_blocks[block_index], r_local );
#ifndef NDEBUG
					std::cout.precision(12);
					std::cout << "logPot=" << logPot << " nblock=" << n_block << " i=" << block_index << std::endl;
#endif // NDEBUG
					for( auto i : block_accounting[n_block][block_index] )
					{
						vector_view_t( logPots[i].data() ) += logPot;
					}
				}
			}
		}
/*
#ifndef NDEBUG
		std::cout.precision(12);
		for( std::size_t i=0; i < logPots.size(); ++i )
		{
			std::cout << std::scientific << "logPot[" << i << "] = " << vector_view_t( logPots[i].data() ) << std::endl;
		}
#endif // NDEBUG
*/
    }

	// update fval and prepare for gradient update

	// Loop over all sequences
    {
        const std::size_t n_seqs = alignment->size(); // number of sequences in the alignment
		const auto& r_states = parameters.get_rstates();

		for( std::size_t i = 0; i < n_seqs; ++i )
		{
			using std::log;
			using apegrunt::sum;
			using apegrunt::exp;

			const vector_view_t logPot( logPots[i].data() );
			const auto state_r = r_states[i];
			const auto weight = real_t( weights[i] );

			// vectorized 64-bit precision
			std::cout.precision(12);
			//std::cout << std::scientific << "weight=" << weight << " logPot=" << logPot << std::endl;
			const auto wlog_z = log( sum( exp( logPot() ) ) );

			// Function value:
			fval += weight * ( wlog_z - logPot[state_r] );

			// The gradient:
			vector_view_t nodeBel( nodeBels[i].data() );
			//std::cout << std::scientific << "weight=" << weight << " nodeBel=" << nodeBel << " wlog_z=" << wlog_z << " exp(" << vector_t( logPot() - vector_t(wlog_z)() ) << ")" << std::endl;
			nodeBel = vector_t(weight)() * exp( logPot() - vector_t(wlog_z)() ); // * element_mask();
			//std::cout << std::scientific << "nodeBel=" << nodeBel << std::endl;
			nodeBel[state_r] -= weight;
			vector_view_t( grad_hr.data() ) += nodeBel;
		}
    }

	// update gradient
	{
		for( std::size_t n_block=0; n_block < last_block+1; ++n_block )
		{
			const auto n_end = ( n_block == last_block ? last_block_size : n_loci_per_block );
			const auto& sequence_blocks = blocks[n_block];
			auto&& grad_Jr_block_acc = grad_Jr.get_accumulator_for_block( n_block, n_end );

			if( r_block != n_block )
			{
				for( std::size_t block_index=0; block_index < block_accounting[n_block].size(); ++block_index )
				{
					const auto seqblock = sequence_blocks[block_index];

					for( auto i : block_accounting[n_block][block_index] )
					{
						vector_view_t nodeBel( nodeBels[i].data() );
						grad_Jr_block_acc.add_to_matrix_rows( seqblock, nodeBel );
					}
				}
			}
			else
			{
				const auto r_local = r % n_loci_per_block;

				for( std::size_t block_index=0; block_index < block_accounting[n_block].size(); ++block_index )
				{
					const auto seqblock = sequence_blocks[block_index];

					for( auto i : block_accounting[n_block][block_index] )
					{
						vector_view_t nodeBel( nodeBels[i].data() );
						grad_Jr_block_acc.add_to_matrix_rows( seqblock, nodeBel, r_local );
					}
				}
			}
		}
	} // gradient

	// Add contributions from R_l2
    {
		using apegrunt::sum;
		using apegrunt::pow;

		// Contribution of h:
		//> Function value:
    	fval += sum( vector_t(lambda_h)() * pow<2>(vector_view_t(h_r.data())()) ); // * element_mask() );

    	//> Gradient:
    	vector_view_t(grad_hr.data()) += vector_t(lambda_h*2.0)() * vector_view_t(h_r.data())(); // * element_mask();

    	// Contribution of the Js
		//const vector_t vec_lambda_J(lambda_J);
		//const vector_t vec_lambda_J2(lambda_J*2.0);

		//const vector_t vec_lambda_J( vector_t(lambda_J)() * element_mask() );
		//const vector_t vec_lambda_J2( vector_t(lambda_J*2.0)() * element_mask() );

		for( std::size_t n_block=0; n_block <= last_block; ++n_block )
		{
			const auto n_end = ( n_block == last_block ? last_block_size : n_loci_per_block );

			auto&& grad_Jr_block = grad_Jr.get_view_for_block( n_block, n_end );
			auto&& Jr_block = J_r.get_view_for_block( n_block, n_end );

			if( r_block != n_block )
			{
				for( std::size_t state=0; state<N; ++state )
				{
					for( std::size_t n=0; n < n_end; ++n )
					{
						const vector_view_t Jr_n( Jr_block(n,state) );
						vector_t vec_lambda_J( lambda_J );
						vector_t vec_lambda_J2( vec_lambda_J() * 2.0 );

						vector_view_t( grad_Jr_block(n,state) ) += vec_lambda_J2() * Jr_n();

						// Function value:
						fval += sum( vec_lambda_J() * pow<2>( Jr_n() ) );
					}
				}
			}
			else
			{
				const auto r_local = r % n_loci_per_block;

				for( std::size_t state=0; state<N; ++state )
				{
					for( std::size_t n=0; n < n_end; ++n )
					{
						if( r_local != n )
						{
							const vector_view_t Jr_n( Jr_block(n,state) );
							vector_t vec_lambda_J( lambda_J );
							vector_t vec_lambda_J2( vec_lambda_J() * 2.0 );

							vector_view_t( grad_Jr_block(n,state) ) += vec_lambda_J2() * Jr_n();

							// Function value:
							fval += sum( vec_lambda_J() * pow<2>( Jr_n() ) );
						}
					}
				}
			}
		}
    }

    parameters.set_fvalue( real_t(fval) );

    return;
}

} // namespace superdca

#endif // SUPERDCA_PLMDCA_OBJECTIVE_HPP
