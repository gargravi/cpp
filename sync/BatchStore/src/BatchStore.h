#include <functional>
#include <mutex>
#include <queue>

template< typename T_ITEM>
class BatchStore {
public:
	typedef T_ITEM value_type;
	typedef std::function< void(T_ITEM&)>					NotifyElementCreate;
	typedef std::function< void(NotifyElementCreate&)>		ProducerFunc;

public:
	BatchStore(ProducerFunc&& func) :
	    m_funcProducer(func),
		m_funcPushCallback([this](value_type& item) {
			m_que.push(item);
		})
	{}

	//TOOD: Move overload
	value_type& get()
	{
		std::lock_guard<decltype(m_mutx)>  lck(m_mutx);
		if (m_que.empty()) {
			produce();
		}
		T_ITEM item = m_que.front();
		m_que.pop();
		//std::cout << "Thread ID: " << std::this_thread::get_id << ", Item val :" << item << std::endl;
		return item;
	}

private:
	void produce()
	{
		//std::cout << "Going to produce" << std::endl;
		//NotifyElementCreate adder = [this](value_type& item) {
		//	m_que.push(item);
		//};
		//m_funcProducer(adder);
		m_funcProducer(m_funcPushCallback);
	}

private:
	std::mutex					m_mutx;
	std::queue<value_type>		m_que;
	ProducerFunc				m_funcProducer;
	NotifyElementCreate			m_funcPushCallback;
};