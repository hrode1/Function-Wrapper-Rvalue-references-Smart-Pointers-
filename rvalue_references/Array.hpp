#include "MyInt.hpp"
#include<iostream>
#include <chrono>
namespace cs540
{
	class Array 
	{
		public:
		
		MyInt* data;
		std::size_t n;
		
		Array(std::initializer_list<MyInt> args) {
			data = new MyInt [args.size()];
			n=0;
			for (MyInt x : args) {data[n++]=x;}
		}
		
		
		// Copy constructor
		Array(const Array& ar) {
			data = new MyInt [ar.n];
			n=0;
			for(std::size_t i = 0; i< ar.n; i++)
			{
				data[n++]=ar.data[i];
			}
		}
		
		// Copy assignment operator
		Array &operator=(const Array& ar)
		{
			if(this != &ar)
			{
				// Deallocate old memory 
				delete [] data;
		
				// copy values 
				data = new MyInt [ar.n];
				n=0;
				for(std::size_t i = 0; i< ar.n; i++)
				{
					data[n++]=ar.data[i];
				}
			}
			return *this;
		}
		
		// Move constructor
		Array(Array&& o) : data(std::move(o.data)), n(std::move(o.n)) 
		{
			o.data = nullptr;
		}
		
		// Move assignment operator
		Array& operator=(Array&& o)
		{
			if(data!=nullptr)
				delete [] data;
			data = std::move(o.data);
			n = std::move(o.n);
			o.data = nullptr;
			return *this;
		}
		
		~Array()
		{
			delete [] data;
		}
		
		friend std::ostream &operator<<(std::ostream &os, const Array &ar) {
            for(std::size_t c=0;c<ar.n-1;c++)
				os << ar.data[c]<<", "; 
			os << ar.data[ar.n-1]; 
			return os;
        }
		
		static void move_performance_test()
		{
			using Milli = std::chrono::duration<double, std::ratio<1,1000>>;
			using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
			
			TimePoint start, end;
			long unsigned int count = 1000000;
			Array a1{1, 2, 3};
			
			std::cout << "Performance with copy constructor...\n";
			start = std::chrono::system_clock::now();			
			for(long unsigned int i = 0; i < count; i++) {
				Array a2{a1};
			}
			end = std::chrono::system_clock::now();		  
			Milli elapsed = end - start;		  
			std::cout << "Copy constructing " << count << " elements took " << elapsed.count() << " milliseconds" << std::endl;
			
			std::cout << "\nPerformance with move constructor...\n";
			start = std::chrono::system_clock::now();			
			for(long unsigned int i = 0; i < count; i++) {
				Array a3{std::move(a1)};
			}
			end = std::chrono::system_clock::now();		  
			elapsed = end - start;		  
			std::cout << "Move constructing " << count << " elements took " << elapsed.count() << " milliseconds" << std::endl;
			
			Array a6{1, 2, 3};
			std::cout << "\nPerformance with copy assignment operator...\n";
			Array a4{1};
			start = std::chrono::system_clock::now();			
			for(long unsigned int i = 0; i < count; i++) {
				a4 = a6;
			}
			end = std::chrono::system_clock::now();		  
			elapsed = end - start;		  
			std::cout << "Copy assigning " << count << " elements took " << elapsed.count() << " milliseconds" << std::endl;
			
			std::cout << "\nPerformance with move assignment operator...\n";
			Array a5{1};
			start = std::chrono::system_clock::now();			
			for(long unsigned int i = 0; i < count; i++) {
				a5 = std::move(a6);
			}
			end = std::chrono::system_clock::now();		  
			elapsed = end - start;		  
			std::cout << "Move assigning " << count << " elements took " << elapsed.count() << " milliseconds" << std::endl;
		  
		}
			
	};
}