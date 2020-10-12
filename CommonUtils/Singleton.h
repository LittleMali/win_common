#pragma once

#include <memory>

namespace LittleUtils 
{

    template <class Type>
    class CSingleton
    {
    protected:
        CSingleton() { }
        virtual ~CSingleton() { }
        
    public:

#ifndef _WIN64
        static Type* Instance()
        {
            if (m_pInstance != 0)
            {
                return reinterpret_cast<Type*>(m_pInstance);
            }

            static Type sInst;
            while (InterlockedCompareExchange((volatile LONG*)&m_pInstance, (LONG)&sInst, 0) == 0)
            {
                ;
            }

            return reinterpret_cast<Type*>(m_pInstance);
        }
#else
        static Type* Instance()
        {
            if (m_pInstance != 0)
            {
                return reinterpret_cast<Type*>(m_pInstance);
            }

            static Type sInst;
            while (InterlockedCompareExchange64((volatile LONG*)&m_pInstance, (LONG)&sInst, 0) == 0)
            {
                ;
            }

            return reinterpret_cast<Type*>(m_pInstance);
        }
#endif // !_WIN64

    private:
        static volatile ULONG_PTR m_pInstance;
    };

    template <class Type>
    volatile ULONG_PTR CSingleton<Type>::m_pInstance = 0;
}