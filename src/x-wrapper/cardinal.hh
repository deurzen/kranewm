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

        cardinal_list_t(unsigned long* card_list, size_t list_len)
            : len(list_len)
        {
            for (size_t i = 0; i < list_len; ++i)
                val.push_back(card_list[i]);
        }

        cardinal_list_t(void* raw_data, unsigned long data_len)
            : len(data_len)
        {
            unsigned long* card_list = (unsigned long*) raw_data;
            for (size_t i = 0; i < data_len; ++i)
                val.push_back(card_list[i]);
        }

        operator ::std::vector<unsigned long>() const { return val; }
        operator unsigned long*() { return val.data(); }
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

        inline ::std::vector<unsigned long> get() { return val; }
        inline unsigned long* get_ptr() { return val.data(); }

    private:
        ::std::vector<unsigned long> val;
        size_t len;

    };
}

#endif//__KRANEWM__X_WRAPPER__CARDINAL__GUARD__
