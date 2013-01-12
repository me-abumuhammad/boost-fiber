#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/assert.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/optional.hpp>

#include <boost/fiber/all.hpp>

typedef boost::fibers::unbounded_channel< std::string >	fifo_t;

inline
void ping( fifo_t & recv_buf, fifo_t & send_buf)
{
    boost::fibers::fiber::id id( boost::this_fiber::get_id() );

	boost::optional< std::string > value;

	send_buf.put("ping");
	BOOST_ASSERT( recv_buf.take( value) );
	std::cout << "statum " <<  id << ": ping received: " << * value << std::endl;
	value.reset();

	send_buf.put("ping");
	BOOST_ASSERT( recv_buf.take( value) );
	std::cout << "statum " <<  id << ": ping received: " << * value << std::endl;
	value.reset();

	send_buf.put("ping");
	BOOST_ASSERT( recv_buf.take( value) );
	std::cout << "statum " <<  id << ": ping received: " << * value << std::endl;
	value.reset();

	send_buf.deactivate();
}

inline
void pong( fifo_t & recv_buf, fifo_t & send_buf)
{
    boost::fibers::fiber::id id( boost::this_fiber::get_id() );

	boost::optional< std::string > value;

	BOOST_ASSERT( recv_buf.take( value) );
	std::cout << "statum " <<  id << ": pong received: " << * value << std::endl;
	value.reset();
	send_buf.put("pong");

	BOOST_ASSERT( recv_buf.take( value) );
	std::cout << "statum " <<  id << ": pong received: " << * value << std::endl;
	value.reset();
	send_buf.put("pong");

	BOOST_ASSERT( recv_buf.take( value) );
	std::cout << "statum " <<  id << ": pong received: " << * value << std::endl;
	value.reset();
	send_buf.put("pong");

	send_buf.deactivate();
}

void f()
{
	fifo_t buf1, buf2;
	
	boost::fibers::fiber s1(
            boost::bind(
                & ping, boost::ref( buf1), boost::ref( buf2) ) );
	boost::fibers::fiber s2(
            boost::bind(
                & pong, boost::ref( buf2), boost::ref( buf1) ) );

    s1.join();
    s2.join();

    std::cout << "both channels deactivated" << std::endl;
}

int main()
{
    boost::fibers::round_robin ds;
    boost::fibers::scheduling_algorithm( & ds);

	try
	{
		boost::fibers::fiber s( f);

        while ( s) boost::fibers::run();

		std::cout << "done." << std::endl;

		return EXIT_SUCCESS;
	}
	catch ( std::exception const& e)
	{ std::cerr << "exception: " << e.what() << std::endl; }
	catch (...)
	{ std::cerr << "unhandled exception" << std::endl; }
	return EXIT_FAILURE;
}
