/** @file plmDCA_utility.hpp

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
#ifndef PLMDCA_UTILITY_HPP
#define PLMDCA_UTILITY_HPP

#include <array>

#include "Matrix_math.hpp"
#include "Coupling_matrix_view.hpp"

namespace superdca {

template< typename MatrixViewT >
auto gauge_shift( MatrixViewT&& J_ij, bool transpose_input=false )
{
	// J_ij: J_ij as estimated from from g_i.

	// Shift the coupling estimates into the Ising gauge.
	const auto Jt_ij = transpose(J_ij);

	if( transpose_input )
	{
	    const auto J_ij_mean = mean(Jt_ij);
	    const auto J_ij_mean_mean = mean(J_ij_mean);
	    const auto J_ij_mean2 = mean( J_ij );

	    return Jt_ij - repmat(J_ij_mean) - repmat(J_ij_mean2,true) + J_ij_mean_mean;
	}
	else
	{
		const auto J_ij_mean = mean(J_ij);
		const auto J_ij_mean_mean = mean(J_ij_mean);
		const auto J_ij_mean2 = mean( Jt_ij );

		return J_ij - repmat(J_ij_mean) - repmat(J_ij_mean2,true) + J_ij_mean_mean;
	}
}

template< typename RealT, std::size_t SquareMatrixSize >
auto gauge_shift( Coupling_matrix_view<RealT,SquareMatrixSize>& Js, std::size_t n, bool transpose_input=false )
{
	// J_ij: J_ij as estimated from from g_i.
	auto J_ij = convert( Js, n );

	//std::cout << "J(" << n << "): " << J_ij << std::endl;

	// Shift the coupling estimates into the Ising gauge.
	const auto Jt_ij = transpose(J_ij);

	if( transpose_input )
	{
	    const auto J_ij_mean = mean(Jt_ij);
	    const auto J_ij_mean_mean = mean(J_ij_mean);
	    const auto J_ij_mean2 = mean( J_ij );

		return Jt_ij - repmat(J_ij_mean) - repmat(J_ij_mean2,true) + J_ij_mean_mean;
	    //auto mat = Jt_ij - repmat(J_ij_mean) - repmat(J_ij_mean2,true) + J_ij_mean_mean;
		//if( negative_count(mat) != 0 ) { std::cout << "Gauge-shifted matrix contains " << negative_count(mat) << " non-positive elements" << std::endl; }
		//return mat;
	}
	else
	{
		const auto J_ij_mean = mean(J_ij);
		const auto J_ij_mean_mean = mean(J_ij_mean);
		const auto J_ij_mean2 = mean( Jt_ij );

		return J_ij - repmat(J_ij_mean) - repmat(J_ij_mean2,true) + J_ij_mean_mean;
	}
}
/*
template< typename RealT, std::size_t Extent >
std::array< std::array<RealT,Extent>, Extent > gauge_shift( const std::array< std::array<RealT,Extent>, Extent >& J_ij )
{
	std::cout << "gauge_shift( " << typeid(J_ij).name() << " )" << std::endl;
    // J_ij: J_ij as estimated from from g_i.

    // Shift the coupling estimates into the Ising gauge.
    const auto J_ij_mean = mean(J_ij);
    const auto J_ij_mean_mean = mean(J_ij_mean);
    const auto J_ij_mean2 = mean( transpose(J_ij) );

    return J_ij - repmat(J_ij_mean) - repmat(J_ij_mean2,true) + J_ij_mean_mean;
}
*/
} // namespace superdca

#endif // PLMDCA_UTILITY_HPP

