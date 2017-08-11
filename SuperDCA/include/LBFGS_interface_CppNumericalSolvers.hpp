/** @file LBFGS_interface_CppNumericalSolvers.hpp

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

#ifndef SUPERDCA_LBFGS_INTERFACE_CPPNUMERICALSOLVERS_HPP
#define SUPERDCA_LBFGS_INTERFACE_CPPNUMERICALSOLVERS_HPP

#include <cmath> // for isfinite
#include <cstring> // for std::memcpy

#ifndef SUPERDCA_NO_CPPNUMERICALSOLVERS

#ifndef NDEBUG
#define NDEBUG
#define UNDEFINE_NDEBUG
#endif

#include "cppoptlib/problem.h"
#include "cppoptlib/solver/lbfgssolver.h"

#ifdef UNDEFINE_NDEBUG
#undef NDEBUG
#undef UNDEFINE_NDEBUG
#endif

#ifndef SUPERDCA_NO_EIGEN
#include "Eigen/Core"
#else
#pragma error("CppNumericalSolvers cannot be built without Eigen")
#endif

#endif // #ifndef SUPERDCA_NO_CPPNUMERICALSOLVERS

#include "apegrunt/aligned_allocator.hpp"

#include "plmDCA_options.h"
#include "plmDCA_optimizer_parameters.hpp"

#include "Array_view.hpp"

/** A function object that implements the cost function
	interface used by CppNumbericalSolvers.
*/
template< typename ParametersT >
class plmDCA_cpu_objective_for_CppNumericalSolvers: public cppoptlib::Problem<typename ParametersT::real_t>
{
public:
	using real_t = typename ParametersT::real_t;
	using base_t = cppoptlib::Problem<real_t>;

	~plmDCA_cpu_objective_for_CppNumericalSolvers() { }

	plmDCA_cpu_objective_for_CppNumericalSolvers( ParametersT& optimizer_parameters )
	: m_nfeval(0),
	  m_parameters( optimizer_parameters ),
	  m_gradient(optimizer_parameters.get_dimensions(),real_t(0.0)) //, m_gradient_ptr(m_gradient.data())
	{
	}

	plmDCA_cpu_objective_for_CppNumericalSolvers( plmDCA_cpu_objective_for_CppNumericalSolvers<ParametersT>& other )
	: m_nfeval(0),
	  m_parameters( other.m_parameters ),
	  m_gradient(other.m_gradient.size(),real_t(0.0))
	//, m_gradient_ptr(m_gradient.data())
	{
	}

	real_t value( const cppoptlib::Vector<real_t> &x )
	{
		m_parameters.set_solution( x.data() ); // set parameter estimate ptr
 		m_parameters.set_gradient( m_gradient.data(), true ); // set gradient ptr and set all values to zero
 		superdca::plmDCA_objective_fval_and_gradient<ParametersT,double>( m_parameters );

		++m_nfeval;
		return m_parameters.get_fvalue();
	}

	real_t value_no_set_gradient( const cppoptlib::Vector<real_t> &x )
	{
		m_parameters.set_solution( x.data() ); // set parameter estimate ptr
 		superdca::plmDCA_objective_fval_and_gradient<ParametersT,double>( m_parameters );

		++m_nfeval;
		return m_parameters.get_fvalue();
	}
	//> Gradient function (overrides the default finite difference implementation)
	void gradient( const cppoptlib::Vector<real_t> &x, cppoptlib::Vector<real_t> &grad ) override
	{
		this->value( x );
		grad = Eigen::Map<Eigen::VectorXd,Eigen::Aligned>( m_gradient.data(), m_gradient.size() );
	}

	// override virtual base
	real_t value_and_gradient( const cppoptlib::Vector<real_t> &x, cppoptlib::Vector<real_t> &grad ) override
	{
 		m_parameters.set_gradient( grad.data(), true ); // set gradient ptr and set all values to zero
		this->value_no_set_gradient(x);
		//this->gradient( x, grad );
		return m_parameters.get_fvalue();
	}

	std::size_t get_nfeval() const { return m_nfeval; }
	void reset_counters() { m_nfeval=0; }

private:
	using allocator_t = typename apegrunt::memory::AlignedAllocator<real_t>;

	std::size_t m_nfeval;
	ParametersT& m_parameters;
	std::vector<real_t,allocator_t> m_gradient;
	real_t *m_gradient_ptr;
};

#endif // SUPERDCA_LBFGS_INTERFACE_CPPNUMERICALSOLVERS_HPP

