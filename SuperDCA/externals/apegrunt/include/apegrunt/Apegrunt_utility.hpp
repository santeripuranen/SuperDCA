/** @file Apegrunt_utility.hpp

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
#ifndef APEGRUNT_UTILITY_HPP
#define APEGRUNT_UTILITY_HPP

#include <iostream>
#include <iomanip>
#include <cmath>

#include <boost/tuple/tuple.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/iterator/zip_iterator.hpp>

namespace apegrunt {

// Helpers for iterables that are passed in shared_ptrs
template< typename T > auto begin( std::shared_ptr<T>& sptr ) { return begin(*sptr); }
template< typename T > auto end( std::shared_ptr<T>& sptr ) { return end(*sptr); }

template< typename T > auto begin( const std::shared_ptr<T>& sptr ) { return begin(*sptr); }
template< typename T > auto end( const std::shared_ptr<T>& sptr ) { return end(*sptr); }

template< typename T > auto cbegin( const std::shared_ptr<T>& sptr ) { return cbegin(*sptr); }
template< typename T > auto cend( const std::shared_ptr<T>& sptr ) { return cend(*sptr); }

template< typename... Containers >
auto zip_range( Containers&&... containers ) // universal reference; will bind to anything
	-> decltype(
		boost::make_iterator_range(
			boost::make_zip_iterator( boost::make_tuple( begin(containers) ... ) ),
			boost::make_zip_iterator( boost::make_tuple( end(containers) ... ) )
		)
	)
{
	return { boost::make_zip_iterator( boost::make_tuple( begin(containers) ... ) ),
		     boost::make_zip_iterator( boost::make_tuple( end(containers) ... ) )
	};
}

struct my_div_t { uint64_t quot; uint64_t rem; };
my_div_t my_div( uint64_t n, uint64_t div ) { my_div_t result{}; result.quot=n/div; result.rem=n%div; return result; }

struct memory_string
{
	memory_string( uint64_t mem_in_bytes ) : m_mem_in_bytes(mem_in_bytes) { }
	~memory_string() { }

	std::ostream& operator()( std::ostream& os ) const
	{
		my_div_t result{};
		result.quot = m_mem_in_bytes;
		std::size_t n = 0;

		//std::cout << m_elapsed_time << std::endl;

		while( result.quot > 1024 && n < 4 )
		{
			++n;
			result = my_div(result.quot,1000);
			//std::cout << "rem=" << result.rem << " quot=" << result.quot << " n=" << n << "\n";
		};

		std::string unit;
		switch(n)
		{
			case 0: unit = "B"; break;
			case 1: unit = "KiB"; break;
			case 2: unit = "MiB"; break;
			case 3: unit = "GiB"; break;
			case 4: unit = "TiB"; break;
		}

		os << std::fixed << std::setprecision(2) << double(m_mem_in_bytes)/double(std::pow(1024,n)) << unit;

		return os;
	}

	uint64_t m_mem_in_bytes;
};

std::ostream& operator<< ( std::ostream& os, const memory_string& mem )
{
	return mem(os);
}



} // namespace apegrunt

#endif // APEGRUNT_UTILITY_HPP
