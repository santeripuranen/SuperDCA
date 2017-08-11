/** @file Threshold_rule.hpp
 
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
#ifndef APEGRUNT_THRESHOLD_RULE_HPP
#define APEGRUNT_THRESHOLD_RULE_HPP

#include <string>
#include <functional> // for std::hash and comparison functions
#include <memory> // for std::unique_ptr and std::make_unique
#include <sstream>
#include <iomanip> // std::setfill, std::setw

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

namespace apegrunt {

// A threshold functor class
template< typename ComparableT >
class Threshold_rule
{
public:
	Threshold_rule() { }
    ~Threshold_rule() { }

    Threshold_rule( std::string rule ) : m_threshold(1), m_comparator(std::greater<ComparableT>())
	{
		m_op_safe_string.assign("gt");
		m_op_symbol.assign(">");
		this->m_update_strings();
	}

    bool operator()( ComparableT comp ) const { return m_comparator(comp,m_threshold); }

    void set_rule( std::string rule_string )
    {
    	m_op_safe_string.assign("gt");
    	m_op_symbol.assign(">");

    	m_safe_string.clear();
    	m_symbol_string.clear();

    	this->m_update_strings();
    }

    const std::string& safe_string() const
    {
    	return m_safe_string;
    }

    operator std::string() const
    {
    	return std::string(m_symbol_string);
    }

    const std::string& str() const { return m_symbol_string; }

private:
    ComparableT m_threshold;
    std::function< bool(ComparableT,ComparableT) > m_comparator;
    std::string m_safe_string;
    std::string m_symbol_string;
    std::string m_op_safe_string;
    std::string m_op_symbol;

    void m_update_strings()
    {
    	if( m_safe_string.empty() )
    	{
    		std::ostringstream os;
    		os << m_op_safe_string << std::setw(2) << std::setfill('0') << m_threshold;
    		m_safe_string.assign(os.str());
    	}
    	if( m_symbol_string.empty() )
    	{
    		std::ostringstream os;
    		os << m_op_symbol << std::setprecision(3) << m_threshold;
    		m_symbol_string.assign( os.str() );
    	}
    }
};

template< typename ComparableT >
std::ostream& operator<< ( std::ostream& os, const Threshold_rule<ComparableT>& rule )
{
	os << std::string(rule);
	return os;
}

} // namespace apegrunt

#endif // APEGRUNT_THRESHOLD_RULE_HPP
