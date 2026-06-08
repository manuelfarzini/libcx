/** @file libcx/ite/iterator.hh **/

#ifndef CX_UTI_ITERATOR_HH
#define CX_UTI_ITERATOR_HH

#include "libcx/uti/members.hh"

namespace cx {
inline namespace uti {

template<typename It>
struct RevIterator {
    using Iter = It;
    Iter cur;

    cons RevIterator() = default;
    cons RevIterator(Iter it) : cur(it) {}

    cons Iter base(this auto const& self) noexce { return self.cur; }

    cons fn operator*(this auto const& self) noexce->declt(auto)
    {
        auto tmp = self.cur;
        --tmp;
        return *tmp;
    }

    cons fn operator->(this auto const& self) noexce->auto
    {
        return &(*self);  // XXX: proxy?
    }

    cons fn operator++(this auto& self) noexce->declt(self)
    {
        --self.cur;
        return self;
    }

    cons fn operator++(this auto& self, int) noexce->auto
    {
        auto tmp = self;
        --self.cur;
        return tmp;
    }

    cons fn operator--(this auto& self) noexce->declt(self)
    {
        ++self.cur;
        return self;
    }

    cons fn operator--(this auto& self, int) noexce->auto
    {
        auto tmp = self;
        ++self.cur;
        return tmp;
    }

    friend cons fn operator==(RevIterator const& a, RevIterator const& b) noexce->b32
    {
        return a.cur == b.cur;
    }

    friend cons fn operator!=(RevIterator const& a, RevIterator const& b) noexce->b32
    {
        return !(a == b);
    }
};

template<typename It> cons auto rev_iterator(It it) noexce -> RevIterator<It>
{
    return RevIterator<It>(it);
}

// FIX:
inln cons fn beg(auto& c) noexce -> declt(auto)
{
    return iter_in(c)(c.begin());
}
inln cons fn beg(auto const& c) noexce -> declt(auto)
{
    return cter_in(c)(c.begin());
}
inln cons fn rbeg(auto& c) noexce -> declt(auto)
{
    return rter_in(c)(c.rbegin());
}
inln cons fn rbeg(auto const& c) noexce -> declt(auto)
{
    return crter_in(c)(c.rbegin());
}

inln cons fn end(auto& c) noexce -> declt(auto)
{
    return iter_in(c)(c.end());
}
inln cons fn end(auto const& c) noexce -> declt(auto)
{
    return cter_in(c)(c.end());
}
inln cons fn rend(auto& c) noexce -> declt(auto)
{
    return rter_in(c)(c.rend());
}
inln cons fn rend(auto const& c) noexce -> declt(auto)
{
    return crter_in(c)(c.rend());
}

}       // namespace uti
}       // namespace cx
#endif  // CX_ITE_ITERATOR_HH
