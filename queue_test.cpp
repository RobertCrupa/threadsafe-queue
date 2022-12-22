#include "threadSafeQueue.cpp"

#include <gtest/gtest.h>
#include <future>

// The fixture for testing class threadSafeQueue.
class ThreadSafeQueueTest : public ::testing::Test {
 protected:
 
  // You can do set-up work for each test here.
  ThreadSafeQueueTest() 
  {

  }

  // You can do clean-up work that doesn't throw exceptions here.
  ~ThreadSafeQueueTest() override 
  {
     
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  // Code here will be called immediately after the constructor (right
  // before each test).
  void SetUp() override 
  {

  }
  
  // Code here will be called immediately after each test (right
  // before the destructor).
  void TearDown() override 
  {

  }

   // Class members declared here can be used by all tests in the test suite.
  
   ThreadSafeQueue<int> q;
   
};

// Tests that the threadSafeQueueTest consructor inialises an empty queue.
TEST_F(ThreadSafeQueueTest, is_initially_empty) 
{
	EXPECT_EQ(q.size(), 0) << "Wrong queue size on construction";
	EXPECT_EQ(q.empty(), true) << "Queue is not empty after contruction";
}

// Tests that the threadSafeQueueTest can emplace.
TEST_F(ThreadSafeQueueTest, can_emplace) 
{
	ASSERT_EQ(q.size(), 0);
	
	q.emplace(2);
	
	EXPECT_EQ(q.size(), 1) << "Emplacing did not change size of queue with";
	EXPECT_EQ(q.empty(), false) << "Queue was empty after emplace";
}

// Tests that the threadSafeQueueTest can push.
TEST_F(ThreadSafeQueueTest, can_push) 
{
	ASSERT_EQ(q.size(), 0);
	
	int num = 1;
	q.push(num);
	
	EXPECT_EQ(q.size(), 1) << "Pushing did not change size of queue";
	EXPECT_EQ(q.empty(), false) << "Queue was empty after Push";
}

// Tests that the threadSafeQueueTest can waitAndPop on single thread.
TEST_F(ThreadSafeQueueTest, can_waitAndPop_single_thread) 
{
	ASSERT_EQ(q.size(), 0);
	
	int num = 1;
	q.push(num);
	
	EXPECT_EQ(q.size(), 1);
	
	auto result = q.waitAndPop();
	
	ASSERT_EQ(*result.get(), 1);
	ASSERT_EQ(q.size(), 0);
}

// Tests that the threadSafeQueueTest can construct byRef.
TEST_F(ThreadSafeQueueTest, can_construct_byRef) 
{
	ASSERT_EQ(q.size(), 0);
	
	q.emplace(1);
	
	EXPECT_EQ(q.size(), 1);
	
	ThreadSafeQueue<int> q2( q );
	
	EXPECT_EQ(q2.size(), 1);
	
	auto result = q2.waitAndPop();
	
	ASSERT_EQ(*result.get(), 1);
	ASSERT_EQ(q2.size(), 0);
}

// Tests that the threadSafeQueueTest can waitAndPop byRef on single thread.
TEST_F(ThreadSafeQueueTest, can_waitAndPop_byRef_single_thread) 
{
	ASSERT_EQ(q.size(), 0);
	
	int num = 1;
	q.push(num);
	
	EXPECT_EQ(q.size(), 1);
	
	int result = 0;
	q.waitAndPop(result);
	
	ASSERT_EQ(result, 1);
	ASSERT_EQ(q.size(), 0);
}

// Tests that the threadSafeQueue can tryPop on single thread with data on queue.
TEST_F(ThreadSafeQueueTest, can_tryPop_true_single_thread) 
{
	ASSERT_EQ(q.size(), 0);
	
	int num = 1;
	q.push(num);
	
	EXPECT_EQ(q.size(), 1);
	
	auto result = q.tryPop();
	
	ASSERT_EQ(*result.get(), 1);
	ASSERT_EQ(q.size(), 0);
}

// Tests that the threadSafeQueue can tryPop on single thread with no data on queue.
TEST_F(ThreadSafeQueueTest, can_tryPop_false_single_thread) 
{
	ASSERT_EQ(q.size(), 0);
	
	auto result = q.tryPop();
	
	ASSERT_EQ(result.get(), nullptr);
	ASSERT_EQ(q.size(), 0);
}

// Tests that the threadSafeQueue can tryPop byRef on single thread with data on queue.
TEST_F(ThreadSafeQueueTest, can_tryPop_byref_true_single_thread) 
{
	ASSERT_EQ(q.size(), 0);
	
	int num = 1;
	q.push(num);
	
	EXPECT_EQ(q.size(), 1);
	
	int result = 0;
	ASSERT_EQ(q.tryPop(result), true);
	
	ASSERT_EQ(result, 1);
	ASSERT_EQ(q.size(), 0);
} 

// Tests that the threadSafeQueue can tryPop byRef on single thread with no data on queue.
TEST_F(ThreadSafeQueueTest, can_tryPop_byref_false_single_thread) 
{
	ASSERT_EQ(q.size(), 0);
	
	int result = 1;
	
	ASSERT_EQ(q.tryPop(result), false);
	
	ASSERT_EQ(result, 1);
	ASSERT_EQ(q.size(), 0);
} 

// Tests that the threadSafeQueueTest can wait and pop concurrently.
TEST_F(ThreadSafeQueueTest, can_waitAndPop_concurrently) 
{
   ThreadSafeQueue<int> q;
	
   std::promise<void> go;
   std::promise<void> push_ready;
   std::promise<void> pop_ready;
   
   std::shared_future<void> ready( go.get_future() );
   
   std::future<void>  push_done;
   std::future<std::shared_ptr<int>>   pop_done;

	ASSERT_EQ(q.size(), 0);
	
	// Start two asynchronous threads.
	push_done = std::async( std::launch::async,
							[&q, &push_ready, ready] () 
							{
								push_ready.set_value();
								ready.wait();
								q.emplace(36);
							} );
							
	pop_done = std::async( std::launch::async,
						   [&q, &pop_ready, ready] ()
						   {
								pop_ready.set_value();
								ready.wait();
								return q.waitAndPop();
						   } );
	
	// Wait for both threads to finish setting up. 					   
	push_ready.get_future().wait();
	pop_ready.get_future().wait();
	
	// Signal both threads to start simutanously.
	go.set_value();
	
	push_done.get();
	
	ASSERT_EQ(*pop_done.get().get(), 36);
	ASSERT_EQ(q.size(), 0);
}

// Tests that the threadSafeQueueTest can wait and pop byRef concurrently.
TEST_F(ThreadSafeQueueTest, can_waitAndPop_byRef_concurrently) 
{
   ThreadSafeQueue<int> q;
	
   std::promise<void> go;
   std::promise<void> push_ready;
   std::promise<void> pop_ready;
   
   std::shared_future<void> ready( go.get_future() );
   
   std::future<void>  push_done;
   std::future<int>   pop_done;

	ASSERT_EQ(q.size(), 0);
	
	// Start two asynchronous threads.
	push_done = std::async( std::launch::async,
							[&q, &push_ready, ready] () 
							{
								push_ready.set_value();
								ready.wait();
								q.emplace(36);
							} );
							
	pop_done = std::async( std::launch::async,
						   [&q, &pop_ready, ready] ()
						   {
								int num = 0;
								
								pop_ready.set_value();
								ready.wait();
								q.waitAndPop(num);
								
								return num;
						   } );
	
	// Wait for both threads to finish setting up. 					   
	push_ready.get_future().wait();
	pop_ready.get_future().wait();
	
	// Signal both threads to start simutanously.
	go.set_value();
	
	push_done.get();
	
	ASSERT_EQ(pop_done.get(), 36);
	ASSERT_EQ(q.size(), 0);
}

// Tests that the threadSafeQueueTest can try and pop byRef concurrently.
TEST_F(ThreadSafeQueueTest, can_tryPop_byRef_concurrently) 
{
   ThreadSafeQueue<int> q;
	
   std::promise<void> go;
   std::promise<void> push_ready;
   std::promise<void> pop_ready;
   
   std::shared_future<void> ready( go.get_future() );
   
   std::future<void>  push_done;
   std::future<int>   pop_done;

	ASSERT_EQ(q.size(), 0);
	
	// Start two asynchronous threads.
	push_done = std::async( std::launch::async,
							[&q, &push_ready, ready] () 
							{
								push_ready.set_value();
								ready.wait();
								q.emplace(36);
							} );
							
	pop_done = std::async( std::launch::async,
						   [&q, &pop_ready, ready] ()
						   {
								int num = 0;
								
								pop_ready.set_value();
								ready.wait();
								q.tryPop(num);								
								
								return num;
						   } );
	
	// Wait for both threads to finish setting up. 					   
	push_ready.get_future().wait();
	pop_ready.get_future().wait();
	
	// Signal both threads to start simutanously.
	go.set_value();
	
	push_done.get();
	int result = pop_done.get();
	
	if( q.size() == 0 )
	{
		ASSERT_EQ(result, 36);
	}
	else if( q.size() == 1 )
	{
		ASSERT_EQ(result, 0);
	}
}

// Tests that the threadSafeQueueTest can try and pop concurrently.
TEST_F(ThreadSafeQueueTest, can_tryPop_concurrently) 
{
   ThreadSafeQueue<int> q;
	
   std::promise<void> go;
   std::promise<void> push_ready;
   std::promise<void> pop_ready;
   
   std::shared_future<void> ready( go.get_future() );
   
   std::future<void>  				   push_done;
   std::future<std::shared_ptr<int>>   pop_done;

	ASSERT_EQ(q.size(), 0);
	
	// Start two asynchronous threads.
	push_done = std::async( std::launch::async,
							[&q, &push_ready, ready] () 
							{
								push_ready.set_value();
								ready.wait();
								q.emplace(36);
							} );
							
	pop_done = std::async( std::launch::async,
						   [&q, &pop_ready, ready] ()
						   {
								pop_ready.set_value();
								ready.wait();								
								return q.tryPop();
						   } );
	
	// Wait for both threads to finish setting up. 					   
	push_ready.get_future().wait();
	pop_ready.get_future().wait();
	
	// Signal both threads to start simutanously.
	go.set_value();
	
	push_done.get();
	auto result = pop_done.get();
	
	if( q.size() == 0 )
	{
		ASSERT_EQ(*result.get(), 36);
	}
	else
	{
		ASSERT_EQ(result, nullptr);
	}
}



int main(int argc, char **argv) 
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
