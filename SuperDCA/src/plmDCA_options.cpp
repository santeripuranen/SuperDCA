/** @file plmDCA_options.cpp

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

#include "plmDCA_options.h"

namespace superdca {

std::ostream* plmDCA_options::s_out = nullptr;
std::ostream* plmDCA_options::s_err = nullptr;
uint plmDCA_options::s_state = 1;
bool plmDCA_options::s_verbose = false;

std::string plmDCA_options::s_logfile_name = "SuperDCA.log";
std::string plmDCA_options::s_errfile_name = "SuperDCA.err";
std::string plmDCA_options::s_outfile_name = "SuperDCA.out";

#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
int plmDCA_options::s_threads = -1;
#else
int plmDCA_options::s_threads = 1;
#endif // SUPERDCA_NO_TBB
#ifndef SUPERDCA_NO_MPI
int plmDCA_options::s_nodes = -1;
#else
int plmDCA_options::s_nodes = 1;
#endif // SUPERDCA_NO_MPI
bool plmDCA_options::s_use_cuda = false;

double plmDCA_options::s_reweighting_threshold = 0.9; // default=1.0, i.e. all elements should be identical for two sequences to be considered as one

bool plmDCA_options::s_output_weights = false;
bool plmDCA_options::s_no_reweighting = false;

bool plmDCA_options::s_no_estimate = true; //false;
bool plmDCA_options::s_no_dca = false;
bool plmDCA_options::s_no_coupling_output = false;

uint plmDCA_options::s_fp_precision = 32;
bool plmDCA_options::s_norm_of_mean_scoring = false;
int plmDCA_options::s_keep_n_best_couples = 1e7;
bool plmDCA_options::s_store_parameter_matrices_to_disk = false;

double plmDCA_options::s_gradient_threshold = 1e-3;
double plmDCA_options::s_lambda_h = -1.0;
double plmDCA_options::s_lambda_J = -1.0;

plmDCA_options::plmDCA_options() { this->m_init(); }
plmDCA_options::~plmDCA_options() { }

plmDCA_options::plmDCA_options( std::ostream *out, std::ostream *err )
{
	plmDCA_options::s_out = out;
	plmDCA_options::s_err = err;
	this->m_init();
}

uint plmDCA_options::state() const { return s_state; }

std::ostream* plmDCA_options::out_stream() { return s_out; }
void plmDCA_options::set_out_stream( std::ostream *out ) { s_out = out->good() ? out : nullptr; }

std::ostream* plmDCA_options::err_stream() { return s_err; }
void plmDCA_options::set_err_stream( std::ostream *err ) { s_err = err->good() ? err : nullptr; }

bool plmDCA_options::verbose() { return ( s_verbose && s_out ); } // be verbose only if we have a valid ostream*.
void plmDCA_options::set_verbose( bool verbose ) { s_verbose = verbose; }

bool plmDCA_options::cuda() { return s_use_cuda; }
#ifndef SUPERDCA_NO_CUDA
void plmDCA_options::set_cuda( bool use_cuda ) { s_use_cuda = use_cuda; }
#else
void plmDCA_options::set_cuda( bool use_cuda ) { } // do nothing
#endif // SUPERDCA_NO_CUDA

int plmDCA_options::nodes() { return s_nodes; }
#ifndef SUPERDCA_NO_MPI
void plmDCA_options::set_nodes( int nnodes ) { s_nodes = nnodes; }
#else
void plmDCA_options::set_nodes( int nnodes ) { } // do nothing
#endif // SUPERDCA_NO_MPI

int plmDCA_options::threads() { return s_threads; }
#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
void plmDCA_options::set_threads( int nthreads ) { s_threads = nthreads; }
#else
void plmDCA_options::set_threads( int nthreads ) { } // do nothing
#endif // SUPERDCA_NO_TBB

// alignment preprocessing
double plmDCA_options::reweighting_threshold() { return s_reweighting_threshold; }
bool plmDCA_options::reweight() { return !s_no_reweighting; }
bool plmDCA_options::output_weights() { return s_output_weights; }

// algorithm and scoring
uint plmDCA_options::fp_precision() { return s_fp_precision; }
bool plmDCA_options::norm_of_mean_scoring() { return s_norm_of_mean_scoring; }
bool plmDCA_options::store_parameter_matrices_to_disk() { return s_store_parameter_matrices_to_disk; }

int plmDCA_options::keep_n_best_couples() { return s_keep_n_best_couples; }
void plmDCA_options::set_keep_n_best_couples( int n ) { s_keep_n_best_couples=n; }

double plmDCA_options::gradient_threshold() { return s_gradient_threshold; }
void plmDCA_options::set_gradient_threshold( double threshold ) { s_gradient_threshold = threshold; }
double plmDCA_options::lambda_h() { return s_lambda_h; }
void plmDCA_options::set_lambda_h( double val ) { s_lambda_h = val; }
double plmDCA_options::lambda_J() { return s_lambda_J; }
void plmDCA_options::set_lambda_J( double val ) { s_lambda_J = val; }

bool plmDCA_options::no_estimate() { return s_no_estimate; }
bool plmDCA_options::no_dca() { return s_no_dca; }
bool plmDCA_options::no_coupling_output() { return s_no_coupling_output; }

void plmDCA_options::m_init()
{
	namespace po = boost::program_options;

	m_alignment_options.add_options()
		("no-reweighting", po::bool_switch( &plmDCA_options::s_no_reweighting )->default_value(plmDCA_options::s_no_reweighting)->notifier(plmDCA_options::s_init_no_reweighting), "Do not reweight samples i.e. do not try to correct for population structure.")
		("reweighting-threshold", po::value< double >( &plmDCA_options::s_reweighting_threshold )->default_value(plmDCA_options::s_reweighting_threshold)->notifier(plmDCA_options::s_init_reweighting_threshold), "Fraction of identical positions required for two sequences to be considered identical.")
		("output-weights", po::bool_switch( &plmDCA_options::s_output_weights )->default_value(plmDCA_options::s_output_weights)->notifier(plmDCA_options::s_init_output_weights), "Write sample weights to file.")
	;
	m_algorithm_options.add_options()
		("norm-of-mean-scoring", po::bool_switch( &plmDCA_options::s_norm_of_mean_scoring )->default_value(plmDCA_options::s_norm_of_mean_scoring)->notifier(plmDCA_options::s_init_norm_of_mean_scoring), "Calculate coupling score as the mean of J(ij) and J(ji) matrices (may require tons of memory).")
//      ("store_parameter_matrices_to_disk", po::bool_switch( &plmDCA_options::s_store_parameter_matrices_to_disk )->default_value(plmDCA_options::s_store_parameter_matrices_to_disk)->notifier(plmDCA_options::s_init_store_parameter_matrices_to_disk), "Store parameter matrices to disk (may require tons of disk space).")
//		("keep-n-best-couples", po::value< int >( &plmDCA_options::s_keep_n_best_couples )->default_value(plmDCA_options::s_keep_n_best_couples)->notifier(plmDCA_options::s_init_keep_n_best_couples), "The number of best solutions that are stored (-1=keep all). Has huge effect on the amount of memory used.")

		("gradient-threshold", po::value< double >( &plmDCA_options::s_gradient_threshold )->default_value(plmDCA_options::s_gradient_threshold)->notifier(plmDCA_options::s_init_gradient_threshold), "L-BFGS gradient threshold stopping criterion.")
		("lambda-h", po::value< double >( &plmDCA_options::s_lambda_h )->default_value(plmDCA_options::s_lambda_h)->notifier(plmDCA_options::s_init_lambda_h), "h vector regularization factor (if lambda_h < 0.0, then value is automatically determined).")
		("lambda-J", po::value< double >( &plmDCA_options::s_lambda_J )->default_value(plmDCA_options::s_lambda_J)->notifier(plmDCA_options::s_init_lambda_J), "J matrix regularization factor (if lambda_J < 0.0, then value is automatically determined).")
//		("fp-precision", po::value< uint >( &plmDCA_options::s_fp_precision )->default_value(plmDCA_options::s_fp_precision)->notifier(plmDCA_options::s_init_fp_precision), "Floating point precision in bits.")
//		("no-estimate", po::bool_switch( &plmDCA_options::s_no_estimate )->default_value(plmDCA_options::s_no_estimate)->notifier(plmDCA_options::s_init_no_estimate), "Don't initialize DCA with estimate.")
//		("no-dca", po::bool_switch( &plmDCA_options::s_no_dca )->default_value(plmDCA_options::s_no_dca)->notifier(plmDCA_options::s_init_no_dca), "Don't run DCA (makes sense only if one wishes to run and output results using the initial estimate alone).")
		("no-coupling-output", po::bool_switch( &plmDCA_options::s_no_coupling_output )->default_value(plmDCA_options::s_no_coupling_output)->notifier(plmDCA_options::s_init_no_coupling_output), "Don't write coupling scores to file. This option is provided for benchmarking purposes.")
	;
}

void plmDCA_options::AddOptions( boost::program_options::options_description *opdesc )
{
	namespace po = boost::program_options;
	opdesc->add(m_alignment_options);
	opdesc->add(m_algorithm_options);
}

/// Check options stored in varmap. Return false if a fatal inconsistency is detected.
bool plmDCA_options::CheckOptions( boost::program_options::variables_map *varmap )
{
	try
	{
		if( s_verbose && s_out )
		{
			*s_out << "plmDCA: being verbose." << std::endl;
		}

	}
	catch( std::exception& e)
	{
		if( s_err )
		{
			// probably an unknown option
			*s_err << "plmDCA error: " << e.what() << "\n\n";
		}
		return false;
	}
	catch(...)
	{
		if( s_err )
		{
			*s_err << "plmDCA error: Exception of unknown type!\n\n";
		}
		return false;
	}

	return true;
}

void plmDCA_options::s_init_verbose( bool verbose )
{
	if( verbose && s_out )
	{
		*s_out << "plmDCA: begin verbose.\n";
	}
}

void plmDCA_options::s_init_no_reweighting( bool flag )
{
	if( flag && s_verbose && s_out )
	{
		*s_out << "plmDCA: do not reweight samples (i.e. do not try to correct for population structure).\n";
	}
}

void plmDCA_options::s_init_output_weights( bool flag )
{
	if( flag && s_verbose && s_out )
	{
		*s_out << "plmDCA: output sample weights to file.\n";
	}
}

#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
void plmDCA_options::s_init_threads( int nthreads )
{
	if( s_verbose && s_out )
	{
		*s_out << "plmDCA: user requests " << nthreads << " compute threads.\n";
	}
}
#endif // SUPERDCA_NO_TBB

#ifndef SUPERDCA_NO_MPI
void plmDCA_options::s_init_nodes( uint nnodes )
{
	if( s_verbose && s_out )
	{
		*s_out << "plmDCA: user requests " << nnodes << " compute nodes.\n";
	}
}
#endif // SUPERDCA_NO_MPI

#ifndef SUPERDCA_NO_CUDA
void plmDCA_options::s_init_use_cuda( bool use_cuda )
{
	if( s_verbose && s_out && s_use_cuda )
	{
		*s_out << "plmDCA: use CUDA if available.\n";
	}
}
#endif // SUPERDCA_NO_CUDA

void plmDCA_options::s_init_fp_precision( uint fp_precision )
{
	if( s_verbose && s_out )
	{
		*s_out << "plmDCA: floating point precision set to " << fp_precision << ".\n";
	}
}

void plmDCA_options::s_init_reweighting_threshold( double threshold )
{
	if( s_verbose && s_out )
	{
		*s_out << "plmDCA: reweighting threshold set to " << threshold << ".\n";
	}
}

void plmDCA_options::s_init_norm_of_mean_scoring( bool flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "plmDCA: calculate coupling score as the mean of J(ij) and J(ji) matrices (may require tons of memory).\n";
	}
}

void plmDCA_options::s_init_keep_n_best_couples( int n )
{
	if( s_verbose && s_out )
	{
		if( n > 0 )
		{
			*s_out << "plmDCA: keep " << n << " best scoring couples.\n";
		}
		if( n < 0 )
		{
			*s_out << "plmDCA: keep all couples, regardless of score (may require tons of memory).\n";
		}
		if( n == 0 )
		{
			*s_out << "plmDCA: keep no solutions at all. We won't do any actual DCA computations now, but we'll do all the pre-processing anyway.\n";
		}
	}
}

void plmDCA_options::s_init_gradient_threshold( double val )
{
	if( s_verbose && s_out && val >= 0.0 )
	{
		*s_out << "plmDCA: L-BFGS gradient threshold set to " << val <<".\n";
	}
}

void plmDCA_options::s_init_lambda_h( double val )
{
	if( s_verbose && s_out && val >= 0.0 )
	{
		*s_out << "plmDCA: lambda_h set to " << val <<".\n";
	}
}

void plmDCA_options::s_init_lambda_J( double val )
{
	if( s_verbose && s_out && val >= 0.0 )
	{
		*s_out << "plmDCA: lambda_J set to " << val <<".\n";
	}
}

void plmDCA_options::s_init_store_parameter_matrices_to_disk( bool flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "plmDCA: store coupling matrices to disk (may require tons of disk space).\n";
	}
}

void plmDCA_options::s_init_no_estimate( bool flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "plmDCA: will not initialize DCA with estimate.\n";
	}
}

void plmDCA_options::s_init_no_dca( bool flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "plmDCA: will not run DCA (are you *sure* this is what you want to do?).\n";
	}
}

void plmDCA_options::s_init_no_coupling_output( bool flag )
{
	if( s_verbose && s_out && flag )
	{
		*s_out << "plmDCA: will not write coupling scores to file (are you *sure* this is what you want to do?).\n";
	}
}

} // namespace superdca

