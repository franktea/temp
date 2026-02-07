
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONTEXT_DETAIL_FCONTEXT_H
#define BOOST_CONTEXT_DETAIL_FCONTEXT_H

#include <cstdio>

typedef void*   fcontext_t;

struct transfer_t {
    fcontext_t  fctx;
    void    *   data;
};

extern "C" 
transfer_t jump_fcontext( fcontext_t const to, void * vp);
extern "C" 
fcontext_t make_fcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );

#endif // BOOST_CONTEXT_DETAIL_FCONTEXT_H

