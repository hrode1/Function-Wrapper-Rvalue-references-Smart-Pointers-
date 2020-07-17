#include <cstddef>
#include <cxxabi.h>
#include <iostream>
#include <mutex>
std::mutex mtx;
#ifndef SHAREDPTR_HPP
#define SHAREDPTR_HPP
namespace cs540
{
	// Class representing a reference counter class 
	class Counter { 
		public: 
			// Constructor 
			Counter()
			{
				m_counter = 0;
			}
		  
			// Destructor 
			virtual ~Counter() {}
		  
			void reset() 
			{
				m_counter = 0;
			} 
		  
			int get() 
			{
				return m_counter;
			} 
		  
			void operator++(int) 
			{ 
				m_counter++;
			} 
			
			void set(int n) 
			{
				m_counter = n;
			}
		  
			void operator--(int) 
			{ 
				if(m_counter>0)
					m_counter--;
				else
					m_counter=0;
			} 
		  
		private: 
			int m_counter; 
	};
	
	template <typename T>
	class CounterObject : public Counter{
		public:
			T* data;
			
			CounterObject() {
				data = nullptr;
				reset();
			}
			CounterObject(T* temp) {
				data = temp;
				//int inc_count = get();
				set(1);
			}
			~CounterObject() {
				if(data)
					delete data;
				data = nullptr;
				reset();
			}
	};
	
	// Class representing a shared pointer 
	template <typename T>
	class SharedPtr
	{
		public: 
			// Reference counter 
			Counter* m_counter; 
		  
			// Shared pointer 
			T* m_ptr;
		
			// Constructs a smart pointer that points to null
			SharedPtr() : m_counter(nullptr), m_ptr(nullptr) { }
		
			// Constructs a smart pointer that points to the given object 
			// The reference count is initialized to one
			template <typename U> 
			explicit SharedPtr(U *sharedPtrObj) 
			{ 
				if(sharedPtrObj) {
					m_ptr = static_cast<T*>(sharedPtrObj);
					m_counter = new CounterObject<U>(sharedPtrObj);
				}
				
			}
			
			// Copy constructor
			SharedPtr(const SharedPtr &p) 
			{
				if(p.m_counter!=nullptr && p.m_ptr!=nullptr) {
					m_ptr = p.m_ptr;
					m_counter = p.m_counter;
					mtx.lock();
					if(m_counter != nullptr)
						(*m_counter)++; 
					mtx.unlock();
				}
				else {
					m_counter = nullptr;
					m_ptr = nullptr;
				}
			}
		
			/* If U * is not implicitly convertible to T *, use of the second constructor will result 
			in a compile-time error when the compiler attempts to instantiate the member template */
			template <typename U> SharedPtr(const SharedPtr<U> &p) 
			{
				if(p.m_counter!=nullptr && p.m_ptr!=nullptr) {
					m_ptr = static_cast<T*>(p.m_ptr);
					m_counter = (CounterObject<T>*)p.m_counter;
					mtx.lock();
					if(m_counter != nullptr)
						(*m_counter)++; 
					mtx.unlock();
				}
				else {
					m_counter = nullptr;
					m_ptr = nullptr;
				}
			}
		
			SharedPtr(SharedPtr &&p) 
			{
				if(p.m_counter!=nullptr && p.m_ptr!=nullptr) {
					m_ptr = p.m_ptr;
					m_counter = p.m_counter;
					p.m_ptr = nullptr;
					p.m_counter = nullptr;
				}
				else 
				{
					m_counter = nullptr;
					m_ptr = nullptr;
				}
			}
		
			template <typename U> SharedPtr(SharedPtr<U> &&p) 
			{
				if(p.m_counter!=nullptr && p.m_ptr!=nullptr) {
					m_ptr = static_cast<T*>(p.m_ptr);
					m_counter = (CounterObject<T>*)p.m_counter;
					p.m_ptr = nullptr;
					p.m_counter = nullptr;
				}
				else 
				{
					m_counter = nullptr;
					m_ptr = nullptr;
				}
			}
			
			SharedPtr &operator=(const SharedPtr &p)
			{
				if(this != &p)
				{
					mtx.lock();
					if(m_counter!=nullptr)
					{
						(*m_counter)--;
						if(m_counter->get() == 0)
						{
							if(m_counter)
								delete m_counter;
						}
					}
					mtx.unlock();
					m_ptr = p.m_ptr;
					m_counter = p.m_counter;
					
					mtx.lock();
					if(m_counter != nullptr)
						(*m_counter)++; 
					mtx.unlock();
				}
				return *this;
			}
			
			template <typename U>
			SharedPtr<T> &operator=(const SharedPtr<U> &p)
			{
				if(this != (SharedPtr<T>*)&p)
				{
					mtx.lock();
					if(m_counter!=nullptr)
					{
						(*m_counter)--;
						if(m_counter->get() == 0)
						{
							if(m_counter)
								delete m_counter;
						}
					}
					mtx.unlock();
					m_ptr = static_cast<T*>(p.m_ptr);
					m_counter = (CounterObject<T>*)p.m_counter;
					mtx.lock();
					if(m_counter != nullptr)
						(*m_counter)++; 
					mtx.unlock();
				}
				return *this;
			}
			
			SharedPtr &operator=(SharedPtr &&p)
			{
					mtx.lock();
					if(m_counter!=nullptr)
					{
						(*m_counter)--;
						if(m_counter->get()==0)
						{
							if(m_counter)
								delete m_counter;
						}
					}	
					mtx.unlock();
					m_ptr = p.m_ptr;
					m_counter = p.m_counter;
					p.m_ptr = nullptr;
					p.m_counter = nullptr;	
					
				return *this;
			}
			
			template <typename U> SharedPtr &operator=(SharedPtr<U> &&p)
			{
					mtx.lock();
					if(m_counter!=nullptr)
					{
						(*m_counter)--;
						if(m_counter->get()==0)
						{
							if(m_counter)
								delete m_counter;
						}
					}	
					mtx.unlock();
					m_ptr = static_cast<T*>(p.m_ptr);
					m_counter = (CounterObject<T>*)p.m_counter;
					p.m_ptr = nullptr;
					p.m_counter = nullptr;	
				return *this;
			}
			
			~SharedPtr()
			{
				mtx.lock();
				if(m_counter!=nullptr) 
				{
					(*m_counter)--;
					if(m_counter->get() == 0)
					{
						if(m_counter)
							delete m_counter;
						m_counter = nullptr;
						m_ptr=nullptr;
					}
				}
				mtx.unlock();
			}
			
			void reset()
			{
				mtx.lock();
				if(m_counter!=nullptr)
				{
					(*m_counter)--;
					if(m_counter->get() == 0)
					{
						if(m_counter)
							delete m_counter;
					}
				}
				mtx.unlock();	
				m_counter = nullptr;
				m_ptr = nullptr;
			}
			
			template <typename U> void reset(U *p)
			{
				if(p) {
					mtx.lock();					
					if(m_counter!=nullptr)
					{
						(*m_counter)--;
						if(m_counter->get() == 0)
						{
							if(m_counter)
								delete m_counter;
						}
					}
					mtx.unlock();					
					m_counter = new CounterObject<U>(p); 
					m_ptr = static_cast<T*>(p);
				}
			}
			
			T *get() const
			{
				return m_ptr; 
			}
			
			T &operator*() const
			{
				return *m_ptr;
			}
			
			T *operator->() const
			{
				return m_ptr;
			}
			
			explicit operator bool() const
			{
				if(m_ptr==nullptr)
				{
					return false;
				}
				else
					return true;
			}
		
	};
	
	template <typename T1, typename T2>
	bool operator==(const SharedPtr<T1> &ptr1, const SharedPtr<T2> &ptr2)
	{
		if( (ptr1.m_ptr == ptr2.m_ptr) || (ptr1.m_ptr==nullptr && ptr2.m_ptr == nullptr) )
			return true;
		else
			return false;
	}
	
	template <typename T>
	bool operator==(const SharedPtr<T> &ptr, std::nullptr_t)
	{
		return !ptr;
	}
	
	template <typename T>
	bool operator==(std::nullptr_t, const SharedPtr<T> &ptr)
	{
		return !ptr;		
	}
	

	template <typename T1, typename T2>
	bool operator!=(const SharedPtr<T1>& ptr1, const SharedPtr<T2>& ptr2)
	{
		if( (ptr1.m_ptr == ptr2.m_ptr) || (ptr1.m_ptr==nullptr && ptr2.m_ptr == nullptr) )
			return false;
		else
			return true; 
	}

	template <typename T>
	bool operator!=(const SharedPtr<T> &ptr, std::nullptr_t)
	{
		return (bool)ptr;
	}
	
	template <typename T>
	bool operator!=(std::nullptr_t, const SharedPtr<T> &ptr)
	{
		return (bool)ptr;
	}
	
	template <typename T, typename U>
	SharedPtr<T> static_pointer_cast(const SharedPtr<U> &sp)
	{
		SharedPtr<T> ret_ptr(sp);
		ret_ptr.m_ptr = static_cast<T*>(sp.m_ptr);
		return ret_ptr;
	}
	
	template <typename T, typename U>
	SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &sp)
	{
		SharedPtr<T> ret_ptr(sp);
		ret_ptr.m_ptr = dynamic_cast<T*>(sp.m_ptr);
		return ret_ptr;
	}

}
#endif