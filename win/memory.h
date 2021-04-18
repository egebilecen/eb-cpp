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

            bool write_byte(HANDLE const& handle, DWORD const& addr, BYTE const& byte);
            bool write(HANDLE const& handle, DWORD const& addr, std::vector<BYTE> const& bytes);

            BYTE read_byte(HANDLE const& handle, DWORD const& addr);
            bool read(HANDLE const& handle, BYTE* buffer, DWORD const& addr, size_t const& size);

            void fill_with_nop(HANDLE const& handle, DWORD const& addr, size_t const& size);
            bool search_bytes(HANDLE const& handle, DWORD const& start_addr, DWORD const& end_addr, std::vector<BYTE> const& bytes, DWORD& addr_out, size_t const& nth=0);
        }
    }
}
