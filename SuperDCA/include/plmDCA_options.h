/** @file plmDCA_options.h

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

#ifndef SUPERDCA_PLMDCA_OPTIONS_H
#define SUPERDCA_PLMDCA_OPTIONS_H

#include <iosfwd>

// Boost includes
#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace superdca {

class plmDCA_options
{
public:
	plmDCA_options();
	plmDCA_options( std::ostream *out, std::ostream *err=nullptr );
	~plmDCA_options();

	void AddOptions( po::options_description *opdesc );
	static bool CheckOptions( po::variables_map *varmap );

	uint state() const;

	//> Set an ostream. An invalid ostream* (as in "out->good() == false", will reset internal ostream ("ostream* == null_ptr").
	static std::ostream* out_stream();
	static void set_out_stream( std::ostream* out );

	static std::ostream* err_stream();
	static void set_err_stream( std::ostream* err );

	static uint fp_precision();

	// algorithm and scoring
	static bool norm_of_mean_scoring();
	static bool output_parameter_matrices();
	static void set_keep_n_best_couples( int n );
	static int keep_n_best_couples();

	static void set_gradient_threshold( double val );
	static double gradient_threshold();
	static void set_lambda_h( double val );
	static double lambda_h();
	static void set_lambda_J( double val );
	static double lambda_J();

	static bool no_estimate();
	static bool no_dca();
	static bool no_coupling_output();

	//> Test if textual output is desired. If true, then a call to get_out_stream() is guaranteed to return a valid (as in != null_ptr) ostream*.
	static bool verbose();
	static void set_verbose( bool verbose=true );

	static bool cuda();
	static void set_cuda( bool use_cuda=true );

	static int nodes();
	static void set_nodes( int nnodes );

	static int threads();
	static void set_threads( int nthreads );

private:
	static uint s_state; // 1 for normal operation, 0 signals a wish to terminate process
	static bool s_verbose;

	static int s_begin_locus;
	static int s_end_locus;

	static bool s_no_estimate;
	static bool s_no_dca;
	static bool s_no_coupling_output;

	static bool s_output_parameter_matrices;

	static std::ostream *s_out;
	static std::ostream *s_err;
	static std::string s_logfile_name;
	static std::string s_errfile_name;
	static std::string s_outfile_name;

	static uint s_fp_precision;
	static bool s_norm_of_mean_scoring;
	static int s_keep_n_best_couples;

	static double s_gradient_threshold;
	static double s_lambda_h;
	static double s_lambda_J;

	static int s_threads;
	static int s_nodes;
	static bool s_use_cuda;

	void m_init();

	static void s_init_verbose( bool verbose );
#ifndef SUPERDCA_NO_TBB // Threading with Threading Building Blocks
	static void s_init_threads( int nthreads );
#endif // SUPERDCA_NO_TBB
#ifndef SUPERDCA_NO_MPI
	static void s_init_nodes( uint nnodes );
#endif // SUPERDCA_NO_MPI
#ifndef SUPERDCA_NO_CUDA
	static void s_init_use_cuda( bool use_cuda );
#endif // SUPERDCA_NO_CUDA
	static void s_init_fp_precision( uint fp_precision );
	static void s_init_norm_of_mean_scoring( bool flag );
	static void s_init_output_parameter_matrices( bool flag );
	static void s_init_keep_n_best_couples( int n );
	static void s_init_gradient_threshold( double val );
	static void s_init_lambda_h( double val );
	static void s_init_lambda_J( double val );
	static void s_init_store_parameter_matrices_to_disk( bool flag );
	static void s_init_no_estimate( bool flag );
	static void s_init_no_dca( bool flag );
	static void s_init_no_coupling_output( bool flag );

	po::options_description
#ifdef PLMDCA_STANDALONE_BUILD
		m_general_options /*("plmDCA general options")*/,
		m_parallel_options /*("plmDCA parallel options")*/,
#endif // PLMDCA_STANDALONE_BUILD
		m_alignment_options /*("plmDCA alignment preprocessing options")*/,
		m_algorithm_options /*("plmDCA algorithm options")*/
	;

};

} // namespace superdca

#endif // SUPERDCA_PLMDCA_OPTIONS_H
