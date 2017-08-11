/** @file Stopwatch.hpp

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

#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <functional>
#include <algorithm>

#include <boost/timer/timer.hpp>

/* Usage:
int function() {

	const int N = 1000;
	stopwatch::stopwatch cputimer;
	cputimer.start();

	for( int i=0; i<N; ++i )
	{
		// do stuff
	}
	cputimer.stop();
	superdca::timer::print_timing_stats( cputimer, N );
}
*/

namespace stopwatch {

using std::cout;
using std::endl;

template< typename RealT1, typename RealT2 >
double x_per_n( RealT1 x, RealT2 n )
{
	return double(x) / double(n);
}

double avg_time_in_ns( std::size_t time, std::size_t N )
{
	return (double)time / (double)N;
}

struct my_div_t { uint64_t quot; uint64_t rem; };

my_div_t my_div( uint64_t n, uint64_t div ) { my_div_t result{}; result.quot=n/div; result.rem=n%div; return result; }

struct time_string
{
	time_string( uint64_t elapsed_time ) : m_elapsed_time(elapsed_time) { }
	~time_string() { }

	std::ostream& operator()( std::ostream& os ) const
	{
		my_div_t result{};
		result.quot = m_elapsed_time;
		std::size_t n = 0;

		//std::cout << m_elapsed_time << std::endl;

		while( result.quot > 1000 && n < 3 )
		{
			++n;
			result = my_div(result.quot,1000);
			//std::cout << "rem=" << result.rem << " quot=" << result.quot << " n=" << n << "\n";
		};

		std::string unit;
		switch(n)
		{
			case 0: unit = "ns"; break;
			case 1: unit = "micros"; break;
			case 2: unit = "ms"; break;
			case 3: unit = "s"; break;
		}

		os << std::fixed << std::setprecision(2) << double(m_elapsed_time)/double(std::pow(1000,n)) << unit;

		return os;
	}

	uint64_t m_elapsed_time;
};

std::ostream& operator<< ( std::ostream& os, const time_string& time )
{
	return time(os);
}

class stopwatch
{
public:
	stopwatch( std::ostream *out = &std::cout ) : m_out(out), m_tick_start(0), m_tick_stop(0) { }
	~stopwatch() { }

	// copy constructor; beware that timer state is not copied, only out stream.
	stopwatch( stopwatch& other ) : m_out(other.m_out), m_tick_start(0), m_tick_stop(0) { }

	void start()
	{
		m_timer.start();
		m_tick_start = s_rdtsc();
	}

	void stop()
	{
		m_tick_stop = s_rdtsc();
	}

	// get elapsed time in ns
	uint64_t elapsed_time() const
	{
		return m_timer.elapsed().wall;
	}

	// get elapsed clock cycles
	uint64_t elapsed_cycles() const
	{
		return m_tick_stop - m_tick_start;
	}

	void print_timing_stats( std::size_t Nops=1 )
	{
		if( !m_out /*|| !m_out->good()*/ ) { return; }
		const uint64_t elapsed_time = this->elapsed_time();
		//const uint64_t elapsed_cycles = this->elapsed_cycles();

		*m_out << "      done in: "
			<< time_string(elapsed_time) << " wall time";
/*		if( N > 1 ) { *m_out
			<< " (" avg_time_in_ns(elapsed_time,Nops) << " ns/op)\n"
			<< "\n      latency: " << x_per_n(elapsed_cycles,Nops) << " cycles/op";
		}
*/		*m_out << "\n";
	}

	std::ostream& operator()( std::ostream& os ) const
	{
		const uint64_t elapsed_time = this->elapsed_time();
		//const uint64_t elapsed_cycles = this->elapsed_cycles();

		return os << time_string(elapsed_time);
	}
	//std::ostream* get_out_stream() { return m_out; }

private:
	boost::timer::cpu_timer m_timer;
	std::ostream* m_out;
	uint64_t m_tick_start;
	uint64_t m_tick_stop;

#ifdef __i386__

	static __inline__ uint64_t s_rdtsc()
	{
		uint64_t x;
		__asm__ volatile ("rdtsc" : "=A" (x));
		return x;
	}
#elif defined(__x86_64__)

	static __inline__ uint64_t s_rdtsc()
	{
		uint64_t a, d;
		__asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
		return (d<<32) | a;
	}
#endif

};

std::ostream& operator<< ( std::ostream& os, const stopwatch& timer )
{
	return timer(os);
}

} // namespace stopwatch

#endif // STOPWATCH_HPP
