/** @file aligned_allocator.cpp

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

#include "apegrunt/aligned_allocator.hpp"

namespace apegrunt {

namespace memory {

namespace detail {

void* allocate_aligned_memory( std::size_t align, std::size_t size )
{
    assert(align >= sizeof(void*));
    assert(misc::is_power_of_two(align));

    if (size == 0) {
        return nullptr;
    }

    void* ptr = nullptr;
    int rc = posix_memalign(&ptr, align, size);

    if (rc != 0) {
        return nullptr;
    }

    return ptr;
}

void deallocate_aligned_memory(void *ptr) noexcept
{
    return free(ptr);
}

} // namespace detail

} // namespace memory

} // namespace apegrunt
