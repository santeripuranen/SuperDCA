/** @file SuperDCA.cpp
	Utility program for pair-correlation analysis using Pseudo-likelihood Maximization Direct Coupling Analysis (plmDCA).

	Copyright (c) 2016-2018 Santeri Puranen.

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

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "boost/program_options.hpp"
#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp

#include "apegrunt/Apegrunt.h"
#include "apegrunt/Apegrunt_IO_misc.hpp"
#include "apegrunt/Loci_generators.hpp"
#include "apegrunt/ValueVector_parser.hpp"
#include "misc/Stopwatch.hpp"

#include "SuperDCA.h"
#include "plmDCA.hpp"

/*
 * The main program
 */
int main(int argc, char **argv)
{
	namespace po = boost::program_options;
	namespace fs = boost::filesystem;

	#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
	tbb::task_scheduler_init tbb_task_scheduler(tbb::task_scheduler_init::deferred); // Threading task scheduler
	#endif // #ifndef SUPERDCA_NO_TBB

	using namespace superdca;
	using std::exit;

	std::cout << SuperDCA_options::s_get_version_string() << "\n"
			  << apegrunt::Apegrunt_options::s_get_version_string() << "\n\n"
			  << SuperDCA_options::s_get_copyright_notice_string() << "\n"
			  << std::endl;

	using real_t = double;
	using default_state_t = apegrunt::nucleic_acid_state_t;
	//using default_state_t = char;
	using alignment_default_storage_t = apegrunt::Alignment_impl_block_compressed_storage< apegrunt::StateVector_impl_block_compressed_alignment_storage<default_state_t> >;

	// Parse command line options

	// Check the command line options
	po::options_description	all_options;
	SuperDCA_options superdca_options( &std::cout, &std::cerr );
	superdca_options.AddOptions( &all_options ); // add options of the SuperDCA routine

	apegrunt::Apegrunt_options apegrunt_options( &std::cout, &std::cerr );
	apegrunt_options.AddOptions( &all_options ); // add options of apegrunt

	plmDCA_options plmdca_options( &std::cout, &std::cerr );
	plmdca_options.AddOptions( &all_options ); // add options that are specific to the plmDCA algorithm

	po::variables_map options_map;

	// Catch the alignment file, even if not specified by a flag in the input
	po::positional_options_description popt;
	popt.add("alignmentfile", -1);

	std::ostringstream options_string;
	options_string << all_options;
	superdca_options.m_set_options_string( options_string.str() );

	try
	{
		po::store( po::command_line_parser(argc, argv).options(all_options).positional(popt).run(), options_map );
		po::notify(options_map);

		// SuperDCA options
		if( !superdca_options.CheckOptions(&options_map) )
		{
			exit(EXIT_FAILURE);
		}

		// Apegrunt options
		apegrunt_options.set_verbose( SuperDCA_options::verbose() );
		if( !apegrunt_options.CheckOptions(&options_map) )
		{
			exit(EXIT_FAILURE);
		}
		apegrunt_options.set_cuda( SuperDCA_options::cuda() );
		apegrunt_options.set_threads( SuperDCA_options::threads() );

		// plmDCA options
		plmdca_options.set_verbose( SuperDCA_options::verbose() );
		if( !plmdca_options.CheckOptions(&options_map) )
		{
			exit(EXIT_FAILURE);
		}
		plmdca_options.set_cuda( SuperDCA_options::cuda() );
		plmdca_options.set_threads( SuperDCA_options::threads() );

		#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
		SuperDCA_options::threads() > 0 ? tbb_task_scheduler.initialize( SuperDCA_options::threads() ) : tbb_task_scheduler.initialize(); // Threading task scheduler
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream()
				<< "SuperDCA: TBB interface version " << TBB_INTERFACE_VERSION << "\n"
				<< "SuperDCA: TBB runtime interface version " << tbb::TBB_runtime_interface_version() << "\n"
				<< "SuperDCA: TBB task scheduler is " << ( tbb_task_scheduler.is_active() ? "ACTIVE" : "INACTIVE" );
			if( tbb_task_scheduler.is_active() )
			{
				*SuperDCA_options::get_out_stream()
					<< ": using "
					<< ( SuperDCA_options::threads() > 0 ? SuperDCA_options::threads() : tbb_task_scheduler.default_num_threads() )
					<< " threads"
				;
			}
			*SuperDCA_options::get_out_stream() << "\n" << std::endl;
		}
		#endif // #ifndef SUPERDCA_NO_TBB
	}

	catch( std::exception& e )
	{
		// probably an unknown option
		*SuperDCA_options::get_err_stream() << "SuperDCA error: " << e.what() << "\n\n";
		*SuperDCA_options::get_out_stream() << SuperDCA_options::s_get_usage_string() << std::endl;
		exit(EXIT_FAILURE);
	}
	catch(...)
	{
		*SuperDCA_options::get_err_stream() << "SuperDCA error: Exception of unknown type!\n\n";
		exit(EXIT_FAILURE);
	}

	// setup global timer
	stopwatch::stopwatch globaltimer( SuperDCA_options::verbose() ? SuperDCA_options::get_out_stream() : nullptr ); // for timing statistics
	globaltimer.start();

	std::vector< apegrunt::Alignment_ptr<default_state_t> > alignments;
	std::vector< apegrunt::Alignment_ptr<apegrunt::triallelic_state_t> > fourstate_alignments;
	apegrunt::Loci_ptr sample_list;

	stopwatch::stopwatch cputimer( SuperDCA_options::verbose() ? SuperDCA_options::get_out_stream() : nullptr ); // for timing statistics

	// Check if we have the compulsory input alignment
	if( apegrunt::Apegrunt_options::has_alignment_filenames() )
	{
		if( apegrunt::Apegrunt_options::get_alignment_filenames().size() > 2 )
		{
			*SuperDCA_options::get_err_stream() << "SuperDCA error: cannot handle more than 2 alignments (got " << apegrunt::Apegrunt_options::get_alignment_filenames().size() << " filenames).\n\n";
			exit(EXIT_FAILURE);
		}

		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: get " << apegrunt::Apegrunt_options::get_alignment_filenames().size() << " alignment" << (apegrunt::Apegrunt_options::get_alignment_filenames().size() > 1 ? "s" : "") << ":\n\n";
		}

		for( auto& filename: apegrunt::Apegrunt_options::get_alignment_filenames() )
		{
			if( SuperDCA_options::verbose() )
			{
				*SuperDCA_options::get_out_stream() << "SuperDCA: get alignment from file \"" << filename << "\"\n";
			}
			cputimer.start();
			auto alignment = apegrunt::parse_Alignment< alignment_default_storage_t >( filename );
			cputimer.stop();
			if( !alignment )
			{
				*SuperDCA_options::get_err_stream() << "SuperDCA error: Could not get alignment from input file \"" << filename << "\"\n\n";
				exit(EXIT_FAILURE);
			}
			else
			{
				alignments.push_back( alignment ); // store the new alignment
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: got alignment \"" << alignment->id_string() << "\":\n";
					alignment->statistics( SuperDCA_options::get_out_stream() );
				}
			}
			cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n";
		}
	}
	else
	{
		*SuperDCA_options::get_err_stream() << "SuperDCA: no input files specified!\n\n";
		exit(EXIT_FAILURE);
	}

	if( apegrunt::Apegrunt_options::has_samplelist_filename() )
	{
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: extract sample list from file \"" << apegrunt::Apegrunt_options::get_samplelist_filename() << "\"\n";
		}
		cputimer.start();
		sample_list = apegrunt::parse_Loci_list( apegrunt::Apegrunt_options::get_samplelist_filename(), apegrunt::Apegrunt_options::get_input_indexing_base() );
		cputimer.stop();
		if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); }

		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: sample list contains " << sample_list->size() << " samples\n";
		}
	}

	if( apegrunt::Apegrunt_options::sample_alignment() > 0 )
	{
		sample_list = apegrunt::create_random_Loci_list( alignments.front()->size(), apegrunt::Apegrunt_options::sample_alignment() );
		sample_list->set_id_string("random_sample");
	}
// /*
	stopwatch::stopwatch steptimer( SuperDCA_options::verbose() ? SuperDCA_options::get_out_stream() : nullptr ); // for timing statistics
	if( SuperDCA_options::verbose() )
	{
		*SuperDCA_options::get_out_stream() << "SuperDCA: pre-process " << alignments.size() << " alignment" << (alignments.size() > 1 ? "s" : "") << ":\n" << std::endl;
	}
	steptimer.start();
	for( auto alignment: alignments )
	{
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: alignment \"" << alignment->id_string() << "\":\n";
		}
		if( alignments.size() < 2 )
		{
			if( apegrunt::Apegrunt_options::has_includelist_filename() )
			{
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: get include list from file \"" << apegrunt::Apegrunt_options::get_includelist_filename() << "\"\n";
				}
				cputimer.start();
				auto include_list = apegrunt::parse_Loci_list( apegrunt::Apegrunt_options::get_includelist_filename(), apegrunt::Apegrunt_options::get_input_indexing_base() );
				cputimer.print_timing_stats();

				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: include list has " << include_list->size() << " loci.\n";
					*SuperDCA_options::get_out_stream() << "SuperDCA: trim alignment based on include list \"" << include_list->id_string() << "\"\n";
				}
				cputimer.start();
				alignment = apegrunt::Alignment_factory< alignment_default_storage_t >().include( alignments.front(), include_list );
				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n"; }
			}

			if( apegrunt::Apegrunt_options::has_excludelist_filename() )
			{
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: get exclude list from file \"" << apegrunt::Apegrunt_options::get_excludelist_filename() << "\"\n";
				}
				cputimer.start();
				auto exclude_list = apegrunt::parse_Loci_list( apegrunt::Apegrunt_options::get_excludelist_filename(), apegrunt::Apegrunt_options::get_input_indexing_base() );
				cputimer.print_timing_stats();

				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: exclude list has " << exclude_list->size() << " loci.\n";
					*SuperDCA_options::get_out_stream() << "SuperDCA: trim alignment based on exclude list \"" << exclude_list->id_string() << "\"\n";
				}
				cputimer.start();
				alignment = apegrunt::Alignment_factory< alignment_default_storage_t >().exclude( alignments.front(), exclude_list );
				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n"; }
			}

			if( superdca_options.output_filterlist_alignment() )
			{
				// output alignment
				cputimer.start();
				{
					auto alignment_file = apegrunt::get_unique_ofstream( alignment->id_string()+"."+apegrunt::size_string(alignment)+".fasta" );
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: write filterlist-selected alignment to file " << alignment_file->name() << "\n";
					}
					apegrunt::generate_Alignment( alignment, alignment_file->stream() );
				}
				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); }
			}
/*
			if( apegrunt::Apegrunt_options::has_mappinglist_filename() )
			{
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: get exclude list from file \"" << apegrunt::Apegrunt_options::get_mappinglist_filename() << "\"\n";
				}
				cputimer.start();
				auto mapping_list = apegrunt::parse_Loci_list( apegrunt::Apegrunt_options::get_mappinglist_filename(), apegrunt::Apegrunt_options::get_input_indexing_base() );
				cputimer.print_timing_stats();

				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: mapping list has " << mapping_list->size() << " loci.\n";
					*SuperDCA_options::get_out_stream() << "SuperDCA: trim alignment based on exclude list \"" << exclude_list->id_string() << "\"\n";
				}
				cputimer.start();
				alignment = apegrunt::Alignment_factory< alignment_default_storage_t >().exclude( alignments.front(), exclude_list );
				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n"; }
			}
*/
		}

		if( apegrunt::Apegrunt_options::filter_alignment() )
		{
			/*
			// if weighted samples are requested, then calculate the weights here
			// so that the filter rules can then operate on weighted column frequencies.
	    	if( apegrunt::Apegrunt_options::reweight_samples() )
	    	{
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: calculate sequence weights" << std::endl;
				}
				cputimer.start();
				auto weights = apegrunt::calculate_weights( alignments.back() );
				for( auto seq_and_weight: apegrunt::zip_range( alignments.back(), weights ) )
				{
				   	using boost::get;
					get<0>(seq_and_weight)->set_weight( get<1>(seq_and_weight) );
				}

				cputimer.stop(); cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n";
	    	}
			*/
	    	// apply filters as defined on the command line
			cputimer.start();
			if( SuperDCA_options::verbose() )
			{
				*SuperDCA_options::get_out_stream() << "SuperDCA: apply filter rules";
				SuperDCA_options::get_out_stream()->flush();
			}
			auto alignment_filter = apegrunt::Alignment_filter( apegrunt::Alignment_filter::ParameterPolicy::AQUIRE_GLOBAL );
			//auto alignment_filter = apegrunt::Alignment_filter( apegrunt::Alignment_filter::ParameterPolicy::FILTER_SNPS );
			alignment = alignment_filter.operator()<alignment_default_storage_t>( alignment );
			cputimer.stop();
			if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n"; }

			if( SuperDCA_options::output_filtered_alignment() )
			{
				cputimer.start();

				// output alignment
				{
					auto alignment_file = apegrunt::get_unique_ofstream( alignment->id_string()+"."+apegrunt::size_string(alignment)+".fasta" );
					auto phandango_file = apegrunt::get_unique_ofstream( alignment->id_string()+"."+apegrunt::size_string(alignment)+".phandango.csv" );
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: write filtered alignment to file " << alignment_file->name() << "\n";
					}
					apegrunt::generate_Alignment( alignment, alignment_file->stream() );
					apegrunt::generate_PhandangoCSV( alignment, phandango_file->stream() );
				}

				// output loci list
				{
					auto locilist_file = apegrunt::get_unique_ofstream( alignment->id_string()+"."+apegrunt::size_string(alignment)+".loci"  );
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: write original loci indices for filtered alignment to file " << locilist_file->name() << "\n";
					}
					apegrunt::generate_Loci_list( alignment->get_loci_translation(), locilist_file->stream() );
				}

				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n"; }
			}
		}

		if( sample_list )
		{
			if( SuperDCA_options::verbose() )
			{
				*SuperDCA_options::get_out_stream() << "SuperDCA: trim alignment samples based on sample list \"" << sample_list->id_string() << "\"\n";
			}
			cputimer.start();
			alignment = apegrunt::Alignment_factory< alignment_default_storage_t >().copy_selected( alignment, sample_list, sample_list->id_string() );
			cputimer.stop();
			if( plmDCA_options::verbose() ) { cputimer.print_timing_stats(); }

			if( SuperDCA_options::output_samplelist_alignment() )
			{
				cputimer.start();
				{
					auto alignment_file = apegrunt::get_unique_ofstream( alignment->id_string()+"."+apegrunt::size_string(alignment)+".fasta" );
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: write samplelist-selected alignment to file " << alignment_file->name() << "\n";
					}
					apegrunt::generate_Alignment( alignment, alignment_file->stream() );
				}
				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); *SuperDCA_options::get_out_stream() << "\n"; }
			}
		}

		// force all alignments into a 4-state envelope; some loci may be dropped in the process
		cputimer.start();
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: transfer to four state container";
			SuperDCA_options::get_out_stream()->flush();
		}
		fourstate_alignments.push_back( apegrunt::transform_alignment<apegrunt::triallelic_state_t>( alignment ) );
		cputimer.stop();
		if( SuperDCA_options::verbose() )
		{
			cputimer.print_timing_stats();
			fourstate_alignments.back()->statistics( SuperDCA_options::get_out_stream() );
			*SuperDCA_options::get_out_stream() << "\n";
		}
/*
		{
			if( SuperDCA_options::verbose() )
			{
				*SuperDCA_options::get_out_stream() << "SuperDCA: transpose alignment";
				SuperDCA_options::get_out_stream()->flush();
			}
			cputimer.start();
			auto transposed_alignment = apegrunt::transpose_alignment( fourstate_alignments.back() );
			cputimer.stop();
			if( SuperDCA_options::verbose() )
			{
				cputimer.print_timing_stats();
				transposed_alignment->statistics( SuperDCA_options::get_out_stream() );
				*SuperDCA_options::get_out_stream() << "\n";
			}
		}
*/
	    // deal with sample weights
		{
			using std::cbegin; using std::cend;
			auto& fsalignment = fourstate_alignments.back();
			if( apegrunt::Apegrunt_options::reweight_samples() )
			{
				cputimer.start();

				if( apegrunt::Apegrunt_options::has_sample_weights_filename() )
				{
					if( plmDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: get sample weights from file \"" << apegrunt::Apegrunt_options::get_sample_weights_filename() << "\"\n";
					}

					auto weights = apegrunt::parse_ValueVector<real_t>( apegrunt::Apegrunt_options::get_sample_weights_filename() );
					const auto n_size = fsalignment->size();
					using apegrunt::cbegin; using apegrunt::cend;
					const auto n_eff = std::accumulate( cbegin(weights), cend(weights), real_t(0) );

					for( auto seq_and_weight: apegrunt::zip_range( fsalignment, weights ) )
					{
						using boost::get;
						if( apegrunt::Apegrunt_options::rescale_sample_weights() )
						{
							// weights scaled to give neff == n_size
							get<0>(seq_and_weight)->set_weight( real_t(get<1>(seq_and_weight)) * real_t(n_size)/real_t(n_eff) );
						}
						else
						{
							// unscaled weights neff != n_size
							get<0>(seq_and_weight)->set_weight( get<1>(seq_and_weight) );
						}
					}
				}
				else
				{
					if( plmDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: calculate sequence weights" << std::endl;
					}

					auto weights = apegrunt::calculate_weights( fsalignment );
					const auto n_size = fsalignment->size();
					const auto n_eff = std::accumulate( cbegin(weights), cend(weights), real_t(0) );

					for( auto seq_and_weight: apegrunt::zip_range( fsalignment, weights ) )
					{
						using boost::get;
						if( apegrunt::Apegrunt_options::rescale_sample_weights() )
						{
							// weights scaled to give neff == n_size
							get<0>(seq_and_weight)->set_weight( real_t(get<1>(seq_and_weight)) * real_t(n_size)/real_t(n_eff) );
						}
						else
						{
							// unscaled weights neff != n_size
							get<0>(seq_and_weight)->set_weight( get<1>(seq_and_weight) );
						}
					}
				}

				cputimer.stop(); cputimer.print_timing_stats(); *plmDCA_options::out_stream() << "\n";
			}
			// output weights
			if( apegrunt::Apegrunt_options::output_sample_weights() )
			{
				// output weights
				auto weights_file = apegrunt::get_unique_ofstream( fsalignment->id_string()+"."+apegrunt::size_string(fsalignment)+".weights" );
				auto& weights_stream = *weights_file->stream();
				weights_stream << std::scientific;
				weights_stream.precision(8);
				auto weights = apegrunt::get_weights( fsalignment );
				for( auto w: weights ) { weights_stream << w << "\n"; }
			}
		}

		if( apegrunt_options.output_state_frequencies() )
		{
			auto& fsalignment = fourstate_alignments.back();

			{ // output un-weighted frequencies
				cputimer.start();
				auto frequencies_file = apegrunt::get_unique_ofstream( fsalignment->id_string()+"."+apegrunt::size_string(fsalignment)+".frequencies.txt" );
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: write columnwise state frequencies to file \"" << frequencies_file->name() << "\"\n";
					SuperDCA_options::get_out_stream()->flush();
				}
				if( apegrunt::output_frequencies( fsalignment, frequencies_file->stream() ) )
				{
					cputimer.stop();
					if( SuperDCA_options::verbose() )
					{
						cputimer.print_timing_stats();
						*SuperDCA_options::get_out_stream() << "\n";
					}
				}
				else
				{
					cputimer.stop();
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_err_stream() << "SuperDCA ERROR: unable to write file \"" << frequencies_file->name() << "\"\n" << std::endl;
					}
				}
			}
			{ // output un-weighted frequency distribution
				cputimer.start();
				auto distribution_file = apegrunt::get_unique_ofstream( fsalignment->id_string()+"."+apegrunt::size_string(fsalignment)+".frequency_distribution.txt" );
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: write columnwise state frequency distribution to file \"" << distribution_file->name() << "\"\n";
					SuperDCA_options::get_out_stream()->flush();
				}
				if( apegrunt::output_frequency_distribution( fsalignment, distribution_file->stream() ) )
				{
					cputimer.stop();
					if( SuperDCA_options::verbose() )
					{
						cputimer.print_timing_stats();
						*SuperDCA_options::get_out_stream() << "\n";
					}
				}
				else
				{
					cputimer.stop();
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_err_stream() << "SuperDCA ERROR: unable to write file \"" << distribution_file->name() << "\"\n" << std::endl;
					}
				}
			}

			if( apegrunt::Apegrunt_options::reweight_samples() )
			{
				{ // output weighted frequencies
					cputimer.start();
					auto frequencies_file = apegrunt::get_unique_ofstream( fsalignment->id_string()+"."+apegrunt::size_string(fsalignment)+".weighted_frequencies.txt" );
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: write weighted columnwise state frequencies to file \"" << frequencies_file->name() << "\"\n";
					}
					if( apegrunt::output_frequencies( fsalignment, frequencies_file->stream(), true ) ) // true == output weighted frequencies
					{
						cputimer.stop();
						if( SuperDCA_options::verbose() )
						{
							cputimer.print_timing_stats();
							*SuperDCA_options::get_out_stream() << "\n";
						}
					}
					else
					{
						cputimer.stop();
						if( plmDCA_options::verbose() )
						{
							*SuperDCA_options::get_err_stream() << "SuperDCA ERROR: unable to write file \"" << frequencies_file->name() << "\"\n" << std::endl;
						}
					}
				}
				{ // output un-weighted frequency distribution
					cputimer.start();
					auto distribution_file = apegrunt::get_unique_ofstream( fsalignment->id_string()+"."+apegrunt::size_string(fsalignment)+".weighted_frequency_distribution.txt" );
					if( SuperDCA_options::verbose() )
					{
						*SuperDCA_options::get_out_stream() << "SuperDCA: write columnwise state frequency distribution to file \"" << distribution_file->name() << "\"\n";
						SuperDCA_options::get_out_stream()->flush();
					}
					if( apegrunt::output_frequency_distribution( fsalignment, distribution_file->stream(), true ) ) // true == output weighted frequencies
					{
						cputimer.stop();
						if( SuperDCA_options::verbose() )
						{
							cputimer.print_timing_stats();
							*SuperDCA_options::get_out_stream() << "\n";
						}
					}
					else
					{
						cputimer.stop();
						if( SuperDCA_options::verbose() )
						{
							*SuperDCA_options::get_err_stream() << "SuperDCA ERROR: unable to write file \"" << distribution_file->name() << "\"\n" << std::endl;
						}
					}
				}
			}
		}

		// output the sample-sample Hamming distance matrix
		if( apegrunt::Apegrunt_options::output_sample_distance_matrix() )
		{
			cputimer.start();
			auto& fsalignment = fourstate_alignments.back();
			auto matrix_file = apegrunt::get_unique_ofstream( fsalignment->id_string()+"."+apegrunt::size_string(fsalignment)+".triangular_sample_distance_matrix" );

			if( SuperDCA_options::verbose() )
			{
				*SuperDCA_options::get_out_stream() << "SuperDCA: write sample-sample Hamming distance matrix to file \"" << matrix_file->name() << "\"\n";
			}
			if( apegrunt::output_sample_distance_matrix( fsalignment, matrix_file->stream() ) )
			{
				cputimer.stop();
				if( SuperDCA_options::verbose() )
				{
					cputimer.print_timing_stats();
					*SuperDCA_options::get_out_stream() << "\n";
				}
			}
			else
			{
				cputimer.stop();
				if( plmDCA_options::verbose() )
				{
					*SuperDCA_options::get_err_stream() << "SuperDCA ERROR: unable to write file \"" << matrix_file->name() << "\"\n" << std::endl;
				}
			}
		}
	}

	apegrunt::Loci_ptr loci_list;

	if( superdca_options.has_locilist_filename() )
	{
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: extract list of loci to be analysed from file \"" << superdca_options.get_locilist_filename() << "\".\n";
		}
		cputimer.start();
		loci_list = apegrunt::parse_Loci_list( superdca_options.get_locilist_filename(), apegrunt_options.get_input_indexing_base() );
		cputimer.print_timing_stats();
		if( loci_list )
		{
			if( SuperDCA_options::verbose() )
			{
				*SuperDCA_options::get_out_stream() << "SuperDCA: loci list contains " << loci_list->size() << " loci that will be analysed.\n";
			}
		}
		else
		{
			*SuperDCA_options::get_err_stream() << "SuperDCA ERROR: failed to parse loci list from file \"" << superdca_options.get_locilist_filename() << "\"!\n";
			exit(EXIT_FAILURE);
		}
	}

	// construct a list of position indices to include in analysis, if we didn't get one from the user
    if( !loci_list )
    {
		const std::size_t end_locus = apegrunt_options.get_end_locus() < 0 ? fourstate_alignments.front()->n_loci() : apegrunt_options.get_end_locus()+1;
		const std::size_t begin_locus = apegrunt_options.get_begin_locus();
		std::vector<std::size_t> loci; loci.reserve(end_locus-begin_locus);
		for( std::size_t i = begin_locus; i < end_locus; ++i ) { loci.push_back(i); }
		loci_list = apegrunt::make_Loci_list( std::move(loci) );
    }
	steptimer.stop();
	if( SuperDCA_options::verbose() )
	{
		*SuperDCA_options::get_out_stream() << "SuperDCA: pre-processing completed\n";
		steptimer.print_timing_stats();
		*SuperDCA_options::get_out_stream() << "\n";
	}

	if( SuperDCA_options::verbose() )
	{
		*SuperDCA_options::get_out_stream() << "SuperDCA: run plmDCA\n" << std::endl;
	}

	// run the inference
	bool plmDCA_success = run_plmDCA<double>( fourstate_alignments, loci_list );

	if( SuperDCA_options::verbose() )
	{
		*SuperDCA_options::get_out_stream() << "SuperDCA: analysis completed\n";
	}
	globaltimer.stop(); globaltimer.print_timing_stats();
	if(	!plmDCA_success )
	{
		*SuperDCA_options::get_err_stream() << "SuperDCA error: plmDCA failed\n\n";
		exit(EXIT_FAILURE);
	}
// */
    exit(EXIT_SUCCESS);
}
