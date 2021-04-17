#include "memory.h"

namespace EB
{
    namespace Windows
    {
        namespace Memory
        {
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
        }
    }
}
