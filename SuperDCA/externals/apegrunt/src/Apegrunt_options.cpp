/** @file Apegrunt_options.cpp

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

#include "apegrunt/Apegrunt_options.h"
#include "apegrunt/Apegrunt_version.h"

namespace apegrunt {

std::ostream* Apegrunt_options::s_out = nullptr;
std::ostream* Apegrunt_options::s_err = nullptr;

uint Apegrunt_options::s_state = 1;
bool Apegrunt_options::s_verbose = false;

std::size_t Apegrunt_options::s_output_indexing_base = 1;
std::size_t Apegrunt_options::s_input_indexing_base = 1;

int Apegrunt_options::s_begin_locus=Apegrunt_options::s_input_indexing_base;
int Apegrunt_options::s_end_locus=-1;

std::string Apegrunt_options::s_outfile_name = "apegrunt.log";
std::string Apegrunt_options::s_errfile_name = "apegrunt.err";

#ifndef APEGRUNT_NO_TBB // Threading with Threading Building Blocks
int Apegrunt_options::s_threads = -1;
#else
int Apegrunt_options::s_threads = 1;
#endif // APEGRUNT_NO_TBB

#ifndef APEGRUNT_NO_CUDA
bool Apegrunt_options::s_use_cuda = true;
#else
bool Apegrunt_options::s_use_cuda = false;
#endif

#ifdef APEGRUNT_STANDALONE_BUILD
std::string Apegrunt_options::s_options_string;

const std::string Apegrunt_options::s_usage_string(
	  std::string("Usage: apegrunt") /*+ std::string(argv[0])*/ + " [options] <alignmentfile> [-o <outputfile>]\nOption '--help' will print a list of available options.\n"
);
#endif // APEGRUNT_STANDALONE_BUILD

const std::string Apegrunt_options::s_title_string(
	  std::string("Apegrunt: A library for parsing, processing and storing alignments of categorical variables.\n")
);

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

const std::string Apegrunt_options::s_version_string(
	std::string("Apegrunt version ") + std::to_string(Apegrunt_version::s_MajorVersion) + "." + std::to_string(Apegrunt_version::s_MinorVersion) + "." + std::to_string(Apegrunt_version::s_SubminorVersion)
	+ " revision " + TOSTRING(GIT_BRANCH) + "-" + TOSTRING(GIT_COMMIT_HASH) + " / " +
#ifdef __AVX2__
	"AVX2"
#elif __AVX__
	"AVX"
#elif __SSE2__
	"SSE2"
#else
	"generic"
#endif
	+ " build " + std::string(__DATE__) + " " + std::string(__TIME__)
);

const std::string Apegrunt_options::s_copyright_notice(
	std::string("Copyright (c) 2016-2017 Santeri Puranen\nLicensed under the GNU Affero General Public License version 3.\n\n")
	+ "THIS SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND."
);

const std::string Apegrunt_options::s_long_copyright_notice(
	std::string("Copyright (c) 2016-2017 Santeri Puranen\nLicensed under the GNU Affero General Public License version 3.\n\n")
	+ "This program is free software: you can redistribute it and/or modify\n"
	+ "it under the terms of the GNU Affero General Public License as\n"
	+ "published by the Free Software Foundation, either version 3 of the\n"
	+ "License, or (at your option) any later version.\n\n"
	+ "This program is distributed in the hope that it will be useful,\n"
	+ "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	+ "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
	+ "GNU Affero General Public License for more details.\n\n"
	+ "You should have received a copy of the GNU Affero General Public License\n"
	+ "along with this program. If not, see <http://www.gnu.org/licenses/>."
);

double Apegrunt_options::s_minor_allele_frequency_threshold = 0.01; // default=0.01, i.e. the alleles below 1% are ignored
double Apegrunt_options::s_gap_frequency_threshold = 0.15; // default=0.15, i.e. positions with more that 15% gaps are ignored
Threshold_rule<int> Apegrunt_options::s_allele_state_rule(">1");

bool Apegrunt_options::s_no_filter_alignment = false;
bool Apegrunt_options::s_output_filtered_alignment = false;
bool Apegrunt_options::s_output_allele_frequencies = false;

bool Apegrunt_options::s_no_optimize_column_order = true; // permanently disabled

bool Apegrunt_options::s_sample_alignment = false;
std::vector<double> Apegrunt_options::s_sample_fractions;

Apegrunt_options::Apegrunt_options() { this->m_init(); }
Apegrunt_options::~Apegrunt_options() { }

Apegrunt_options::Apegrunt_options( std::ostream *out, std::ostream *err )
{
	Apegrunt_options::s_out = out;
	Apegrunt_options::s_err = err;
	this->m_init();
}

const std::string& Apegrunt_options::s_get_copyright_notice_string() { return s_copyright_notice; }
const std::string& Apegrunt_options::s_get_usage_string() { return s_usage_string; }
const std::string& Apegrunt_options::s_get_version_string() { return s_version_string; }
const std::string& Apegrunt_options::s_get_title_string() { return s_title_string; }

uint Apegrunt_options::state() const { return s_state; }

void Apegrunt_options::set_out_stream( std::ostream *out ) { s_out = out->good() ? out : nullptr; }
void Apegrunt_options::set_err_stream( std::ostream *err ) { s_err = err->good() ? err : nullptr; }

std::ostream* Apegrunt_options::get_out_stream() { return s_out; }
std::ostream* Apegrunt_options::get_err_stream() { return s_err; }

bool Apegrunt_options::verbose() { return ( s_verbose && s_out ); } // be verbose only if we have a valid ostream*.
void Apegrunt_options::set_verbose( bool verbose ) { s_verbose = verbose; }

std::size_t Apegrunt_options::get_output_indexing_base() { return s_output_indexing_base; }
void Apegrunt_options::set_output_indexing_base( std::size_t base_index ) { s_output_indexing_base = base_index; }
std::size_t Apegrunt_options::get_input_indexing_base() { return s_input_indexing_base; }
void Apegrunt_options::set_input_indexing_base( std::size_t base_index ) { s_input_indexing_base = base_index; }
int Apegrunt_options::get_begin_locus() { return s_begin_locus-s_input_indexing_base; }
int Apegrunt_options::get_end_locus() { return s_end_locus-s_input_indexing_base; }

bool Apegrunt_options::cuda() { return s_use_cuda; }
#ifndef APEGRUNT_NO_CUDA
void Apegrunt_options::set_cuda( bool use_cuda ) { s_use_cuda = use_cuda; }
#else
void Apegrunt_options::set_cuda( bool use_cuda ) { } // do nothing
#endif // APEGRUNT_NO_CUDA

int Apegrunt_options::threads() { return s_threads; }
#ifndef APEGRUNT_NO_TBB // Threading with Threading Building Blocks
void Apegrunt_options::set_threads( int nthreads ) { s_threads = nthreads; }
#else
void Apegrunt_options::set_threads( int nthreads ) { } // do nothing
#endif // APEGRUNT_NO_TBB

// alignment filtering
bool Apegrunt_options::fuse_duplicates() { return s_fuse_duplicates; }
double Apegrunt_options::get_minor_allele_frequency_threshold() { return s_minor_allele_frequency_threshold; }
double Apegrunt_options::get_gap_frequency_threshold() { return s_gap_frequency_threshold; }
Threshold_rule<int> Apegrunt_options::get_allele_state_rule() { return s_allele_state_rule; }
bool Apegrunt_options::filter_alignment() { return !s_no_filter_alignment; }
bool Apegrunt_options::output_filtered_alignment() { return s_output_filtered_alignment; }
bool Apegrunt_options::output_allele_frequencies() { return s_output_allele_frequencies; }
bool Apegrunt_options::optimize_column_order() { return !s_no_optimize_column_order; }
bool Apegrunt_options::sample_alignment() { return s_sample_alignment; }
const std::vector<double>& Apegrunt_options::sample_fractions() { return s_sample_fractions; }

void Apegrunt_options::m_init()
{
	namespace po = boost::program_options;

#ifdef APEGRUNT_STANDALONE_BUILD
m_general_options.add_options()
		("outfile", po::value< std::string >( &Apegrunt_options::s_outfile_name )->default_value(Apegrunt_options::s_outfile_name), "Log filename.")
		("errfile", po::value< std::string >( &Apegrunt_options::s_errfile_name )->default_value(Apegrunt_options::s_errfile_name), "Error log filename.")
	;
	m_parallel_options.add_options()
#ifndef APEGRUNT_NO_TBB // Threading with Threading Building Blocks
		("threads,t", po::value< int >( &Apegrunt_options::s_threads )->default_value(Apegrunt_options::s_threads)->notifier(Apegrunt_options::s_init_threads), "Number of threads per shared memory node (-1=use all hardware threads that the OS/environment exposes).")
#endif // APEGRUNT_NO_TBB
#ifndef APEGRUNT_NO_CUDA
		("cuda", po::bool_switch( &Apegrunt_options::s_use_cuda )->default_value(Apegrunt_options::s_use_cuda)->notifier(Apegrunt_options::s_init_use_cuda), "Use CUDA devices, if available.")
#endif // APEGRUNT_NO_CUDA
	;
#endif // APEGRUNT_STANDALONE_BUILD

	m_alignment_options.add_options()
		("no-filter-alignment", po::bool_switch( &Apegrunt_options::s_no_filter_alignment )->default_value(Apegrunt_options::s_no_filter_alignment)->notifier(Apegrunt_options::s_init_no_filter_alignment), "Do not reduce the number of apegrunt input loci by applying MAF and GAP thresholds.")
		("output-filtered-alignment", po::bool_switch( &Apegrunt_options::s_output_filtered_alignment )->default_value(Apegrunt_options::s_output_filtered_alignment)->notifier(Apegrunt_options::s_init_output_filtered_alignment), "Write filtered alignment to file.")
		("output-allele-frequencies", po::bool_switch( &Apegrunt_options::s_output_allele_frequencies )->default_value(Apegrunt_options::s_output_allele_frequencies)->notifier(Apegrunt_options::s_init_output_allele_frequencies), "Write allele frequencies to file.")
		("input-indexing-base", po::value< std::size_t >( &Apegrunt_options::s_input_indexing_base )->default_value(Apegrunt_options::s_input_indexing_base)->notifier(Apegrunt_options::s_init_input_indexing_base), "Base index for input." )
		("output-indexing-base", po::value< std::size_t >( &Apegrunt_options::s_output_indexing_base )->default_value(Apegrunt_options::s_output_indexing_base)->notifier(Apegrunt_options::s_init_output_indexing_base), "Base index for output." )
//		("no-optimize-column-order", po::bool_switch( &Apegrunt_options::s_no_optimize_column_order )->default_value(Apegrunt_options::s_no_optimize_column_order), "Do not optimize data column order.")
		("maf-threshold", po::value< double >( &Apegrunt_options::s_minor_allele_frequency_threshold )->default_value(Apegrunt_options::s_minor_allele_frequency_threshold)->notifier(Apegrunt_options::s_init_minor_allele_frequency_threshold), "Minor allele frequency threshold. Alleles with a frequency below the threshold are excluded from the pair-analysis.")
		("gap-threshold", po::value< double >( &Apegrunt_options::s_gap_frequency_threshold )->default_value(Apegrunt_options::s_gap_frequency_threshold)->notifier(Apegrunt_options::s_init_gap_frequency_threshold), "Gap frequency threshold. Positions with a gap frequency above the threshold are excluded from the pair-analysis.")
//		("allele-state-rule", po::value< std::string >()/*->default_value( Apegrunt_options::s_allele_state_rule.str() )*/->notifier(Apegrunt_options::s_init_allele_state_rule), "Allele state filtering rule.")
	;
	m_algorithm_options.add_options()
		("begin", po::value< int >( &Apegrunt_options::s_begin_locus )->default_value(Apegrunt_options::s_begin_locus)->notifier(Apegrunt_options::s_init_begin_locus), "The first locus index to work on. Used to define a range.")
		("end", po::value< int >( &Apegrunt_options::s_end_locus )->default_value(Apegrunt_options::s_end_locus)->notifier(Apegrunt_options::s_init_end_locus), "The last locus index to work on (-1=end of input). Used to define a range.")
	;
}

void Apegrunt_options::AddOptions( boost::program_options::options_description *opdesc )
{
	namespace po = boost::program_options;
#ifdef APEGRUNT_STANDALONE_BUILD
	opdesc->add(m_general_options);
	opdesc->add(m_parallel_options);
#endif // APEGRUNT_STANDALONE_BUILD
	opdesc->add(m_alignment_options);
	opdesc->add(m_algorithm_options);
}

/// Check options stored in varmap. Return false if a fatal inconsistency is detected.
bool Apegrunt_options::CheckOptions( boost::program_options::variables_map *varmap )
{
	try
	{
		if( s_verbose && s_out )
		{
			*s_out << "apegrunt: Being verbose." << std::endl;
		}

	}
	catch( std::exception& e)
	{
		if( s_err )
		{
			// probably an unknown option
			*s_err << "apegrunt error: " << e.what() << "\n\n";
		}
		return false;
	}
	catch(...)
	{
		if( s_err )
		{
			*s_err << "apegrunt error: Exception of unknown type!\n\n";
		}
		return false;
	}

	return true;
}

void Apegrunt_options::s_init_verbose( bool verbose )
{
	if( verbose && s_out )
	{
		*s_out << "apegrunt: begin verbose.\n";
	}
}

void Apegrunt_options::s_init_no_filter_alignment( bool flag )
{
	if( flag && s_verbose && s_out )
	{
		*s_out << "apegrunt: do not reduce the number of apegrunt analyzable input loci by applying MAF and GAP thresholds.\n";
	}
}
void Apegrunt_options::s_init_output_filtered_alignment( bool flag )
{
	if( flag && s_verbose && s_out )
	{
		*s_out << "apegrunt: output filtered alignment to file.\n";
	}
}
void Apegrunt_options::s_init_output_allele_frequencies( bool flag )
{
	if( flag && s_verbose && s_out )
	{
		*s_out << "apegrunt: output allele frequencies to file.\n";
	}
}

#ifndef APEGRUNT_NO_TBB // Threading with Threading Building Blocks
void Apegrunt_options::s_init_threads( int nthreads )
{
	if( s_verbose && s_out )
	{
		*s_out << "apegrunt: user requests " << nthreads << " compute threads.\n";
	}
}
#endif // APEGRUNT_NO_TBB

#ifndef APEGRUNT_NO_CUDA
void Apegrunt_options::s_init_use_cuda( bool use_cuda )
{
	if( s_verbose && s_out && s_use_cuda )
	{
		*s_out << "apegrunt: use CUDA if available.\n";
	}
}
#endif // APEGRUNT_NO_CUDA

void Apegrunt_options::s_init_fuse_duplicates( bool flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "plmDCA: fuse duplicate sequences.\n";
	}
}

void Apegrunt_options::s_init_minor_allele_frequency_threshold( double threshold )
{
	if( s_verbose && s_out )
	{
		*s_out << "apegrunt: minor allele frequency threshold set to " << threshold << ".\n";
	}
}

void Apegrunt_options::s_init_gap_frequency_threshold( double threshold )
{
	if( s_verbose && s_out )
	{
		*s_out << "apegrunt: gap frequency threshold set to " << threshold << ".\n";
	}
}

void Apegrunt_options::s_init_sample_alignment( bool flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "apegrunt: create an alignment sample.\n";
	}
}

void Apegrunt_options::s_init_output_indexing_base( std::size_t base_index )
{
	if( s_verbose && s_out )
	{
		*s_out << "apegrunt: output indexing will begin at " << base_index << ".\n";
	}
}

void Apegrunt_options::s_init_input_indexing_base( std::size_t base_index )
{
	if( s_verbose && s_out )
	{
		*s_out << "apegrunt: will assume that input indexing begins at " << base_index << ".\n";
	}
}

void Apegrunt_options::s_init_allele_state_rule( const std::string& rule_string )
{
	s_allele_state_rule.set_rule(rule_string);
	if( s_verbose && s_out )
	{
		*s_out << "apegrunt: filter for loci that have " << s_allele_state_rule << " states.\n";
	}
}

void Apegrunt_options::s_init_begin_locus( int locus ) { }
void Apegrunt_options::s_init_end_locus( int locus ) { }

} // namespace apegrunt

