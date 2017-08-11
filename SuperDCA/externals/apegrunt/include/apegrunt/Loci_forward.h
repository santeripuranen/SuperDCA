/** @file Loci_forward.h

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

#ifndef APEGRUNT_LOCI_FORWARD_H
#define APEGRUNT_LOCI_FORWARD_H

#include <memory> // std::shared_ptr, std::make_shared

namespace apegrunt {

// forward declarations
class Loci;

using Loci_ptr = std::shared_ptr< Loci >;

template< typename LociT, typename... Args >
Loci_ptr make_Loci_ptr( Args&&... args )
{
	return std::make_shared< LociT >( std::move(args...) );
	//return Loci_ptr( new LociT( std::move( args... ) ) );
}

template< typename LociT >
Loci_ptr make_Loci_ptr( const LociT& loci )
{
	return std::make_shared< LociT >( loci );
}

} // namespace apegrunt

#endif // APEGRUNT_LOCI_FORWARD_H

