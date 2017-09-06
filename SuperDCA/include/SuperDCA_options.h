/** @file SuperDCA_options.h

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

#ifndef SUPERDCA_OPTIONS_H
#define SUPERDCA_OPTIONS_H

#include <iosfwd>

// Boost includes
#include <boost/program_options.hpp>

#include "SuperDCA_commons.h"

namespace po = boost::program_options;

namespace superdca {

class SuperDCA_options
{
public:
	SuperDCA_options();
	SuperDCA_options( std::ostream *out, std::ostream *err=nullptr );
	~SuperDCA_options();

	void AddOptions( po::options_description *opdesc );
	static bool CheckOptions( po::variables_map *varmap );

	uint state() const;

	bool has_alignment_filenames() const;
	const std::vector< std::string >& get_alignment_filenames() const;

	bool has_filterlist_filename() const;
	const std::string& get_filterlist_filename() const;

	bool has_samplelist_filename() const;
	const std::string& get_samplelist_filename() const;

	static bool output_SNPs();
	static bool output_filtered_alignment();
	static bool output_filterlist_alignment();
	static bool output_samplelist_alignment();
	//static bool translate_output_alignment();
	//static bool force_translation();
	//static bool complementary_read();

	bool has_locilist_filename() const;
	const std::string& get_locilist_filename() const;

	static void set_out_stream( std::ostream* out );
	static void set_err_stream( std::ostream* err );
	//> Set an ostream. An invalid ostream* (as in "out->good() == false", will reset internal ostream ("ostream* == null_ptr").
	static std::ostream* get_out_stream();
	static std::ostream* get_err_stream();

	static const std::string& s_get_copyright_notice_string();
	static const std::string& s_get_usage_string();
	static const std::string& s_get_version_string();
	static const std::string& s_get_title_string();

	static const std::string& s_get_options_string();
	void m_set_options_string( const std::string& options_string );

	//> Test if textual output is desired. If true, then a call to get_out_stream() is guaranteed to return a valid (as in != null_ptr) ostream*.
	static bool verbose();
	static bool cuda();
	static int nodes();
	static int threads();

private:
	static uint s_state; // 1 for normal operation, 0 signals a wish to terminate process
	static bool s_verbose;
	static std::ostream *s_out;
	static std::ostream *s_err;
	static std::string s_log_file_name;
	static std::string s_err_file_name;
	static std::string s_out_file_name;

	static int s_threads;
	static int s_nodes;
	static bool s_use_cuda;
	static bool	s_output_SNPs;
	static bool s_output_filtered_alignment;
	static bool	s_output_filterlist_alignment;
	static bool	s_output_samplelist_alignment;
	//static bool s_translate_output_alignment;
	//static bool s_force_translation;
	//static bool s_complementary_read;

	std::vector< std::string > m_alignment_file_names;
	std::string m_filterlist_file_name;
	std::string m_locilist_file_name;
	std::string m_samplelist_file_name;

	static const std::string s_title_string;
	static const std::string s_usage_string;
	static const std::string s_version_string;
	static const std::string s_copyright_notice;
	static const std::string s_long_copyright_notice;
	static std::string s_options_string;
	void m_init();

	static void s_init_verbose( const bool& verbose );
	static void s_init_threads( const int& nthreads );
	static void s_init_nodes( const int& nnodes );
	static void s_init_use_cuda( const bool& use_cuda );
	static void s_init_output_SNPs( const bool& flag );
	static void s_init_output_filtered_alignment( const bool& flag );
	static void s_init_output_filterlist_alignment( const bool& flag );
	static void s_init_output_samplelist_alignment( const bool& flag );
	//static void s_init_translate_output_alignment( const bool& flag );
	//static void s_init_force_translation( const bool& flag );
	//static void s_init_complementary_read( const bool& flag );

	po::options_description
		m_general_options/*("SuperDCA general options")*/,
		m_parallel_options/*("SuperDCA parallel options")*/
	;

};

} // namespace superdca

#endif // SUPERDCA_OPTIONS_H
