#include <stdexcept>

#ifndef FUNCTION_HPP
#define FUNCTION_HPP

namespace cs540 {

	class BadFunctionCall: public std::exception {
	public:
		virtual const char* What() const throw () {
			return "Bad Function Call";
		}
	};

	template<typename>
	class Function;

	template<typename ResultType, typename ... ArgumentTypes>
	class Function<ResultType(ArgumentTypes...)> {

		private:

			class ICallable {
			public:
				virtual ResultType operator()(ArgumentTypes ... args) = 0;
				virtual ICallable *invoke() const = 0;
				virtual ~ICallable() = default;
			};

			template<typename FunctionType>
			class CallableT : public ICallable
			{
			public:
				CallableT(const FunctionType &f) : func_type(f) { }

				ResultType operator()(ArgumentTypes ... args) override {
					return func_type(args...);
				}

				CallableT *invoke() const override {
					return new CallableT { func_type };
				}
				
				~CallableT() override = default;
			
			private:
				FunctionType func_type;

			};
			
			ICallable *func_wrap;

		public:

			// Constructs a function wrapper that does not yet reference a target function
			Function()
			{
				func_wrap = nullptr;
			}
			
			// Constructs a function wrapper with a function of FunctionType as its target
			template<typename FunctionType> 
			Function(FunctionType functionType)
			{
				func_wrap = new CallableT<FunctionType>(functionType);
			}
			
			// Copy construct from an existing cs540::Function
			Function(const Function &f)
			{
				if (f.func_wrap == nullptr) {
					func_wrap = nullptr;
				} else {
					func_wrap = f.func_wrap->invoke();
				}
			}
			
			// Copy assignment
			Function &operator=(const Function &f)
			{
				if (this == &f) {
					return *this;
				}
				delete func_wrap;
				func_wrap = f.func_wrap->invoke();
				return *this;
			}
			
			// Destructor
			~Function()
			{
				delete func_wrap;
			}
			
			// Call the wrapped function target with the given arguments and return its result
			ResultType operator()(ArgumentTypes...args)
			{
				if (func_wrap == nullptr) {
					// Throw cs540::BadFunctionCall if a callable function is not stored
					throw BadFunctionCall();
				}
				return (*func_wrap)(args...);
			}
			
			// Return true if the cs540::Function references a callable function and false otherwise
			explicit operator bool() const
			{
				if (func_wrap == nullptr) {
					return false;
				}
				return true;
			}
	};
	
	template <typename ResultType, typename... ArgumentTypes>
	bool operator==(const Function<ResultType(ArgumentTypes...)> &f, std::nullptr_t)
	{
		return !f;
	}

	template <typename ResultType, typename... ArgumentTypes>
	bool operator==(std::nullptr_t, const Function<ResultType(ArgumentTypes...)> &f)
	{
		return !f;
	}
	
	template <typename ResultType, typename... ArgumentTypes>
	bool operator!=(const Function<ResultType(ArgumentTypes...)> &f, std::nullptr_t)
	{
		return bool(f);
	}

	template <typename ResultType, typename... ArgumentTypes>
	bool operator!=(std::nullptr_t, const Function<ResultType(ArgumentTypes...)> &f)
	{
		return bool(f);
	}

}
#endif