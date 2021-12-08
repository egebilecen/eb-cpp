#pragma once

#include <Windows.h>
#include <vector>
#include <array>
#include <memory>

namespace EB
{
    namespace Windows
    {
        namespace Memory
        {
            static size_t _memory_chunk_size = 0x00100000; // 1024kb

                                                           // External Functions
            void set_memory_chunk_size(size_t const& size);

            void write_byte(HANDLE const& handle, uintptr_t const& addr, BYTE const& byte);
            void write(HANDLE const& handle, uintptr_t const& addr, std::vector<BYTE> const& bytes);
            void write(HANDLE const& handle, uintptr_t const& addr, BYTE* bytes, size_t const& size);

            BYTE read_byte(HANDLE const& handle, uintptr_t const& addr);
            void read(HANDLE const& handle, BYTE* buffer, uintptr_t const& addr, size_t const& size);

            void fill_with_nop(HANDLE const& handle, uintptr_t const& addr, size_t const& size);
            bool search_bytes(HANDLE const& handle, uintptr_t const& start_addr, uintptr_t const& end_addr, std::vector<BYTE> const& bytes, uintptr_t& addr_out, size_t const& nth=0);

            // Internal Functions
            void write_byte(uintptr_t const& addr, BYTE const& byte);
            void write(uintptr_t const& addr, std::vector<BYTE> const& bytes);
            void write(uintptr_t const& addr, BYTE* bytes, size_t const& size);

            BYTE read_byte(uintptr_t const& addr);
            void read(BYTE* buffer, uintptr_t const& addr, size_t const& size);

            void fill_with_nop(uintptr_t const& addr, size_t const& size);
            bool search_bytes(uintptr_t const& start_addr, uintptr_t const& end_addr, std::vector<BYTE> const& bytes, uintptr_t& addr_out, size_t const& nth=0);

        #ifdef _WIN64
            uintptr_t get_function_real_address(void* func);
            size_t get_function_size(void* func);
        #endif
        }
    }
}
