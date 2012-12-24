
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_FIBERS_SOURCE

#include <boost/fiber/detail/fiber_base.hpp>

#include <boost/foreach.hpp>

#include <boost/fiber/detail/scheduler.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

void
fiber_base::notify_()
{
    BOOST_ASSERT( is_complete() );
    BOOST_ASSERT( ! is_resumed() );

    BOOST_FOREACH( fiber_base::ptr_t & p, joining_)
    { boost::fibers::detail::scheduler::instance().notify( p); }
    joining_.clear();
}

fiber_base::fiber_base( context::fcontext_t * callee, bool unwind, bool preserve_fpu) :
    use_count_( 0),
    caller_(),
    callee_( callee),
    priority_( 0),
    flags_( 0),
    except_(),
    joining_()
{
    if ( unwind) flags_ |= flag_force_unwind;
    if ( preserve_fpu) flags_ |= flag_preserve_fpu;
}

void
fiber_base::resume()
{
    BOOST_ASSERT( ! is_complete() );
    BOOST_ASSERT( ! is_resumed() );

    flags_ |= flag_resumed;
    context::jump_fcontext( & caller_, callee_, 0, preserve_fpu() );

    if ( is_complete() ) notify_();

    BOOST_ASSERT( ! is_resumed() );
}

void
fiber_base::suspend()
{
    BOOST_ASSERT( ! is_complete() );
    BOOST_ASSERT( is_resumed() );

    flags_ &= ~flag_resumed;
    context::jump_fcontext( callee_, & caller_, 0, preserve_fpu() );

    BOOST_ASSERT( is_resumed() );

    if ( 0 != ( flags_ & flag_unwind_stack) )
        throw forced_unwind();
}

void
fiber_base::terminate()
{
    BOOST_ASSERT( ! is_resumed() );

    if ( ! is_complete() )
    {
        flags_ |= flag_canceled;
        if ( ! is_complete() && force_unwind() )
            unwind_stack();
    }

    notify_();

    BOOST_ASSERT( is_complete() );
    BOOST_ASSERT( ! is_resumed() );
    BOOST_ASSERT( joining_.empty() );
}

void
fiber_base::join( ptr_t const& p)
{
    BOOST_ASSERT( ! p->is_complete() );
    BOOST_ASSERT( p->is_resumed() );

    joining_.push_back( p);
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif