/** @file SuperDCA_version.h

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
#ifndef SUPERDCA_VERSION_H
#define SUPERDCA_VERSION_H

namespace superdca {

struct SuperDCA_version
{
	static const int s_MajorVersion = 0; // substantial rewrite
	static const int s_MinorVersion = 2; // feature change
	static const int s_SubminorVersion = 0; // bugfix, small enhancement
};

} // namespace superdca

#endif // SUPERDCA_VERSION_H
