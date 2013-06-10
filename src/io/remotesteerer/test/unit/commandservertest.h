#include <cxxtest/TestSuite.h>
#include <libgeodecomp/io/remotesteerer/commandserver.h>

using namespace LibGeoDecomp;
using namespace LibGeoDecomp::RemoteSteererHelpers;

namespace LibGeoDecomp {

namespace RemoteSteererHelpers {

class CommandServerTest : public CxxTest::TestSuite
{
public:
    class MockAction : public Action<int>
    {
    public:
        MockAction() :
            RemoteSteererHelpers::Action<int>("mock", "this is but a dummy action")
        {}

        virtual void operator()(const StringVec& parameters, Pipe& pipe)
        {
            pipe.addSteeringFeedback("MockAction mocks you!");
        }
    };

    void setUp()
    {
        pipe.reset(new Pipe);
    }

    void tearDown()
    {
        pipe.reset();
    }

    void testActionInvocationAndFeedback()
    {
#ifdef LIBGEODECOMP_FEATURE_THREADS
        {
            CommandServer<int> server(47110, pipe);
            server.addAction(new MockAction());
            CommandServer<int>::sendCommand("mock 1 2 3", 47110);
        }
        StringVec feedback = pipe->retrieveSteeringFeedback();
        TS_ASSERT_EQUALS(feedback.size(), 1);
        TS_ASSERT_EQUALS(feedback[0], "MockAction mocks you!");
#endif
    }

    void testInvalidCommand()
    {
#ifdef LIBGEODECOMP_FEATURE_THREADS
        CommandServer<int> server(47110, pipe);
        StringVec feedback = CommandServer<int>::sendCommandWithFeedback("blah", 1, 47110);

        TS_ASSERT_EQUALS(feedback.size(), 1);
        TS_ASSERT_EQUALS(feedback[0], "command not found: blah\n");
#endif
    }

private:
    boost::shared_ptr<Pipe> pipe;
};

}
}
