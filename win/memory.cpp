#include "memory.h"

namespace EB
{
    namespace Windows
    {
        namespace Memory
        {
            void set_memory_chunk_size(size_t const& size)
            {
                _memory_chunk_size = size;
            }
            
            bool write_byte(HANDLE const& handle, DWORD const& addr, BYTE const& byte)
            {
                return WriteProcessMemory(handle, (LPVOID)addr, &byte, 1, NULL);
            }

            bool write(HANDLE const& handle, DWORD const& addr, std::vector<BYTE> const& bytes)
            {
                return WriteProcessMemory(handle, (LPVOID)addr, bytes.data(), bytes.size(), NULL);
            }

            BYTE read_byte(HANDLE const& handle, DWORD const& addr)
            {
                BYTE byte;
                return ReadProcessMemory(handle, (LPVOID)addr, &byte, 1, NULL) ? byte : 0x00;
            }

            bool read(HANDLE const& handle, BYTE* buffer, DWORD const& addr, size_t const& size)
            {
                return ReadProcessMemory(handle, (LPVOID)addr, buffer, size, NULL);
            }

            void fill_with_nop(HANDLE const& handle, DWORD const& addr, size_t const& size)
            {
                for(int i=0; i < size; i++)
                    write_byte(handle, addr + i, 0x90);
            }

            bool search_bytes(HANDLE const& handle, DWORD const& start_addr, DWORD const& end_addr, std::vector<BYTE> const& bytes, DWORD& addr_out, size_t const& nth)
            {
                DWORD current_addr = start_addr;

                size_t chunk_size  = _memory_chunk_size;
                BYTE* memory_chunk = new BYTE[chunk_size];
                ZeroMemory(memory_chunk, chunk_size);

                size_t found = 0;

                while(current_addr >= start_addr
                &&    current_addr <= end_addr)
                {
                    if(!ReadProcessMemory(handle, (LPVOID)current_addr, memory_chunk, chunk_size, NULL))
                    {
                        delete[] memory_chunk;
                        return false;
                    }

                    // Check bytes
                    for(size_t i=0; i <= chunk_size - sizeof(BYTE); i++)
                    {
                        size_t match_count = 0;

                        if(bytes[0] == memory_chunk[i])
                        {
                            match_count += 1;

                            for(size_t j=1; j < bytes.size(); j++)
                            {
                                if(bytes[j] == memory_chunk[i+j])
                                    match_count += 1;
                                else break;
                            }

                            if(match_count == bytes.size()
                            && ++found == (nth+1))
                            {
                                addr_out = current_addr + i;

                                delete[] memory_chunk;
                                return true;
                            }
                        }
                    }

                    // Go to next chunk
                    if(current_addr + chunk_size > end_addr) chunk_size = end_addr - current_addr;
                    current_addr += chunk_size;
                }

                delete[] memory_chunk;
                return false;
            }
        }
    }
}
