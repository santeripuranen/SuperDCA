/** @file SuperDCA_options.cpp

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

#include "SuperDCA_options.h"
#include "SuperDCA_version.h"

namespace superdca {

std::ostream* SuperDCA_options::s_out = nullptr;
std::ostream* SuperDCA_options::s_err = nullptr;
uint SuperDCA_options::s_state = 1;
bool SuperDCA_options::s_verbose = false;
bool SuperDCA_options::s_output_SNPs = false;
bool SuperDCA_options::s_output_filtered_alignment = false;
bool SuperDCA_options::s_output_filterlist_alignment = false;
bool SuperDCA_options::s_output_samplelist_alignment = false;
//bool SuperDCA_options::s_translate_output_alignment = false;
//bool SuperDCA_options::s_force_translation = false;
//bool SuperDCA_options::s_complementary_read = false;
std::string SuperDCA_options::s_log_file_name = "SuperDCA.log";
std::string SuperDCA_options::s_err_file_name = "SuperDCA.err";
std::string SuperDCA_options::s_out_file_name = "SuperDCA.out";

#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
int SuperDCA_options::s_threads = -1;
#else
int SuperDCA_options::s_threads = 1;
#endif // SUPERDCA_NO_TBB
#ifndef SUPERDCA_NO_MPI
int SuperDCA_options::s_nodes = -1;
#else
int SuperDCA_options::s_nodes = 1;
#endif // SUPERDCA_NO_MPI
bool SuperDCA_options::s_use_cuda = false;

std::string SuperDCA_options::s_options_string;

const std::string SuperDCA_options::s_title_string(
	  std::string("SuperDCA: Parameter inference by Pseudo-Likelihood Maximization Direct Coupling Analysis (plmDCA).\n")
);

const std::string SuperDCA_options::s_usage_string(
	  std::string("Usage: SuperDCA") /*+ std::string(argv[0])*/ + " [options] <alignmentfile> [-o <outputfile>]\nOption '--help' will print a list of available options.\n"
);

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

const std::string SuperDCA_options::s_version_string(
	std::string("SuperDCA version ") + std::to_string(SuperDCA_version::s_MajorVersion) + "." + std::to_string(SuperDCA_version::s_MinorVersion) + "." + std::to_string(SuperDCA_version::s_SubminorVersion)
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

const std::string SuperDCA_options::s_copyright_notice(
	std::string("Copyright (c) 2016-2017 Santeri Puranen\nLicensed under the GNU Affero General Public License version 3.\n\n")
	+ "THIS SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND."
);

const std::string SuperDCA_options::s_long_copyright_notice(
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

SuperDCA_options::SuperDCA_options() { this->m_init(); }
SuperDCA_options::~SuperDCA_options() { }

SuperDCA_options::SuperDCA_options( std::ostream *out, std::ostream *err )
{
	SuperDCA_options::s_out = out;
	SuperDCA_options::s_err = err;
	this->m_init();
}

const std::string& SuperDCA_options::s_get_copyright_notice_string() { return s_copyright_notice; }
const std::string& SuperDCA_options::s_get_usage_string() { return s_usage_string; }
const std::string& SuperDCA_options::s_get_version_string() { return s_version_string; }
const std::string& SuperDCA_options::s_get_title_string() { return s_title_string; }

uint SuperDCA_options::state() const { return s_state; }

bool SuperDCA_options::has_alignment_filenames() const { return !m_alignment_file_names.empty(); }
const std::vector< std::string >& SuperDCA_options::get_alignment_filenames() const { return m_alignment_file_names; }

bool SuperDCA_options::has_filterlist_filename() const { return !m_filterlist_file_name.empty(); }
const std::string& SuperDCA_options::get_filterlist_filename() const { return m_filterlist_file_name; }

bool SuperDCA_options::has_samplelist_filename() const { return !m_samplelist_file_name.empty(); }
const std::string& SuperDCA_options::get_samplelist_filename() const { return m_samplelist_file_name; }

bool SuperDCA_options::output_SNPs() { return s_output_SNPs; }
bool SuperDCA_options::output_filtered_alignment() { return s_output_filtered_alignment; }
bool SuperDCA_options::output_filterlist_alignment() { return s_output_filterlist_alignment; }
bool SuperDCA_options::output_samplelist_alignment() { return s_output_samplelist_alignment; }
//bool SuperDCA_options::translate_output_alignment() { return s_translate_output_alignment; }
//bool SuperDCA_options::force_translation() { return s_force_translation; }
//bool SuperDCA_options::complementary_read() { return s_complementary_read; }

bool SuperDCA_options::has_locilist_filename() const { return !m_locilist_file_name.empty(); }
const std::string& SuperDCA_options::get_locilist_filename() const { return m_locilist_file_name; }

void SuperDCA_options::set_out_stream( std::ostream *out ) { s_out = out->good() ? out : nullptr; }
void SuperDCA_options::set_err_stream( std::ostream *err ) { s_err = err->good() ? err : nullptr; }

std::ostream* SuperDCA_options::get_out_stream() { return s_out; }
std::ostream* SuperDCA_options::get_err_stream() { return s_err; }

const std::string& SuperDCA_options::s_get_options_string() { return s_options_string; }
void SuperDCA_options::m_set_options_string( const std::string& options_string ) { s_options_string = options_string; }

bool SuperDCA_options::verbose() { return ( s_verbose && s_out ); } // be verbose only if we have a valid outstream.

bool SuperDCA_options::cuda() { return s_use_cuda; }

int SuperDCA_options::nodes() { return s_nodes; }

int SuperDCA_options::threads() { return s_threads; }

void SuperDCA_options::m_init()
{
	namespace po = boost::program_options;

	m_general_options.add_options()
		("help,h", "Print this help message.")
		("verbose,v", po::bool_switch( &SuperDCA_options::s_verbose )->default_value(SuperDCA_options::s_verbose)->notifier(SuperDCA_options::s_init_verbose), "Be verbose.")
		("alignmentfile", po::value< std::vector< std::string > >( &m_alignment_file_names )->composing(), "The input alignment filename(s). When two filenames are specified, only inter-alignment links will be probed for.")
		("filterlistfile", po::value< std::string >( &m_filterlist_file_name ), "The loci filter list input filename.")
//		("output-SNPs", po::bool_switch( &SuperDCA_options::s_output_SNPs )->default_value(SuperDCA_options::s_output_SNPs)->notifier(SuperDCA_options::s_init_output_SNPs), "Extract and output all SNPs.")
		("output-filtered-alignment", po::bool_switch( &SuperDCA_options::s_output_filtered_alignment )->default_value(SuperDCA_options::s_output_filtered_alignment)->notifier(SuperDCA_options::s_init_output_filtered_alignment), "Write filtered alignment to file.")
		("output-filterlist-alignment", po::bool_switch( &SuperDCA_options::s_output_filterlist_alignment )->default_value(SuperDCA_options::s_output_filterlist_alignment)->notifier(SuperDCA_options::s_init_output_filterlist_alignment), "Output alignment after filterlist selection.")
		("samplelistfile", po::value< std::string >( &m_samplelist_file_name ), "The sample filter list input filename.")
		("output-samplelist-alignment", po::bool_switch( &SuperDCA_options::s_output_samplelist_alignment )->default_value(SuperDCA_options::s_output_samplelist_alignment)->notifier(SuperDCA_options::s_init_output_samplelist_alignment), "Output alignment after samplelist selection.")
//		("translate-output-alignment", po::bool_switch( &SuperDCA_options::s_translate_output_alignment )->default_value(SuperDCA_options::s_translate_output_alignment)->notifier(SuperDCA_options::s_init_translate_output_alignment), "Output alignments are translated into amino acid sequences.")
//		("force-translation", po::bool_switch( &SuperDCA_options::s_force_translation )->default_value(SuperDCA_options::s_force_translation)->notifier(SuperDCA_options::s_init_force_translation), "Ignore start and stop codons when performing translation.")
//		("complementary-read", po::bool_switch( &SuperDCA_options::s_complementary_read )->default_value(SuperDCA_options::s_complementary_read)->notifier(SuperDCA_options::s_init_complementary_read), "Read sequence from the complementary strand.")
		("locilistfile", po::value< std::string >( &m_locilist_file_name ), "Analysable loci list filename.")
		//("outfile", po::value< std::string >( &SuperDCA_options::s_out_file_name )->default_value(SuperDCA_options::s_out_file_name), "The output filename.")
		//("logfile", po::value< std::string >( &SuperDCA_options::s_log_file_name )->default_value(SuperDCA_options::s_log_file_name), "Log filename.")
		//("errfile", po::value< std::string >( &SuperDCA_options::s_err_file_name )->default_value(SuperDCA_options::s_err_file_name), "Error log filename.")
	;
	m_parallel_options.add_options()
#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
		("threads,t", po::value< int >( &SuperDCA_options::s_threads )->default_value(SuperDCA_options::s_threads)->notifier(SuperDCA_options::s_init_threads), "Number of threads per MPI/shared memory node (-1=use all hardware threads that the OS/environment exposes).")
#endif // SUPERDCA_NO_TBB
/*
#ifndef SUPERDCA_NO_MPI
		("nodes,n", po::value< int >( &SuperDCA_options::s_nodes )->default_value(SuperDCA_options::s_nodes)->notifier(SuperDCA_options::s_init_nodes), "Number of MPI/distributed memory nodes (1=single node; no MPI).")
#endif // SUPERDCA_NO_MPI
#ifndef SUPERDCA_NO_CUDA
		("cuda", po::bool_switch( &SuperDCA_options::s_use_cuda )->default_value(SuperDCA_options::s_use_cuda)->notifier(SuperDCA_options::s_init_use_cuda), "Use CUDA.")
#endif // SUPERDCA_NO_CUDA
*/
	;
}

void SuperDCA_options::AddOptions( boost::program_options::options_description *opdesc )
{
	namespace po = boost::program_options;
	opdesc->add(m_general_options);
	opdesc->add(m_parallel_options);
}

/// Check options stored in varmap. Return false if a fatal inconsistency is detected.
bool SuperDCA_options::CheckOptions( boost::program_options::variables_map *varmap )
{
	try
	{
		if( varmap->count("help") && s_out )
		{
			*s_out << s_title_string << "\n" << s_usage_string << s_options_string << std::endl;
			superdca::Exit(EXIT_SUCCESS);
		}
/*
		if( s_verbose && s_out )
		{
			*s_out << "SuperDCA: Being verbose." << std::endl;
		}
*/
		if( !varmap->count("alignmentfile") && s_err )
		{
			*s_err << "SuperDCA ERROR: No alignment file specified!" << std::endl;
			if( s_out )
			{
				*s_out << s_usage_string << std::endl;
			}
			return false;
		}

    }
	catch( std::exception& e)
	{
		if( s_err )
		{
			// probably an unknown option
			*s_err << "SuperDCA error: " << e.what() << "\n\n";
			*s_err << s_usage_string << "\n\n";
		}
		return false;
	}
	catch(...)
	{
		if( s_err )
		{
			*s_err << "SuperDCA error: Exception of unknown type!\n\n";
		}
		return false;
	}

	return true;
}

void SuperDCA_options::s_init_verbose( const bool& verbose )
{
	if( verbose && s_out )
	{
		*s_out << "SuperDCA: being verbose.\n";
	}
}

#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
void SuperDCA_options::s_init_threads( const int& nthreads )
{
	if( s_verbose && s_out )
	{
		*s_out << "SuperDCA: user requests " << nthreads << " compute threads.\n";
	}
}
#endif // SUPERDCA_NO_TBB

#ifndef SUPERDCA_NO_MPI
void SuperDCA_options::s_init_nodes( const int& nnodes )
{
	if( s_verbose && s_out )
	{
		*s_out << "SuperDCA: user requests " << nnodes << " compute nodes.\n";
	}
}
#endif // SUPERDCA_NO_MPI

#ifndef SUPERDCA_NO_CUDA
void SuperDCA_options::s_init_use_cuda( const bool& use_cuda )
{
	if( s_verbose && s_out && s_use_cuda )
	{
		*s_out << "SuperDCA: use CUDA if available.\n";
	}
}
#endif // SUPERDCA_NO_CUDA
void SuperDCA_options::s_init_output_SNPs( const bool& flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "SuperDCA: will extract and output all SNPs.\n";
	}
}
void SuperDCA_options::s_init_output_filtered_alignment( const bool& flag )
{
	if( flag && s_verbose && s_out )
	{
		*s_out << "SuperDCA: output filtered alignment to file.\n";
	}
}

void SuperDCA_options::s_init_output_filterlist_alignment( const bool& flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "SuperDCA: will output alignment after filterlist selection.\n";
	}
}

void SuperDCA_options::s_init_output_samplelist_alignment( const bool& flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "SuperDCA: will output alignment after samplelist selection.\n";
	}
}
/*
void SuperDCA_options::s_init_translate_output_alignment( const bool& flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "SuperDCA: will translate output alignment(s) into amino acid sequences.\n";
	}
}

void SuperDCA_options::s_init_force_translation( const bool& flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "SuperDCA: will ignore start and stop codons in translation of alignment(s) into amino acid sequences.\n";
	}
}

void SuperDCA_options::s_init_complementary_read( const bool& flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "SuperDCA: will perform complementary strand read.\n";
	}
}
*/
} // namespace superdca

