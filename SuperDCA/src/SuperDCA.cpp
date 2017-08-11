/** @file SuperDCA.cpp
	Utility program for pair-correlation analysis using Pseudo-likelihood Maximization Direct Coupling Analysis (plmDCA).

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

#include <iostream>
#include <fstream>

#include "boost/program_options.hpp"
#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp

#include "apegrunt/Apegrunt.h"

#include "SuperDCA.h"
#include "plmDCA.hpp"
#include "Stopwatch.hpp"
#include "SuperDCA_commons.h"

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

	std::cout << SuperDCA_options::s_get_version_string() << "\n"
			  << apegrunt::Apegrunt_options::s_get_version_string() << "\n\n"
			  << SuperDCA_options::s_get_copyright_notice_string() << "\n"
			  << std::endl;

	using default_state_t = apegrunt::nucleic_acid_state_t;
	using alignment_default_storage_t = apegrunt::Alignment_impl_block_compressed_storage< apegrunt::StateVector_impl_block_compressed_alignment_storage<default_state_t> >;

	// Parse command line options

	// Check the command line options
	po::options_description	all_options;
	SuperDCA_options superdca_options( &std::cout, &std::cerr );
	superdca_options.AddOptions( &all_options ); // add options of the SuperDCA routine

	apegrunt::Apegrunt_options apegrunt_options( &std::cout, &std::cerr );
	apegrunt_options.AddOptions( &all_options ); // add options of libapegrunt

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
			Exit(EXIT_FAILURE);
		}

		// Apegrunt options
		apegrunt_options.set_verbose( SuperDCA_options::verbose() );
		if( !apegrunt_options.CheckOptions(&options_map) )
		{
			Exit(EXIT_FAILURE);
		}
		apegrunt_options.set_cuda( SuperDCA_options::cuda() );
		apegrunt_options.set_threads( SuperDCA_options::threads() );

		// plmDCA options
		plmdca_options.set_verbose( SuperDCA_options::verbose() );
		if( !plmdca_options.CheckOptions(&options_map) )
		{
			Exit(EXIT_FAILURE);
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
			*SuperDCA_options::get_out_stream()<< std::endl;
		}
		#endif // #ifndef SUPERDCA_NO_TBB
	}

	catch( std::exception& e )
	{
		// probably an unknown option
		*SuperDCA_options::get_err_stream() << "SuperDCA error: " << e.what() << "\n\n";
		*SuperDCA_options::get_out_stream() << SuperDCA_options::s_get_usage_string() << std::endl;
		Exit(EXIT_FAILURE);
	}
	catch(...)
	{
		*SuperDCA_options::get_err_stream() << "SuperDCA error: Exception of unknown type!\n\n";
		Exit(EXIT_FAILURE);
	}

	// setup global timer
	stopwatch::stopwatch globaltimer( SuperDCA_options::verbose() ? SuperDCA_options::get_out_stream() : nullptr ); // for timing statistics
	globaltimer.start();

	std::vector< apegrunt::Alignment_ptr<default_state_t> > alignments;
	stopwatch::stopwatch cputimer( SuperDCA_options::verbose() ? SuperDCA_options::get_out_stream() : nullptr ); // for timing statistics

	// Check if we have the compulsory input alignment
	if( superdca_options.has_alignment_filenames() )
	{
		if( superdca_options.get_alignment_filenames().size() > 2 )
		{
			*SuperDCA_options::get_err_stream() << "SuperDCA error: cannot handle more than 2 alignments (got " << superdca_options.get_alignment_filenames().size() << " filenames).\n\n";
			Exit(EXIT_FAILURE);
		}

		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: get " << superdca_options.get_alignment_filenames().size() << " alignment(s).\n";
		}

		for( auto& filename: superdca_options.get_alignment_filenames() )
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
				Exit(EXIT_FAILURE);
			}
			else
			{
				alignments.push_back( alignment ); // store the new alignment
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: got alignment \"" << alignment->id_string() << "\" with " << alignment->size() << " sequences and " << alignment->n_loci() << " loci.\n";
					alignment->statistics( SuperDCA_options::get_out_stream() );
				}
			}
			cputimer.print_timing_stats();
		}
	}
	else
	{
		*SuperDCA_options::get_err_stream() << "SuperDCA: no input files specified!\n\n";
		Exit(EXIT_FAILURE);
	}

	if( SuperDCA_options::verbose() )
	{
		*SuperDCA_options::get_out_stream() << "\nSuperDCA: process alignments\n";
	}

	if( superdca_options.output_SNPs() )
	{
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: extract SNPs\n";
		}
		auto alignment_filter = apegrunt::Alignment_filter( apegrunt::Alignment_filter::ParameterPolicy::FILTER_SNPS );
		for( auto& alignment: alignments )
		{
			cputimer.start();
			alignment = alignment_filter.operator()<alignment_default_storage_t>( alignment );
			if( SuperDCA_options::verbose() )
			{
				*SuperDCA_options::get_out_stream() << "SuperDCA: alignment has " << alignment->size() << " sequences and " << alignment->n_loci() << " SNPs\n";
				alignment->statistics( SuperDCA_options::get_out_stream() );
			}
			fs::path filepath( alignment->id_string()+".fasta" );
			if( !fs::exists( filepath ) )
			{
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: write alignment to file " << filepath << "\n";
				}

				cputimer.start();
				std::ofstream alignment_file( filepath.c_str(), std::ios_base::binary );
				apegrunt::generate_Alignment( alignment, &alignment_file );
				alignment_file.close();
				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); }
			}

			cputimer.stop(); cputimer.print_timing_stats();
		}
	}

	if( superdca_options.has_filterlist_filename() )
	{
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: extract filter list from file \"" << superdca_options.get_filterlist_filename() << "\"\n";
		}
		cputimer.start();
		auto accept_list = apegrunt::parse_Loci_list( superdca_options.get_filterlist_filename(), apegrunt_options.get_input_indexing_base() );
		cputimer.print_timing_stats();

		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: filter list contains " << accept_list->size() << " loci.\n";
		}
		//std::cout << "SuperDCA: filter list = {" << accept_list << " }" << std::endl;

		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: trim alignment based on loci list \"" << accept_list->id_string() << "\"\n";
		}
		cputimer.start();
		alignments.front() = apegrunt::Alignment_factory< alignment_default_storage_t >()( alignments.front(), accept_list );
		cputimer.stop();
		if( plmDCA_options::verbose() ) { cputimer.print_timing_stats(); }

		if( SuperDCA_options::output_filterlist_alignment() )
		{
			fs::path filepath( alignments.front()->id_string()+".fasta" );
			if( !fs::exists( filepath ) )
			{
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: write filter list-selected alignment to file " << filepath << "\n";
				}

				cputimer.start();
				std::ofstream alignment_file( filepath.c_str(), std::ios_base::binary );
				apegrunt::generate_Alignment( alignments.front(), &alignment_file );
				alignment_file.close();
				cputimer.stop();
				if( plmDCA_options::verbose() ) { cputimer.print_timing_stats(); }
			}
		}
	}

	if( superdca_options.has_samplelist_filename() )
	{
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: extract sample list from file \"" << superdca_options.get_samplelist_filename() << "\"\n";
		}
		cputimer.start();
		auto sample_list = apegrunt::parse_Loci_list( superdca_options.get_samplelist_filename(), apegrunt_options.get_input_indexing_base() );
		cputimer.print_timing_stats();

		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: sample list contains " << sample_list->size() << " samples.\n";
		}

		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: trim alignment samples based on sample list \"" << sample_list->id_string() << "\"\n";
		}
		cputimer.start();
		alignments.front() = apegrunt::Alignment_factory< alignment_default_storage_t >().copy_selected( alignments.front(), sample_list, sample_list->id_string() );
		cputimer.stop();
		if( plmDCA_options::verbose() ) { cputimer.print_timing_stats(); }

		if( SuperDCA_options::output_samplelist_alignment() )
		{
			fs::path filepath( alignments.front()->id_string()+".fasta" );
			if( !fs::exists( filepath ) )
			{
				if( SuperDCA_options::verbose() )
				{
					*SuperDCA_options::get_out_stream() << "SuperDCA: write samplelist-selected alignment to file " << filepath << "\n";
				}

				cputimer.start();
				std::ofstream alignment_file( filepath.c_str(), std::ios_base::binary );
				apegrunt::generate_Alignment( alignments.front(), &alignment_file );
				alignment_file.close();
				cputimer.stop();
				if( SuperDCA_options::verbose() ) { cputimer.print_timing_stats(); }
			}
		}
	}
	else // apply filters as defined on the command line
	{
		auto alignment_filter = apegrunt::Alignment_filter( apegrunt::Alignment_filter::ParameterPolicy::AQUIRE_GLOBAL );
		for( auto& alignment: alignments )
		{
			alignment = alignment_filter.operator()<alignment_default_storage_t>( alignment );
		}
	}

	// force all alignments into a 4-state envelope; some loci may be dropped in the process
	std::vector< apegrunt::Alignment_ptr<apegrunt::triallelic_state_t> > fourstate_alignments;

	for( auto& alignment: alignments )
	{
		fourstate_alignments.push_back( apegrunt::transform_alignment<apegrunt::triallelic_state_t>( alignment ) );
		if( SuperDCA_options::verbose() )
		{
			*SuperDCA_options::get_out_stream() << "SuperDCA: alignment \"" << alignment->id_string() << ":\n"; // has " << alignment->size() << " sequences and " << alignment->n_loci() << " loci.\n";
			alignment->statistics( SuperDCA_options::get_out_stream() );
		}
	}

	if( apegrunt_options.output_allele_frequencies() )
	{
		for( auto& alignment: fourstate_alignments )
		{
			apegrunt::output_frequencies( alignment );
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
			Exit(EXIT_FAILURE);
		}
	}

	// construct a list of position indices to include in analysis, if we didn't get one from the user
    if( !loci_list )
    {
		const std::size_t end_locus = apegrunt_options.get_end_locus() < 0 ? fourstate_alignments.front()->n_loci() : apegrunt_options.get_end_locus()+1;
		const std::size_t begin_locus = apegrunt_options.get_begin_locus();
		std::vector<std::size_t> loci; loci.reserve(end_locus-begin_locus);
		for( std::size_t i = begin_locus; i < end_locus; ++i ) { loci.push_back(i); }
		loci_list = apegrunt::make_Loci_list( loci );
    }

	if( SuperDCA_options::verbose() ) {	*SuperDCA_options::get_out_stream() << "\nSuperDCA: run plmDCA.\n"; }

	// run the inference
	bool plmDCA_success = run_plmDCA<double>( fourstate_alignments, loci_list );

	if( SuperDCA_options::verbose() )
	{
		*SuperDCA_options::get_out_stream() << "SuperDCA: analysis completed.\n";
	}
	globaltimer.stop(); globaltimer.print_timing_stats();
	if(	!plmDCA_success )
	{
		*SuperDCA_options::get_err_stream() << "SuperDCA error: plmDCA failed.\n\n";
		Exit(EXIT_FAILURE);
	}
    Exit(EXIT_SUCCESS);
}
