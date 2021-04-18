#pragma once

#include<Windows.h>
#include<vector>
#include<array>
#include<memory>

namespace EB
{
    namespace Windows
    {
        namespace Memory
        {
            static size_t _memory_chunk_size = 0x00100000; // 1024kb

            void set_memory_chunk_size(size_t const& size);

            bool write_byte(HANDLE const& handle, uintptr_t const& addr, BYTE const& byte);
            bool write(HANDLE const& handle, uintptr_t const& addr, std::vector<BYTE> const& bytes);

            BYTE read_byte(HANDLE const& handle, uintptr_t const& addr);
            bool read(HANDLE const& handle, BYTE* buffer, uintptr_t const& addr, size_t const& size);

            void fill_with_nop(HANDLE const& handle, uintptr_t const& addr, size_t const& size);
            bool search_bytes(HANDLE const& handle, uintptr_t const& start_addr, uintptr_t const& end_addr, std::vector<BYTE> const& bytes, uintptr_t& addr_out, size_t const& nth=0);
        }
    }
}
