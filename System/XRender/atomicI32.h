#ifndef __ATOMIC_I32_STORM_H_
#define __ATOMIC_I32_STORM_H_

///Атомарный I32 тип
class atomicI32
{

        volatile long m_val;

public:

        static const long False = 0;
        static const long True = 1;

        atomicI32() : m_val (0)
        {
        }

        atomicI32(long v) : m_val (v)
        {
        }

        ~atomicI32()
        {
        }

        operator long() const
        {
                return m_val;
        }

        bool operator! () const
        {
                return (m_val == 0);
        }

        bool operator== ( long rhs ) const
        {
                return ( m_val == rhs );
        }

        bool operator!= ( long rhs ) const
        {
                return ( m_val != rhs );
        }

        long operator++ ()
        {
                return InterlockedIncrement((long *)&m_val);
        }

        long operator++ (int)
        {
                return operator++() - 1;
        }

        long operator-- ()
        {
                return InterlockedDecrement((long *)&m_val);
        }

        long operator-- (int)
        {
                return operator--() + 1;
        }


        ///Сравнивет значение в памяти со значением conditionValue, если результат сравнения == true
        ///Значение в памяти изменяется на newValue и функция возвращает true
        ///Иначе ничего не происходит и функция возвращает false
        bool CompareAndSwap( long conditionValue, long newValue )
        {
                return (InterlockedCompareExchange( (long *)&m_val, (long)newValue, (long)conditionValue ) == conditionValue);
        }


        long operator= ( long newValue )        
        {
                InterlockedExchange((long *)&m_val, newValue);
                return m_val;
        }

        void operator+= ( long add )
        {
                InterlockedExchangeAdd( (long *)&m_val, (long)add );
        }

        void operator-= ( long subtract )
        {
                operator+=( -subtract );
        }

        void operator*= ( long multiplier )
        { 
                long original, result; 
                do 
                { 
                        original = m_val; 
                        result = original * multiplier; 
                } while (CompareAndSwap( original, result ) == false);
        }

        void operator/= ( long divisor )
        { 
                long original, result; 
                do 
                { 
                        original = m_val; 
                        result = original / divisor;
                } while (CompareAndSwap( original, result ) == false);
        }

        long operator+ ( long rhs ) const
        {
                return m_val + rhs;
        }

        long operator- ( long rhs ) const
        {
                return m_val - rhs;
        }
};

#endif