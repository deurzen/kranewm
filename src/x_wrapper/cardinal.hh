#ifndef __KRANEWM__X_WRAPPER__CARDINAL__GUARD__
#define __KRANEWM__X_WRAPPER__CARDINAL__GUARD__

#include "common.hh"
#include "type.hh"

extern "C" {
#include <X11/Xmd.h>
#include <X11/Xatom.h>
}

#include <vector>
#include <cstring>


namespace x_wrapper
{
    class cardinal_t : public x_type
    {
    public:
        cardinal_t() = default;

        cardinal_t(CARD32 x)
            : val(x)
        {}

        cardinal_t(void* raw_data, unsigned long _)
            : val(*(CARD32*)raw_data)
        {}

        operator CARD32() const { return val; }
        operator bool()   const { return val != 0; }

        inline bool operator==(const cardinal_t& card) const
        {
            return card.val == val;
        }

        inline bool operator==(const CARD32& card) const
        {
            return card == val;
        }

        inline int  length() const { return 1; }
        inline Atom type()   const { return XA_CARDINAL; }
        inline int  size()   const { return 32; }

        inline CARD32 get() const { return val; }
        inline const CARD32* get_ptr() const { return &val; }

    private:
        CARD32 val;

    };


    class cardinal_list_t : public x_type
    {
    public:
        cardinal_list_t() = default;

        cardinal_list_t(CARD32* card_list, size_t list_length)
            : len(list_length)
        {
            ::std::memcpy(val, card_list, sizeof(CARD32) * list_length);
        }

        cardinal_list_t(void* raw_data, unsigned long data_len)
            : len(data_len)
        {
            ::std::memcpy(val, (CARD32*)raw_data, sizeof(CARD32) * data_len);
        }

        operator ::std::vector<CARD32>() const { return ::std::vector<CARD32>(val, val + len); }
        operator CARD32*() const { return val; }
        operator bool() const { return len != 0; }

        inline bool operator==(const cardinal_list_t& card_list) const
        {
            if (card_list.len != len)
                return false;

            bool different = false;
            for (size_t i = 0; i < len; ++i)
                if (val[i] != card_list.val[i])
                    different = true;

            return !different;
        }

        inline int  length() const { return len; }
        inline Atom type() const { return XA_CARDINAL; }
        inline int  size() const { return 32; }

        inline ::std::vector<CARD32> get() { return ::std::vector<CARD32>(val, val + len); }
        inline CARD32* get_ptr() { return val; }

    private:
        CARD32* val;
        size_t len;

    };
}

#endif//__KRANEWM__X_WRAPPER__CARDINAL__GUARD__
