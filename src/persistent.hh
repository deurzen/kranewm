#ifndef __KRANEWM_PERSISTENT_GUARD__
#define __KRANEWM_PERSISTENT_GUARD__

// fwd decl
typedef class context_t* context_ptr_t;

class statehandler_t
{
public:
    explicit statehandler_t() = default;

    void serialize(context_ptr_t) const;
    void deserialize() const;

};


#endif//__KRANEWM_PERSISTENT_GUARD__
