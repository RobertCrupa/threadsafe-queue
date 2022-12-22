#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template<typename T>
class ThreadSafeQueue
{
public:
  ThreadSafeQueue() = default;

  ThreadSafeQueue( const ThreadSafeQueue& t_queue ) 
  {
    std::lock_guard<std::mutex> lock( t_queue.m_queue_mutex );
    m_data_queue = t_queue.m_data_queue;
  }
  
  ThreadSafeQueue& operator=( const ThreadSafeQueue& ) = delete;

  void push( const T& data )
  {
    std::lock_guard<std::mutex> lock( m_queue_mutex );
    m_data_queue.push(data);
    m_cond_var.notify_one();
  }

  void emplace( const T&& data )
  {
    std::lock_guard<std::mutex> lock( m_queue_mutex );
    m_data_queue.emplace(data);
    m_cond_var.notify_one();
  }

  void waitAndPop( T& value )
  {
    std::unique_lock<std::mutex> lock( m_queue_mutex );
    m_cond_var.wait( lock, [this] () { return !m_data_queue.empty(); } );
    value = m_data_queue.front();
    m_data_queue.pop();
  }

  std::shared_ptr<T> waitAndPop()
  {
    std::unique_lock<std::mutex> lock( m_queue_mutex );
    m_cond_var.wait( lock, [this] () { return !m_data_queue.empty(); } );
    std::shared_ptr<T> data( std::make_shared<T>( m_data_queue.front() ) );
    m_data_queue.pop();
    return data;
  }

  bool tryPop( T& value )
  {
    std::lock_guard<std::mutex> lock( m_queue_mutex );
    if( m_data_queue.empty() )
    {
      return false;
    }
    value = m_data_queue.front();
    m_data_queue.pop();
    return true;
  }

  std::shared_ptr<T> tryPop()
  {
    std::lock_guard<std::mutex> lock( m_queue_mutex );
    if( m_data_queue.empty() )
    {
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> data( std::make_shared<T>(m_data_queue.front()) );
    m_data_queue.pop();
    return data;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock( m_queue_mutex );
    return m_data_queue.empty();
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock( m_queue_mutex );
    return m_data_queue.size();
  }

private:
  std::queue<T>             m_data_queue;
  mutable std::mutex        m_queue_mutex;
  std::condition_variable   m_cond_var;
};
