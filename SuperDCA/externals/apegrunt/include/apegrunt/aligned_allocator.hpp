/** @file aligned_allocator.hpp

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
#ifndef APEGRUNT_MEMORY_ALIGNED_ALLOCATOR_HPP
#define APEGRUNT_MEMORY_ALIGNED_ALLOCATOR_HPP

#include <memory>
#include <cstddef>
#include <cassert>

namespace apegrunt {

namespace memory {

enum class Alignment : size_t
{
    Normal = sizeof(void*),
    SSE    = 16,
    AVX    = 32,
    AVX512 = 64 // = cacheline alignment
};

namespace misc {
	inline bool is_power_of_two( size_t x ) { return ((x != 0) && !(x & (x - 1))); }
}

namespace detail {
    void* allocate_aligned_memory(size_t align, size_t size);
    void deallocate_aligned_memory(void* ptr) noexcept;
}
#ifdef __AVX512F__
template <typename T, std::size_t Align = std::size_t(Alignment::AVX512)>
#elif __AVX2__
template <typename T, std::size_t Align = std::size_t(Alignment::AVX)>
#elif __AVX__
template <typename T, std::size_t Align = std::size_t(Alignment::AVX)>
#else
template <typename T, std::size_t Align = std::size_t(Alignment::SSE)>
#endif
class AlignedAllocator;

template <std::size_t Align>
class AlignedAllocator<void, Align>
{
public:
    using pointer = void*;
    using const_pointer = const void*;
    using value_type = void;

    template <class U> struct rebind { using other = AlignedAllocator<U, Align>; };
};

template <typename T, std::size_t Align>
class AlignedAllocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = std::true_type;

    template <class U>
    struct rebind { using other = AlignedAllocator<U, Align>; };

public:
    AlignedAllocator() noexcept
    {}

    template <class U>
    AlignedAllocator(const AlignedAllocator<U, Align>&) noexcept
    {}

    size_type
    max_size() const noexcept
    { return (size_type(~0) - size_type(Align)) / sizeof(T); }

    pointer
    address(reference x) const noexcept
    { return std::addressof(x); }

    const_pointer
    address(const_reference x) const noexcept
    { return std::addressof(x); }

    pointer
    allocate(size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0)
    {
        const size_type alignment = static_cast<size_type>( Align );
        void* ptr = detail::allocate_aligned_memory(alignment , n * sizeof(T));
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }

        return reinterpret_cast<pointer>(ptr);
    }

    void
    deallocate(pointer p, size_type) noexcept
    { return detail::deallocate_aligned_memory(p); }

    template <class U, class ...Args>
    void
    construct(U* p, Args&&... args)
    { ::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...); }

    void
    destroy(pointer p)
    { p->~T(); }

	/// Helper class for memory management thru std::shared_ptr
	class deleter
	{
	public:
		deleter( std::size_t N ) : m_N(N) { }

		void operator()( pointer p )
		{
			allocator_t().deallocate( p, m_N );
		}
	private:
		using allocator_t = AlignedAllocator<T,Align>;
		const std::size_t m_N;
	};

};

template <typename T, std::size_t Align>
class AlignedAllocator<const T, Align>
{
public:
    using value_type = T;
    using pointer = const T*;
    using const_pointer = const T*;
    using reference = const T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = std::true_type;

    template <class U>
    struct rebind { using other = AlignedAllocator<U, Align>; };

public:
    AlignedAllocator() noexcept
    {}

    template <class U>
    AlignedAllocator(const AlignedAllocator<U, Align>&) noexcept
    {}

    size_type
    max_size() const noexcept
    { return (size_type(~0) - size_type(Align)) / sizeof(T); }

    const_pointer
    address(const_reference x) const noexcept
    { return std::addressof(x); }


    pointer
    allocate(size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0)
    {
        const size_type alignment = static_cast<size_type>( Align );
        void* ptr = detail::allocate_aligned_memory(alignment , n * sizeof(T));
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }

        return reinterpret_cast<pointer>(ptr);
    }

    void
    deallocate(pointer p, size_type) noexcept
    { return detail::deallocate_aligned_memory(p); }

    template <class U, class ...Args>
    void
    construct(U* p, Args&&... args)
    { ::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...); }

    void
    destroy(pointer p)
    { p->~T(); }

	/// Helper class for memory management thru std::shared_ptr
	class deleter
	{
	public:
		deleter( std::size_t N ) : m_N(N) { }

		void operator()( pointer p )
		{
			allocator_t().deallocate( p, m_N );
		}
	private:
		using allocator_t = AlignedAllocator<T,Align>;
		const std::size_t m_N;
	};

};

template <typename T, std::size_t TAlign, typename U, std::size_t UAlign>
inline
bool
operator== (const AlignedAllocator<T,TAlign>&, const AlignedAllocator<U, UAlign>&) noexcept
{ return TAlign == UAlign; }

template <typename T, std::size_t TAlign, typename U, std::size_t UAlign>
inline
bool
operator!= (const AlignedAllocator<T,TAlign>&, const AlignedAllocator<U, UAlign>&) noexcept
{ return TAlign != UAlign; }
/*
void* detail::allocate_aligned_memory(size_t align, size_t size);

void detail::deallocate_aligned_memory(void *ptr) noexcept;
*/
} // namespace memory

} // namespace apegrunt

#endif // APEGRUNT_MEMORY_ALIGNED_ALLOCATOR_HPP
