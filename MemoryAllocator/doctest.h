//
// doctest.h - the lightest feature rich C++ single header testing framework
//
// Copyright (c) 2016 Viktor Kirilov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
// The documentation can be found at the library's page:
// https://github.com/onqtam/doctest/blob/master/doc/markdown/readme.md
//
// The library is heavily influenced by Catch - https://github.com/philsquared/Catch
// which uses the Boost Software License - Version 1.0
// see here - https://github.com/philsquared/Catch/blob/master/LICENSE_1_0.txt
// The concept of subcases (sections in Catch) and expression decomposition are from there.
// Some parts of the code are taken directly:
// - stringification - the detection of "ostream& operator<<(ostream&, const T&)" and the StringMaker class
// - the Approx() helper class for floating point comparison
// - colors in the console
// - breaking into a debugger

// =================================================================================================
// =================================================================================================
// =================================================================================================

// Suppress this globally - there is no way to silence it in the expression decomposition macros
// _Pragma() in macros doesn't work for the c++ front-end of g++
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55578
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69543
// Also the warning is completely worthless nowadays - http://stackoverflow.com/questions/14016993
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Waggregate-return"
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif // __clang__

#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
#pragma GCC diagnostic push
#endif // > gcc 4.6
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Winline"
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif // > gcc 4.6
//#pragma GCC diagnostic ignored "-Wlong-long"
#endif // __GNUC__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // The compiler encountered a deprecated declaration
#pragma warning(disable : 4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable : 4706) // assignment within conditional expression
#pragma warning(disable : 4512) // 'class' : assignment operator could not be generated
#pragma warning(disable : 4127) // conditional expression is constant
#endif                          // _MSC_VER

#ifndef DOCTEST_LIBRARY_INCLUDED
#define DOCTEST_LIBRARY_INCLUDED

#define DOCTEST_VERSION_MAJOR 1
#define DOCTEST_VERSION_MINOR 0
#define DOCTEST_VERSION_PATCH 0
#define DOCTEST_VERSION "1.0.0"

// internal macros for string concatenation and anonymous variable name generation
#define DOCTEST_CONCAT_IMPL(s1, s2) s1##s2
#define DOCTEST_CONCAT(s1, s2) DOCTEST_CONCAT_IMPL(s1, s2)
#ifdef __COUNTER__ // not standard and may be missing for some compilers
#define DOCTEST_ANONYMOUS(x) DOCTEST_CONCAT(x, __COUNTER__)
#else // __COUNTER__
#define DOCTEST_ANONYMOUS(x) DOCTEST_CONCAT(x, __LINE__)
#endif // __COUNTER__

// internal macro for making a string
#define DOCTEST_TOSTR_IMPL(x) #x
#define DOCTEST_TOSTR(x) DOCTEST_TOSTR_IMPL(x)

// internal macro for concatenating 2 literals and making the result a string
#define DOCTEST_STR_CONCAT_TOSTR(s1, s2) DOCTEST_TOSTR(s1) DOCTEST_TOSTR(s2)

// not using __APPLE__ because... this is how Catch does it
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
#define DOCTEST_PLATFORM_MAC
#elif defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
#define DOCTEST_PLATFORM_IPHONE
#elif defined(_WIN32) || defined(_MSC_VER)
#define DOCTEST_PLATFORM_WINDOWS
#else
#define DOCTEST_PLATFORM_LINUX
#endif

#define DOCTEST_COUNTOF(x) (sizeof(x) / sizeof(x[0]))

#define DOCTEST_GCS doctest::detail::getTestsContextState

// snprintf() not in the C++98 standard
#ifdef _MSC_VER
#define DOCTEST_SNPRINTF _snprintf
#else
#define DOCTEST_SNPRINTF snprintf
#endif

// for anything below Visual Studio 2005 (VC++6 has no __debugbreak() - not sure about VS 2003)
#if defined(_MSC_VER) && _MSC_VER < 1400
#define __debugbreak() __asm { int 3}
#endif

#ifdef DOCTEST_PLATFORM_MAC
// The following code snippet based on:
// http://cocoawithlove.com/2008/03/break-into-debugger.html
#ifdef DEBUG
#if defined(__ppc64__) || defined(__ppc__)
#define DOCTEST_BREAK_INTO_DEBUGGER()                                                              \
    if(doctest::detail::isDebuggerActive() && !DOCTEST_GCS()->no_breaks)                           \
    __asm__("li r0, 20\nsc\nnop\nli r0, 37\nli r4, 2\nsc\nnop\n" : : : "memory", "r0", "r3", "r4")
#else // __ppc64__ || __ppc__
#define DOCTEST_BREAK_INTO_DEBUGGER()                                                              \
    if(doctest::detail::isDebuggerActive() && !DOCTEST_GCS()->no_breaks)                           \
    __asm__("int $3\n" : :)
#endif // __ppc64__ || __ppc__
#endif // DEBUG
#elif defined(_MSC_VER)
#define DOCTEST_BREAK_INTO_DEBUGGER()                                                              \
    if(doctest::detail::isDebuggerActive() && !DOCTEST_GCS()->no_breaks)                           \
    __debugbreak()
#elif defined(__MINGW32__)
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#define DOCTEST_BREAK_INTO_DEBUGGER()                                                              \
    if(doctest::detail::isDebuggerActive() && !DOCTEST_GCS()->no_breaks)                           \
    ::DebugBreak()
#else // linux
#define DOCTEST_BREAK_INTO_DEBUGGER() ((void)0)
#endif // linux

#ifdef __clang__
#include <ciso646>
#endif // __clang__

#ifdef _LIBCPP_VERSION
#include <iosfwd>
#else // _LIBCPP_VERSION
#ifndef DOCTEST_CONFIG_USE_IOSFWD
namespace std
{
	template <class charT>
	struct char_traits;
	template <>
	struct char_traits<char>;
	template <class charT, class traits>
	class basic_ostream;
	typedef basic_ostream<char, char_traits<char> > ostream;
}
#else // DOCTEST_CONFIG_USE_IOSFWD
#include <iosfwd>
#endif // DOCTEST_CONFIG_USE_IOSFWD
#endif // _LIBCPP_VERSION

#ifndef DOCTEST_CONFIG_WITH_LONG_LONG
#if __cplusplus >= 201103L || (defined(_MSC_VER) && (_MSC_VER >= 1400))
#define DOCTEST_CONFIG_WITH_LONG_LONG
#endif // __cplusplus / _MSC_VER
#endif // DOCTEST_CONFIG_WITH_LONG_LONG

namespace doctest
{
	class String
	{
		char* m_str;

		void copy(const String& other);

	public:
		String(const char* in = "");
		String(const String& other);
		~String();

		String& operator=(const String& other);

		String operator+(const String& other) const;
		String& operator+=(const String& other);

		char& operator[](unsigned pos) { return m_str[pos]; }
		const char& operator[](unsigned pos) const { return m_str[pos]; }

		char*       c_str() { return m_str; }
		const char* c_str() const { return m_str; }

		unsigned size() const;
		unsigned length() const;

		int compare(const char* other, bool no_case = false) const;
		int compare(const String& other, bool no_case = false) const;
	};

	// clang-format off
	inline bool operator==(const String& lhs, const String& rhs) { return lhs.compare(rhs) == 0; }
	inline bool operator!=(const String& lhs, const String& rhs) { return lhs.compare(rhs) != 0; }
	inline bool operator< (const String& lhs, const String& rhs) { return lhs.compare(rhs) < 0; }
	inline bool operator> (const String& lhs, const String& rhs) { return lhs.compare(rhs) > 0; }
	inline bool operator<=(const String& lhs, const String& rhs) { return (lhs != rhs) ? lhs.compare(rhs) < 0 : true; }
	inline bool operator>=(const String& lhs, const String& rhs) { return (lhs != rhs) ? lhs.compare(rhs) > 0 : true; }
	// clang-format on

	std::ostream& operator<<(std::ostream& stream, const String& in);

	namespace detail
	{
		template <bool>
		struct STATIC_ASSERT_No_output_stream_operator_found_for_type;

		template <>
		struct STATIC_ASSERT_No_output_stream_operator_found_for_type<true>
		{};

		namespace has_insertion_operator_impl
		{
			typedef char no;
			typedef char yes[2];

			template <bool in>
			void           f() {
				STATIC_ASSERT_No_output_stream_operator_found_for_type<in>();
			}

			struct any_t
			{
				template <typename T>
				any_t(const T&) {
					f<false>();
				}
			};

			yes& testStreamable(std::ostream&);
			no   testStreamable(no);

			no operator<<(std::ostream&, const any_t&);

			template <typename T>
			struct has_insertion_operator
			{
				static std::ostream& s;
				static const T&      t;
				// for anything below Visual Studio 2005 (VC++6 is troublesome - not sure about VS 2003)
#if defined(_MSC_VER) && _MSC_VER < 1400
				enum
				{
					value
				};
#else // _MSC_VER
				static const bool value = sizeof(testStreamable(s << t)) == sizeof(yes);
#endif // _MSC_VER
			};
		} // namespace has_insertion_operator_impl

		template <typename T>
		struct has_insertion_operator : has_insertion_operator_impl::has_insertion_operator<T>
		{};

		std::ostream* createStream();
		String        getStreamResult(std::ostream*);
		void          freeStream(std::ostream*);

		template <bool C>
		struct StringMakerBase
		{
			template <typename T>
			static String convert(const T&) {
				return "{?}";
			}
		};

		template <>
		struct StringMakerBase<true>
		{
			template <typename T>
			static String convert(const T& in) {
				std::ostream* stream = createStream();
				*stream << in;
				String result = getStreamResult(stream);
				freeStream(stream);
				return result;
			}
		};

		String rawMemoryToString(const void* object, unsigned size);

		template <typename T>
		String rawMemoryToString(const T& object) {
			return rawMemoryToString(&object, sizeof(object));
		}
	} // namespace detail

	  // for anything below Visual Studio 2005 (VC++6 is troublesome - not sure about VS 2003)
#if defined(_MSC_VER) && _MSC_VER < 1400
	template <typename T>
	struct StringMaker : detail::StringMakerBase<false>
	{};
#else  // _MSC_VER
	template <typename T>
	struct StringMaker : detail::StringMakerBase<detail::has_insertion_operator<T>::value>
	{};
#endif // _MSC_VER

	// not for anything below Visual Studio 2005 (VC++6 is troublesome - not sure about VS 2003)
#if defined(_MSC_VER) && _MSC_VER >= 1400
	template <typename T>
	struct StringMaker<T*>
	{
		template <typename U>
		static String convert(U* p) {
			if (!p)
				return "NULL";
			else
				return detail::rawMemoryToString(p);
		}
	};

	template <typename R, typename C>
	struct StringMaker<R C::*>
	{
		static String convert(R C::*p) {
			if (!p)
				return "NULL";
			else
				return detail::rawMemoryToString(p);
		}
	};
#endif // _MSC_VER

	template <typename T>
	String toString(const T& value) {
		return StringMaker<T>::convert(value);
	}

	String toString(const char* in);
	String toString(bool in);
	String toString(float in);
	String toString(double in);
	String toString(double long in);

	String toString(char in);
	String toString(char unsigned in);
	String toString(int short in);
	String toString(int short unsigned in);
	String toString(int in);
	String toString(int unsigned in);
	String toString(int long in);
	String toString(int long unsigned in);

#ifdef DOCTEST_CONFIG_WITH_LONG_LONG
	String toString(int long long in);
	String toString(int long long unsigned in);
#endif // DOCTEST_CONFIG_WITH_LONG_LONG

	class Approx
	{
	public:
		explicit Approx(double value);

		Approx(Approx const& other)
			: m_epsilon(other.m_epsilon)
			, m_scale(other.m_scale)
			, m_value(other.m_value) {}

		Approx operator()(double value) {
			Approx approx(value);
			approx.epsilon(m_epsilon);
			approx.scale(m_scale);
			return approx;
		}

		friend bool operator==(double lhs, Approx const& rhs);
		friend bool operator==(Approx const& lhs, double rhs) { return operator==(rhs, lhs); }
		friend bool operator!=(double lhs, Approx const& rhs) { return !operator==(lhs, rhs); }
		friend bool operator!=(Approx const& lhs, double rhs) { return !operator==(rhs, lhs); }

		Approx& epsilon(double newEpsilon) {
			m_epsilon = newEpsilon;
			return *this;
		}

		Approx& scale(double newScale) {
			m_scale = newScale;
			return *this;
		}

		String toString() const;

	private:
		double m_epsilon;
		double m_scale;
		double m_value;
	};

	template <>
	inline String toString<Approx>(Approx const& value) {
		return value.toString();
	}

#if !defined(DOCTEST_CONFIG_DISABLE)

	namespace detail
	{
		// the function type this library works with
		typedef void(*funcType)(void);

		// not for anything below Visual Studio 2005 (VC++6 has no SFINAE - not sure about VS 2003)
#if !defined(_MSC_VER) || _MSC_VER >= 1400
		// clang-format off
		template<class T>               struct decay_array { typedef T type; };
		template<class T, unsigned N>   struct decay_array<T[N]> { typedef T* type; };
		template<class T>               struct decay_array<T[]> { typedef T* type; };

		template<class T>   struct not_char_pointer { enum { value = true }; };
		template<>          struct not_char_pointer<char*> { enum { value = false }; };
		template<>          struct not_char_pointer<const char*> { enum { value = false }; };

		template<class T> struct can_use_op : not_char_pointer<typename decay_array<T>::type> {};

		template<bool, class = void> struct enable_if {};
		template<class T> struct enable_if<true, T> { typedef T type; };
		// clang-format on
#endif // _MSC_VER

		struct TestFailureException
		{};

		void checkIfShouldThrow(const char* assert_name);
		void  throwException();
		bool  always_false();
		void* getNullPtr();

		// a struct defining a registered test callback
		struct TestData
		{
			// not used for determining uniqueness
			const char* m_suite; // the test suite in which the test was added
			const char* m_name;  // name of the test function
			funcType    m_f;     // a function pointer to the test function

								 // fields by which uniqueness of test cases shall be determined
			const char* m_file; // the file in which the test was registered
			unsigned    m_line; // the line where the test was registered

			TestData(const char* suite, const char* name, funcType f, const char* file, int line)
				: m_suite(suite)
				, m_name(name)
				, m_f(f)
				, m_file(file)
				, m_line(line) {}

			bool operator==(const TestData& other) const;
		};

		struct Subcase
		{
			const char* m_name;
			const char* m_file;
			int         m_line;
			bool        m_entered;

			Subcase(const char* name, const char* file, int line);
			~Subcase();
			Subcase(const Subcase& other);
			Subcase& operator=(const Subcase& other);

			bool operator==(const Subcase& other) const;
			operator bool() const { return m_entered; }
		};

		template <typename L, typename R>
		String stringifyBinaryExpr(const L& lhs, const char* op, const R& rhs) {
			return toString(lhs) + " " + op + " " + toString(rhs);
		}

		// TODO: think about this
		//struct STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison;

		struct Result
		{
			bool   m_passed;
			String m_decomposition;

			// to fix gcc 4.7 "-Winline" warnings
#if defined(__GNUC__) && !defined(__clang__)
			__attribute__((noinline))
#endif
				~Result() {
			}

			Result(bool passed = false, const String& decomposition = String())
				: m_passed(passed)
				, m_decomposition(decomposition) {}

			// to fix gcc 4.7 "-Winline" warnings
#if defined(__GNUC__) && !defined(__clang__)
			__attribute__((noinline))
#endif
				Result&
				operator=(const Result& other) {
				m_passed = other.m_passed;
				m_decomposition = other.m_decomposition;

				return *this;
			}

			operator bool() { return !m_passed; }

			void invert() { m_passed = !m_passed; }

			// clang-format off
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator+(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator-(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator/(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator*(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator&&(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator||(const R&);
			//
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator==(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator!=(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator<(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator<=(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator>(const R&);
			//template <typename R> STATIC_ASSERT_Expression_Too_Complex_Please_Rewrite_As_Binary_Comparison& operator>=(const R&);
			// clang-format on
		};

		// clang-format off

		// for anything below Visual Studio 2005 (VC++6 has no SFINAE - not sure about VS 2003)
#if defined(_MSC_VER) && _MSC_VER < 1400
		template <typename L, typename R> bool eq(const L& lhs, const R& rhs) { return lhs == rhs; }
		template <typename L, typename R> bool neq(const L& lhs, const R& rhs) { return lhs != rhs; }
		template <typename L, typename R> bool lt(const L& lhs, const R& rhs) { return lhs < rhs; }
		template <typename L, typename R> bool gt(const L& lhs, const R& rhs) { return lhs > rhs; }
		template <typename L, typename R> bool lte(const L& lhs, const R& rhs) { return eq(lhs, rhs) != 0 ? lhs < rhs : true; }
		template <typename L, typename R> bool gte(const L& lhs, const R& rhs) { return eq(lhs, rhs) != 0 ? lhs > rhs : true; }
#else // _MSC_VER
		template <typename L, typename R>
		typename enable_if<can_use_op<L>::value || can_use_op<R>::value, bool>::type eq(const L& lhs, const R& rhs) { return lhs == rhs; }
		template <typename L, typename R>
		typename enable_if<can_use_op<L>::value || can_use_op<R>::value, bool>::type neq(const L& lhs, const R& rhs) { return lhs != rhs; }
		template <typename L, typename R>
		typename enable_if<can_use_op<L>::value || can_use_op<R>::value, bool>::type lt(const L& lhs, const R& rhs) { return lhs < rhs; }
		template <typename L, typename R>
		typename enable_if<can_use_op<L>::value || can_use_op<R>::value, bool>::type gt(const L& lhs, const R& rhs) { return lhs > rhs; }
		template <typename L, typename R>
		typename enable_if<can_use_op<L>::value || can_use_op<R>::value, bool>::type lte(const L& lhs, const R& rhs) { return neq(lhs, rhs) ? lhs < rhs : true; }
		template <typename L, typename R>
		typename enable_if<can_use_op<L>::value || can_use_op<R>::value, bool>::type gte(const L& lhs, const R& rhs) { return neq(lhs, rhs) ? lhs > rhs : true; }

		inline bool eq(const char* lhs, const char* rhs) { return String(lhs) == String(rhs); }
		inline bool neq(const char* lhs, const char* rhs) { return String(lhs) != String(rhs); }
		inline bool lt(const char* lhs, const char* rhs) { return String(lhs) < String(rhs); }
		inline bool gt(const char* lhs, const char* rhs) { return String(lhs) > String(rhs); }
		inline bool lte(const char* lhs, const char* rhs) { return String(lhs) <= String(rhs); }
		inline bool gte(const char* lhs, const char* rhs) { return String(lhs) >= String(rhs); }
#endif // _MSC_VER

		// clang-format on

		template <typename L>
		struct Expression_lhs
		{
			L lhs;

			Expression_lhs(L in)
				: lhs(in) {}

			Expression_lhs(const Expression_lhs& other)
				: lhs(other.lhs) {}

			operator Result() { return Result(!!lhs, toString(lhs)); }

			// clang-format off
			template <typename R> Result operator==(const R& rhs) { return Result(eq(lhs, rhs), stringifyBinaryExpr(lhs, "==", rhs)); }
			template <typename R> Result operator!=(const R& rhs) { return Result(neq(lhs, rhs), stringifyBinaryExpr(lhs, "!=", rhs)); }
			template <typename R> Result operator< (const R& rhs) { return Result(lt(lhs, rhs), stringifyBinaryExpr(lhs, "<", rhs)); }
			template <typename R> Result operator<=(const R& rhs) { return Result(lte(lhs, rhs), stringifyBinaryExpr(lhs, "<=", rhs)); }
			template <typename R> Result operator> (const R& rhs) { return Result(gt(lhs, rhs), stringifyBinaryExpr(lhs, ">", rhs)); }
			template <typename R> Result operator>=(const R& rhs) { return Result(gte(lhs, rhs), stringifyBinaryExpr(lhs, ">=", rhs)); }
			// clang-format on
		};

		struct ExpressionDecomposer
		{
			template <typename L>
			Expression_lhs<const L&> operator<<(const L& operand) {
				return Expression_lhs<const L&>(operand);
			}
		};

		// forward declarations of functions used by the macros
		int regTest(void(*f)(void), unsigned line, const char* file, const char* name);
		int setTestSuiteName(const char* name);

		void addFailedAssert(const char* assert_name);

		void logTestStart(const char* name, const char* file, unsigned line);
		void logTestEnd();

		void logTestCrashed();

		void logAssert(bool passed, const char* decomposition, bool threw, const char* expr,
			const char* assert_name, const char* file, int line);

		void logAssertThrows(bool threw, const char* expr, const char* assert_name, const char* file,
			int line);

		void logAssertThrowsAs(bool threw, bool threw_as, const char* as, const char* expr,
			const char* assert_name, const char* file, int line);

		void logAssertNothrow(bool threw, const char* expr, const char* assert_name, const char* file,
			int line);

		bool isDebuggerActive();
		void writeToDebugConsole(const String&);

		struct TestAccessibleContextState
		{
			bool            success;   // include successful assertions in output
			bool            no_throw;  // to skip exceptions-related assertion macros
			bool            no_breaks; // to not break into the debugger
			const TestData* currentTest;
			bool            hasLoggedCurrentTestStart;
			int             numAssertionsForCurrentTestcase;
		};

		struct ContextState;

		TestAccessibleContextState* getTestsContextState();
	} // namespace detail

#endif // DOCTEST_CONFIG_DISABLE

	class Context
	{
#if !defined(DOCTEST_CONFIG_DISABLE)
		detail::ContextState* p;

		void parseArgs(int argc, const char* const* argv, bool withDefaults = false);

#endif // DOCTEST_CONFIG_DISABLE

	public:
		Context(int argc, const char* const* argv);

		// to fix gcc 4.7 "-Winline" warnings
#if defined(__GNUC__) && !defined(__clang__)
		__attribute__((noinline))
#endif
			~Context();

		void addFilter(const char* filter, const char* value);
		void setOption(const char* option, int value);
		void setOption(const char* option, const char* value);

		bool shouldExit();

		int run();
	};

} // namespace doctest

  // if registering is not disabled
#if !defined(DOCTEST_CONFIG_DISABLE)

  // registers the test by initializing a dummy var with a function
#if defined(__GNUC__) && !defined(__clang__)
#define DOCTEST_REGISTER_FUNCTION(f, name)                                                         \
    static int DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) __attribute__((unused)) =                   \
            doctest::detail::regTest(f, __LINE__, __FILE__, name);
#elif defined(__clang__)
#define DOCTEST_REGISTER_FUNCTION(f, name)                                                         \
    _Pragma("clang diagnostic push")                                                               \
            _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") static int               \
                    DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) =                                      \
                            doctest::detail::regTest(f, __LINE__, __FILE__, name);                 \
    _Pragma("clang diagnostic pop")
#else // MSVC
#define DOCTEST_REGISTER_FUNCTION(f, name)                                                         \
    static int DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) =                                           \
            doctest::detail::regTest(f, __LINE__, __FILE__, name);
#endif // MSVC

#define DOCTEST_IMPLEMENT_FIXTURE(der, base, func, name)                                           \
    namespace                                                                                      \
    {                                                                                              \
        struct der : base                                                                          \
        { void f(); };                                                                             \
        static void func() {                                                                       \
            der v;                                                                                 \
            v.f();                                                                                 \
        }                                                                                          \
        DOCTEST_REGISTER_FUNCTION(func, name)                                                      \
    }                                                                                              \
    inline void der::f()

#define DOCTEST_CREATE_AND_REGISTER_FUNCTION(f, name)                                              \
    static void f();                                                                               \
    DOCTEST_REGISTER_FUNCTION(f, name)                                                             \
    inline void f()

  // for registering tests
#define DOCTEST_TEST_CASE(name)                                                                    \
    DOCTEST_CREATE_AND_REGISTER_FUNCTION(DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FUNC_), name)

  // for registering tests with a fixture
#define DOCTEST_TEST_CASE_FIXTURE(c, name)                                                         \
    DOCTEST_IMPLEMENT_FIXTURE(DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_CLASS_), c,                        \
                              DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FUNC_), name)

  // for subcases
#if defined(__GNUC__)
#define DOCTEST_SUBCASE(name)                                                                      \
    if(const doctest::detail::Subcase & DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_SUBCASE_)                \
                                                __attribute__((unused)) =                          \
               doctest::detail::Subcase(name, __FILE__, __LINE__))
#else // __GNUC__
#define DOCTEST_SUBCASE(name)                                                                      \
    if(const doctest::detail::Subcase & DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_SUBCASE_) =              \
               doctest::detail::Subcase(name, __FILE__, __LINE__))
#endif // __GNUC__

  // for starting a testsuite block
#if defined(__GNUC__) && !defined(__clang__)
#define DOCTEST_TEST_SUITE(name)                                                                   \
    static int DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) __attribute__((unused)) =                   \
            doctest::detail::setTestSuiteName(name);                                               \
    void DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FOR_SEMICOLON_)()
#elif defined(__clang__)
#define DOCTEST_TEST_SUITE(name)                                                                   \
    _Pragma("clang diagnostic push")                                                               \
            _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") static int               \
                    DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) =                                      \
                            doctest::detail::setTestSuiteName(name);                               \
    _Pragma("clang diagnostic pop") void DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FOR_SEMICOLON_)()
#else // MSVC
#define DOCTEST_TEST_SUITE(name)                                                                   \
    static int DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) = doctest::detail::setTestSuiteName(name);  \
    void       DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FOR_SEMICOLON_)()
#endif // MSVC

  // for ending a testsuite block
#if defined(__GNUC__) && !defined(__clang__)
#define DOCTEST_TEST_SUITE_END                                                                     \
    static int DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) __attribute__((unused)) =                   \
            doctest::detail::setTestSuiteName("");                                                 \
    void DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_TESTSUITE_END_)
#elif defined(__clang__)
#define DOCTEST_TEST_SUITE_END                                                                     \
    _Pragma("clang diagnostic push")                                                               \
            _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") static int               \
                    DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) =                                      \
                            doctest::detail::setTestSuiteName("");                                 \
    _Pragma("clang diagnostic pop") void DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_TESTSUITE_END_)
#else // MSVC
#define DOCTEST_TEST_SUITE_END                                                                     \
    static int DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_VAR_) = doctest::detail::setTestSuiteName("");    \
    void       DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_TESTSUITE_END_)
#endif // MSVC

#define DOCTEST_LOG_START()                                                                        \
    do {                                                                                           \
        if(!DOCTEST_GCS()->hasLoggedCurrentTestStart) {                                            \
            doctest::detail::logTestStart(DOCTEST_GCS()->currentTest->m_name,                      \
                                          DOCTEST_GCS()->currentTest->m_file,                      \
                                          DOCTEST_GCS()->currentTest->m_line);                     \
            DOCTEST_GCS()->hasLoggedCurrentTestStart = true;                                       \
        }                                                                                          \
    } while(doctest::detail::always_false())

#define DOCTEST_ASSERT_IMPLEMENT(expr, assert_name, false_invert_op)                               \
    doctest::detail::Result res;                                                                   \
    bool                    threw = false;                                                         \
    try {                                                                                          \
        res = doctest::detail::ExpressionDecomposer() << expr;                                     \
    } catch(...) { threw = true; }                                                                 \
    false_invert_op;                                                                               \
    if(res || DOCTEST_GCS()->success) {                                                            \
        DOCTEST_LOG_START();                                                                       \
        doctest::detail::logAssert(res.m_passed, res.m_decomposition.c_str(), threw, #expr,        \
                                   assert_name, __FILE__, __LINE__);                               \
    }                                                                                              \
    DOCTEST_GCS()->numAssertionsForCurrentTestcase++;                                              \
    if(res) {                                                                                      \
        doctest::detail::addFailedAssert(assert_name);                                             \
        DOCTEST_BREAK_INTO_DEBUGGER();                                                             \
        doctest::detail::checkIfShouldThrow(assert_name);                                          \
    }

#if defined(__clang__)
#define DOCTEST_ASSERT_PROXY(expr, assert_name, false_invert_op)                                   \
    do {                                                                                           \
        _Pragma("clang diagnostic push")                                                           \
                _Pragma("clang diagnostic ignored \"-Woverloaded-shift-op-parentheses\"")          \
                        DOCTEST_ASSERT_IMPLEMENT(expr, assert_name, false_invert_op)               \
                                _Pragma("clang diagnostic pop")                                    \
    } while(doctest::detail::always_false())
#else // __clang__
#define DOCTEST_ASSERT_PROXY(expr, assert_name, false_invert_op)                                   \
    do {                                                                                           \
        DOCTEST_ASSERT_IMPLEMENT(expr, assert_name, false_invert_op)                               \
    } while(doctest::detail::always_false())
#endif // __clang__

#define DOCTEST_WARN(expr) DOCTEST_ASSERT_PROXY(expr, "WARN", ((void)0))
#define DOCTEST_CHECK(expr) DOCTEST_ASSERT_PROXY(expr, "CHECK", ((void)0))
#define DOCTEST_REQUIRE(expr) DOCTEST_ASSERT_PROXY(expr, "REQUIRE", ((void)0))

#define DOCTEST_WARN_FALSE(expr) DOCTEST_ASSERT_PROXY(expr, "WARN_FALSE", res.invert())
#define DOCTEST_CHECK_FALSE(expr) DOCTEST_ASSERT_PROXY(expr, "CHECK_FALSE", res.invert())
#define DOCTEST_REQUIRE_FALSE(expr) DOCTEST_ASSERT_PROXY(expr, "REQUIRE_FALSE", res.invert())

#define DOCTEST_ASSERT_THROWS(expr, assert_name)                                                   \
    do {                                                                                           \
        if(!DOCTEST_GCS()->no_throw) {                                                             \
            bool threw = false;                                                                    \
            try {                                                                                  \
                expr;                                                                              \
            } catch(...) { threw = true; }                                                         \
            if(!threw || DOCTEST_GCS()->success) {                                                 \
                DOCTEST_LOG_START();                                                               \
                doctest::detail::logAssertThrows(threw, #expr, assert_name, __FILE__, __LINE__);   \
            }                                                                                      \
            DOCTEST_GCS()->numAssertionsForCurrentTestcase++;                                      \
            if(!threw) {                                                                           \
                doctest::detail::addFailedAssert(assert_name);                                     \
                DOCTEST_BREAK_INTO_DEBUGGER();                                                     \
                doctest::detail::checkIfShouldThrow(assert_name);                                  \
            }                                                                                      \
        }                                                                                          \
    } while(doctest::detail::always_false())

#define DOCTEST_ASSERT_THROWS_AS(expr, as, assert_name)                                            \
    do {                                                                                           \
        if(!DOCTEST_GCS()->no_throw) {                                                             \
            bool threw    = false;                                                                 \
            bool threw_as = false;                                                                 \
            try {                                                                                  \
                expr;                                                                              \
            } catch(as) {                                                                          \
                threw    = true;                                                                   \
                threw_as = true;                                                                   \
            } catch(...) { threw = true; }                                                         \
            if(!threw_as || DOCTEST_GCS()->success) {                                              \
                DOCTEST_LOG_START();                                                               \
                doctest::detail::logAssertThrowsAs(threw, threw_as, #as, #expr, assert_name,       \
                                                   __FILE__, __LINE__);                            \
            }                                                                                      \
            DOCTEST_GCS()->numAssertionsForCurrentTestcase++;                                      \
            if(!threw_as) {                                                                        \
                doctest::detail::addFailedAssert(assert_name);                                     \
                DOCTEST_BREAK_INTO_DEBUGGER();                                                     \
                doctest::detail::checkIfShouldThrow(assert_name);                                  \
            }                                                                                      \
        }                                                                                          \
    } while(doctest::detail::always_false())

#define DOCTEST_ASSERT_NOTHROW(expr, assert_name)                                                  \
    do {                                                                                           \
        if(!DOCTEST_GCS()->no_throw) {                                                             \
            bool threw = false;                                                                    \
            try {                                                                                  \
                expr;                                                                              \
            } catch(...) { threw = true; }                                                         \
            if(threw || DOCTEST_GCS()->success) {                                                  \
                DOCTEST_LOG_START();                                                               \
                doctest::detail::logAssertNothrow(threw, #expr, assert_name, __FILE__, __LINE__);  \
            }                                                                                      \
            DOCTEST_GCS()->numAssertionsForCurrentTestcase++;                                      \
            if(threw) {                                                                            \
                doctest::detail::addFailedAssert(assert_name);                                     \
                DOCTEST_BREAK_INTO_DEBUGGER();                                                     \
                doctest::detail::checkIfShouldThrow(assert_name);                                  \
            }                                                                                      \
        }                                                                                          \
    } while(doctest::detail::always_false())

#define DOCTEST_WARN_THROWS(expr) DOCTEST_ASSERT_THROWS(expr, "WARN_THROWS")
#define DOCTEST_CHECK_THROWS(expr) DOCTEST_ASSERT_THROWS(expr, "CHECK_THROWS")
#define DOCTEST_REQUIRE_THROWS(expr) DOCTEST_ASSERT_THROWS(expr, "REQUIRE_THROWS")

#define DOCTEST_WARN_THROWS_AS(expr, ex) DOCTEST_ASSERT_THROWS_AS(expr, ex, "WARN_THROWS_AS")
#define DOCTEST_CHECK_THROWS_AS(expr, ex) DOCTEST_ASSERT_THROWS_AS(expr, ex, "CHECK_THROWS_AS")
#define DOCTEST_REQUIRE_THROWS_AS(expr, ex) DOCTEST_ASSERT_THROWS_AS(expr, ex, "REQUIRE_THROWS_AS")

#define DOCTEST_WARN_NOTHROW(expr) DOCTEST_ASSERT_NOTHROW(expr, "WARN_NOTHROW")
#define DOCTEST_CHECK_NOTHROW(expr) DOCTEST_ASSERT_NOTHROW(expr, "CHECK_NOTHROW")
#define DOCTEST_REQUIRE_NOTHROW(expr) DOCTEST_ASSERT_NOTHROW(expr, "REQUIRE_NOTHROW")

  // =================================================================================================
  // == WHAT FOLLOWS IS VERSIONS OF THE MACROS THAT DO NOT DO ANY REGISTERING!                      ==
  // == THIS CAN BE ENABLED BY DEFINING DOCTEST_CONFIG_DISABLE GLOBALLY!                            ==
  // =================================================================================================
#else // DOCTEST_CONFIG_DISABLE

#define DOCTEST_IMPLEMENT_FIXTURE(der, base, func, name)                                           \
    namespace                                                                                      \
    {                                                                                              \
        template <typename T>                                                                      \
        struct der : base                                                                          \
        { void f(); };                                                                             \
    }                                                                                              \
    template <typename T>                                                                          \
    inline void der<T>::f()

#define DOCTEST_CREATE_AND_REGISTER_FUNCTION(f, name)                                              \
    template <typename T>                                                                          \
    static inline void f()

  // for registering tests
#define DOCTEST_TEST_CASE(name)                                                                    \
    DOCTEST_CREATE_AND_REGISTER_FUNCTION(DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FUNC_), name)

  // for registering tests with a fixture
#define DOCTEST_TEST_CASE_FIXTURE(x, name)                                                         \
    DOCTEST_IMPLEMENT_FIXTURE(DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_CLASS_), x,                        \
                              DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FUNC_), name)

  // for subcases
#define DOCTEST_SUBCASE(name)

  // for starting a testsuite block
#define DOCTEST_TEST_SUITE(name) void DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_FOR_SEMICOLON_)()

  // for ending a testsuite block
#define DOCTEST_TEST_SUITE_END void DOCTEST_ANONYMOUS(DOCTEST_AUTOGEN_TESTSUITE_END_)

#define DOCTEST_WARN(expr) ((void)0)
#define DOCTEST_WARN_FALSE(expr) ((void)0)
#define DOCTEST_WARN_THROWS(expr) ((void)0)
#define DOCTEST_WARN_THROWS_AS(expr, ex) ((void)0)
#define DOCTEST_WARN_NOTHROW(expr) ((void)0)
#define DOCTEST_CHECK(expr) ((void)0)
#define DOCTEST_CHECK_FALSE(expr) ((void)0)
#define DOCTEST_CHECK_THROWS(expr) ((void)0)
#define DOCTEST_CHECK_THROWS_AS(expr, ex) ((void)0)
#define DOCTEST_CHECK_NOTHROW(expr) ((void)0)
#define DOCTEST_REQUIRE(expr) ((void)0)
#define DOCTEST_REQUIRE_FALSE(expr) ((void)0)
#define DOCTEST_REQUIRE_THROWS(expr) ((void)0)
#define DOCTEST_REQUIRE_THROWS_AS(expr, ex) ((void)0)
#define DOCTEST_REQUIRE_NOTHROW(expr) ((void)0)

#endif // DOCTEST_CONFIG_DISABLE

  // BDD style macros
#define DOCTEST_SCENARIO(name) TEST_CASE("Scenario: " name)
#define DOCTEST_GIVEN(name) SUBCASE("   Given: " name)
#define DOCTEST_WHEN(name) SUBCASE("    When: " name)
#define DOCTEST_AND_WHEN(name) SUBCASE("And when: " name)
#define DOCTEST_THEN(name) SUBCASE("    Then: " name)
#define DOCTEST_AND_THEN(name) SUBCASE("     And: " name)

  // == SHORT VERSIONS OF THE MACROS
#if !defined(DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)

#define TEST_CASE DOCTEST_TEST_CASE
#define TEST_CASE_FIXTURE DOCTEST_TEST_CASE_FIXTURE
#define SUBCASE DOCTEST_SUBCASE
#define TEST_SUITE DOCTEST_TEST_SUITE
#define TEST_SUITE_END DOCTEST_TEST_SUITE_END
#define WARN DOCTEST_WARN
#define WARN_FALSE DOCTEST_WARN_FALSE
#define WARN_THROWS DOCTEST_WARN_THROWS
#define WARN_THROWS_AS DOCTEST_WARN_THROWS_AS
#define WARN_NOTHROW DOCTEST_WARN_NOTHROW
#define CHECK DOCTEST_CHECK
#define CHECK_FALSE DOCTEST_CHECK_FALSE
#define CHECK_THROWS DOCTEST_CHECK_THROWS
#define CHECK_THROWS_AS DOCTEST_CHECK_THROWS_AS
#define CHECK_NOTHROW DOCTEST_CHECK_NOTHROW
#define REQUIRE DOCTEST_REQUIRE
#define REQUIRE_FALSE DOCTEST_REQUIRE_FALSE
#define REQUIRE_THROWS DOCTEST_REQUIRE_THROWS
#define REQUIRE_THROWS_AS DOCTEST_REQUIRE_THROWS_AS
#define REQUIRE_NOTHROW DOCTEST_REQUIRE_NOTHROW

#define SCENARIO DOCTEST_SCENARIO
#define GIVEN DOCTEST_GIVEN
#define WHEN DOCTEST_WHEN
#define AND_WHEN DOCTEST_AND_WHEN
#define THEN DOCTEST_THEN
#define AND_THEN DOCTEST_AND_THEN

#endif // DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES

  // this is here to clear the 'current test suite' for the current translation unit - at the top
DOCTEST_TEST_SUITE_END();

#endif // DOCTEST_LIBRARY_INCLUDED

// =================================================================================================
// == WHAT FOLLOWS IS THE IMPLEMENTATION OF THE TEST RUNNER                                       ==
// =================================================================================================
#if(defined(DOCTEST_CONFIG_IMPLEMENT) || defined(DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN))
#ifndef DOCTEST_LIBRARY_IMPLEMENTATION
#define DOCTEST_LIBRARY_IMPLEMENTATION

// required includes - will go only in one translation unit!
#include <ctime>
#include <cmath>
#include <new>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>
#include <iomanip>

namespace doctest
{
	namespace detail
	{
		// not using std::strlen() because of valgrind errors when optimizations are turned on
		// 'Invalid read of size 4' when the test suite len (with '\0') is not a multiple of 4
		// for details see http://stackoverflow.com/questions/35671155
		size_t my_strlen(const char* in) {
			const char* temp = in;
			while (*temp)
				++temp;
			return temp - in;
		}

		template <typename T>
		T my_max(const T& lhs, const T& rhs) {
			return lhs > rhs ? lhs : rhs;
		}

		// case insensitive strcmp
		int stricmp(char const* a, char const* b) {
			for (;; a++, b++) {
				int d = tolower(*a) - tolower(*b);
				if (d != 0 || !*a)
					return d;
			}
		}

		template <typename T>
		String fpToString(T value, int precision) {
			std::ostringstream oss;
			oss << std::setprecision(precision) << std::fixed << value;
			std::string d = oss.str();
			size_t      i = d.find_last_not_of('0');
			if (i != std::string::npos && i != d.size() - 1) {
				if (d[i] == '.')
					i++;
				d = d.substr(0, i + 1);
			}
			return d.c_str();
		}

		struct Endianness
		{
			enum Arch
			{
				Big,
				Little
			};

			static Arch which() {
				union _
				{
					int  asInt;
					char asChar[sizeof(int)];
				} u;

				u.asInt = 1;
				return (u.asChar[sizeof(int) - 1] == 1) ? Big : Little;
			}
		};

		String rawMemoryToString(const void* object, unsigned size) {
			// Reverse order for little endian architectures
			int i = 0, end = static_cast<int>(size), inc = 1;
			if (Endianness::which() == Endianness::Little) {
				i = end - 1;
				end = inc = -1;
			}

			unsigned char const* bytes = static_cast<unsigned char const*>(object);
			std::ostringstream   os;
			os << "0x" << std::setfill('0') << std::hex;
			for (; i != end; i += inc)
				os << std::setw(2) << static_cast<unsigned>(bytes[i]);
			return os.str().c_str();
		}

		std::ostream* createStream() { return new std::ostringstream(); }
		String getStreamResult(std::ostream* in) {
			return static_cast<std::ostringstream*>(in)->str().c_str();
		}
		void freeStream(std::ostream* in) { delete in; }

#ifndef DOCTEST_CONFIG_DISABLE
		template <class T>
		class Vector
		{
			unsigned m_size;
			unsigned m_capacity;
			T*       m_buffer;

		public:
			Vector();
			Vector(unsigned num, const T& val = T());
			Vector(const Vector& other);
			~Vector();
			Vector& operator=(const Vector& other);

			T*       data() { return m_buffer; }
			const T* data() const { return m_buffer; }
			unsigned size() const { return m_size; }

			T& operator[](unsigned index) { return m_buffer[index]; }
			const T& operator[](unsigned index) const { return m_buffer[index]; }

			void clear();
			void pop_back();
			void push_back(const T& item);
			void resize(unsigned num, const T& val = T());
		};

		// the default Hash() implementation that the HashTable class uses - returns 0 - very naive
		// specialize for better HashTable performance
		template <typename T>
		unsigned Hash(const T&) {
			return 0;
		}

		template <class T>
		class HashTable
		{
			Vector<Vector<T> > buckets;

		public:
			explicit HashTable(unsigned num_buckets)
				: buckets(num_buckets) {}

			bool has(const T& in) const {
				const Vector<T>& bucket = buckets[Hash(in) % buckets.size()];
				for (unsigned i = 0; i < bucket.size(); ++i)
					if (bucket[i] == in)
						return true;
				return false;
			}

			void insert(const T& in) {
				if (!has(in))
					buckets[Hash(in) % buckets.size()].push_back(in);
			}

			void clear() {
				for (unsigned i = 0; i < buckets.size(); ++i)
					buckets[i].clear();
			}

			const Vector<Vector<T> >& getBuckets() const { return buckets; }
		};

		// this holds both parameters for the command line and runtime data for tests
		struct ContextState : TestAccessibleContextState
		{
			// == parameters from the command line

			detail::Vector<detail::Vector<String> > filters;

			String   order_by;  // how tests should be ordered
			unsigned rand_seed; // the seed for rand ordering

			unsigned first; // the first (matching) test to be executed
			unsigned last;  // the last (matching) test to be executed

			int  abort_after;    // stop tests after this many failed assertions
			bool case_sensitive; // if filtering should be case sensitive
			bool exit;           // if the program should be exited after the tests are ran/whatever
			bool no_overrides;   // to disable overrides from code
			bool no_exitcode;    // if the framework should return 0 as the exitcode
			bool no_run;         // to not run the tests at all (can be done with an "*" exclude)
			bool no_colors;      // if output to the console should be colorized
			bool no_path_in_filenames; // if the path to files should be removed from the output

			bool help;                 // to print the help
			bool version;              // to print the version
			bool count;                // if only the count of matching tests is to be retreived
			bool list_test_cases;      // to list all tests matching the filters
			bool list_test_suites;     // to list all suites matching the filters
			bool hash_table_histogram; // if the hash table should be printed as a histogram

									   // == data for the tests being ran

			int numAssertions;
			int numFailedAssertions;
			int numFailedAssertionsForCurrentTestcase;

			// stuff for subcases
			HashTable<Subcase> subcasesPassed;
			HashTable<int>     subcasesEnteredLevels;
			Vector<Subcase>    subcasesStack;
			int                subcasesCurrentLevel;
			bool               subcasesHasSkipped;

			void resetRunData() {
				numAssertions = 0;
				numFailedAssertions = 0;
			}

			ContextState()
				: filters(6) // 6 different filters total
				, subcasesPassed(100)
				, subcasesEnteredLevels(100) {
				resetRunData();
			}
		};

		ContextState*& getContextState();
#endif
	} // namespace detail

	String::String(const char* in) {
		m_str = static_cast<char*>(malloc(detail::my_strlen(in) + 1));
		strcpy(m_str, in);
	}

	String::String(const String& other)
		: m_str(0) {
		copy(other);
	}

	void String::copy(const String& other) {
		if (m_str)
			free(m_str);
		m_str = 0;

		if (other.m_str) {
			m_str = static_cast<char*>(malloc(detail::my_strlen(other.m_str) + 1));
			strcpy(m_str, other.m_str);
		}
	}

	String::~String() {
		if (m_str)
			free(m_str);
	}

	String& String::operator=(const String& other) {
		if (this != &other)
			copy(other);
		return *this;
	}

	String String::operator+(const String& other) const { return String(m_str) += other; }

	String& String::operator+=(const String& other) {
		using namespace detail;
		if (m_str == 0) {
			copy(other);
		}
		else if (other.m_str != 0) {
			char* newStr = static_cast<char*>(malloc(my_strlen(m_str) + my_strlen(other.m_str) + 1));
			strcpy(newStr, m_str);
			strcpy(newStr + my_strlen(m_str), other.m_str);
			free(m_str);
			m_str = newStr;
		}
		return *this;
	}

	unsigned String::size() const { return m_str ? detail::my_strlen(m_str) : 0; }
	unsigned String::length() const { return size(); }

	int String::compare(const char* other, bool no_case) const {
		if (no_case)
			return detail::stricmp(m_str, other);
		return strcmp(m_str, other);
	}

	int String::compare(const String& other, bool no_case) const {
		if (no_case)
			return detail::stricmp(m_str, other.m_str);
		return strcmp(m_str, other.m_str);
	}

	std::ostream& operator<<(std::ostream& stream, const String& in) {
		stream << in.c_str();
		return stream;
	}

	Approx::Approx(double value)
		: m_epsilon(static_cast<double>(std::numeric_limits<float>::epsilon()) * 100)
		, m_scale(1.0)
		, m_value(value) {}

	bool operator==(double lhs, Approx const& rhs) {
		// Thanks to Richard Harris for his help refining this formula
		return fabs(lhs - rhs.m_value) <
			rhs.m_epsilon * (rhs.m_scale + detail::my_max(fabs(lhs), fabs(rhs.m_value)));
	}

	String Approx::toString() const { return String("Approx( ") + doctest::toString(m_value) + " )"; }

	String toString(const char* in) { return String("\"") + (in ? in : "{null string}") + "\""; }
	String toString(bool in) { return in ? "true" : "false"; }
	String toString(float in) { return detail::fpToString(in, 5) + "f"; }
	String toString(double in) { return detail::fpToString(in, 10); }
	String toString(double long in) { return detail::fpToString(in, 15); }

	String toString(char in) {
		char buf[64];
		if (in < ' ')
			sprintf(buf, "%d", in);
		else
			sprintf(buf, "%c", in);
		return buf;
	}

	String toString(char unsigned in) {
		char buf[64];
		if (in < ' ')
			sprintf(buf, "%ud", in);
		else
			sprintf(buf, "%c", in);
		return buf;
	}

	String toString(int short in) {
		char buf[64];
		sprintf(buf, "%d", in);
		return buf;
	}

	String toString(int short unsigned in) {
		char buf[64];
		sprintf(buf, "%u", in);
		return buf;
	}

	String toString(int in) {
		char buf[64];
		sprintf(buf, "%d", in);
		return buf;
	}

	String toString(int unsigned in) {
		char buf[64];
		sprintf(buf, "%u", in);
		return buf;
	}

	String toString(int long in) {
		char buf[64];
		sprintf(buf, "%ld", in);
		return buf;
	}

	String toString(int long unsigned in) {
		char buf[64];
		sprintf(buf, "%lu", in);
		return buf;
	}

#ifdef DOCTEST_CONFIG_WITH_LONG_LONG
	String toString(int long long in) {
		char buf[64];
		sprintf(buf, "%lld", in);
		return buf;
	}
	String toString(int long long unsigned in) {
		char buf[64];
		sprintf(buf, "%llu", in);
		return buf;
	}
#endif // DOCTEST_CONFIG_WITH_LONG_LONG

} // namespace doctest

#if defined(DOCTEST_CONFIG_DISABLE)
namespace doctest
{
	Context::Context(int, const char* const*) {}
	Context::~Context() {}
	void Context::addFilter(const char*, const char*) {}
	void Context::setOption(const char*, int) {}
	void Context::setOption(const char*, const char*) {}
	bool Context::shouldExit() { return false; }
	int  Context::run() { return 0; }
} // namespace doctest
#else // DOCTEST_CONFIG_DISABLE

#if !defined(DOCTEST_CONFIG_COLORS_NONE)
#if !defined(DOCTEST_CONFIG_COLORS_WINDOWS) && !defined(DOCTEST_CONFIG_COLORS_ANSI)
#ifdef DOCTEST_PLATFORM_WINDOWS
#define DOCTEST_CONFIG_COLORS_WINDOWS
#else // linux
#define DOCTEST_CONFIG_COLORS_ANSI
#endif // platform
#endif // DOCTEST_CONFIG_COLORS_WINDOWS && DOCTEST_CONFIG_COLORS_ANSI
#endif // DOCTEST_CONFIG_COLORS_NONE

#define DOCTEST_PRINTF_COLORED(buffer, color)                                                      \
    do {                                                                                           \
        if(buffer[0] != 0) {                                                                       \
            doctest::detail::Color col(color);                                                     \
            printf("%s", buffer);                                                                  \
        }                                                                                          \
    } while(doctest::detail::always_false())

  // the number of buckets used for the hash set
#if !defined(DOCTEST_HASH_TABLE_NUM_BUCKETS)
#define DOCTEST_HASH_TABLE_NUM_BUCKETS 1024
#endif // DOCTEST_HASH_TABLE_NUM_BUCKETS

  // the buffer size used for snprintf() calls
#if !defined(DOCTEST_SNPRINTF_BUFFER_LENGTH)
#define DOCTEST_SNPRINTF_BUFFER_LENGTH 1024
#endif // DOCTEST_SNPRINTF_BUFFER_LENGTH

#if defined(_MSC_VER) || defined(__MINGW32__)
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
#endif // DOCTEST_PLATFORM_WINDOWS

#ifdef DOCTEST_CONFIG_COLORS_ANSI
#include <unistd.h>
#endif // DOCTEST_CONFIG_COLORS_ANSI

#ifdef DOCTEST_CONFIG_COLORS_WINDOWS

// defines for a leaner windows.h
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif // WIN32_MEAN_AND_LEAN
#ifndef VC_EXTRA_LEAN
#define VC_EXTRA_LEAN
#endif // VC_EXTRA_LEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

// not sure what AfxWin.h is for - here I do what Catch does
#ifdef __AFXDLL
#include <AfxWin.h>
#else
#include <windows.h>
#endif

#endif // DOCTEST_CONFIG_COLORS_WINDOWS

namespace doctest
{
	namespace detail
	{
		bool TestData::operator==(const TestData& other) const {
			return m_line == other.m_line && strcmp(m_file, other.m_file) == 0;
		}

		void checkIfShouldThrow(const char* assert_name) {
			if (strncmp(assert_name, "REQUIRE", 7) == 0)
				throwException();

			if (strncmp(assert_name, "CHECK", 5) == 0 && getContextState()->abort_after > 0) {
				if (getContextState()->numFailedAssertions >= getContextState()->abort_after)
					throwException();
			}
		}
		void  throwException() { throw doctest::detail::TestFailureException(); }
		bool  always_false() { return false; }
		void* getNullPtr() { return 0; }

		// lowers ascii letters
		char tolower(const char c) { return ((c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c); }

		// matching of a string against a wildcard mask (case sensitivity configurable) taken from
		// http://www.emoticode.net/c/simple-wildcard-string-compare-globbing-function.html
		int wildcmp(const char* str, const char* wild, bool caseSensitive) {
			const char* cp = 0;
			const char* mp = 0;

			// rolled my own tolower() to not include more headers
			while ((*str) && (*wild != '*')) {
				if ((caseSensitive ? (*wild != *str) : (tolower(*wild) != tolower(*str))) &&
					(*wild != '?')) {
					return 0;
				}
				wild++;
				str++;
			}

			while (*str) {
				if (*wild == '*') {
					if (!*++wild) {
						return 1;
					}
					mp = wild;
					cp = str + 1;
				}
				else if ((caseSensitive ? (*wild == *str) : (tolower(*wild) == tolower(*str))) ||
					(*wild == '?')) {
					wild++;
					str++;
				}
				else {
					wild = mp;
					str = cp++;
				}
			}

			while (*wild == '*') {
				wild++;
			}
			return !*wild;
		}

		// C string hash function (djb2) - taken from http://www.cse.yorku.ca/~oz/hash.html
		unsigned hashStr(unsigned const char* str) {
			unsigned long hash = 5381;
			char          c;

			while ((c = *str++))
				hash = ((hash << 5) + hash) + c; // hash * 33 + c

			return hash;
		}

		// checks if the name matches any of the filters (and can be configured what to do when empty)
		int matchesAny(const char* name, Vector<String> filters, int matchEmpty, bool caseSensitive) {
			if (filters.size() == 0 && matchEmpty)
				return 1;
			for (unsigned i = 0; i < filters.size(); ++i)
				if (wildcmp(name, filters[i].c_str(), caseSensitive))
					return 1;
			return 0;
		}

		template <class T>
		Vector<T>::Vector()
			: m_size(0)
			, m_capacity(0)
			, m_buffer(0) {}

		template <class T>
		Vector<T>::Vector(unsigned num, const T& val)
			: m_size(num)
			, m_capacity(num)
			, m_buffer(static_cast<T*>(malloc(sizeof(T) * m_capacity))) {
			for (unsigned i = 0; i < m_size; ++i)
				new(m_buffer + i) T(val);
		}

		template <class T>
		Vector<T>::Vector(const Vector& other)
			: m_size(other.m_size)
			, m_capacity(other.m_capacity)
			, m_buffer(static_cast<T*>(malloc(sizeof(T) * m_capacity))) {
			for (unsigned i = 0; i < m_size; ++i)
				new(m_buffer + i) T(other.m_buffer[i]);
		}

		template <class T>
		Vector<T>::~Vector() {
			for (unsigned i = 0; i < m_size; ++i)
				(*(m_buffer + i)).~T();
			free(m_buffer);
		}

		template <class T>
		Vector<T>& Vector<T>::operator=(const Vector& other) {
			if (this != &other) {
				for (unsigned i = 0; i < m_size; ++i)
					(*(m_buffer + i)).~T();
				free(m_buffer);

				m_size = other.m_size;
				m_capacity = other.m_capacity;

				m_buffer = static_cast<T*>(malloc(sizeof(T) * m_capacity));
				for (unsigned i = 0; i < m_size; ++i)
					new(m_buffer + i) T(other.m_buffer[i]);
			}
			return *this;
		}

		template <class T>
		void Vector<T>::clear() {
			for (unsigned i = 0; i < m_size; ++i)
				(*(m_buffer + i)).~T();
			m_size = 0;
		}

		template <class T>
		void Vector<T>::pop_back() {
			if (m_size > 0)
				(*(m_buffer + --m_size)).~T();
		}

		template <class T>
		void Vector<T>::push_back(const T& item) {
			if (m_size < m_capacity) {
				new(m_buffer + m_size++) T(item);
			}
			else {
				if (m_capacity == 0)
					m_capacity = 5; // initial capacity
				else
					m_capacity *= 2; // capacity growth factor
				T* temp = static_cast<T*>(malloc(sizeof(T) * m_capacity));
				for (unsigned i = 0; i < m_size; ++i) {
					new(temp + i) T(m_buffer[i]);
					(*(m_buffer + i)).~T();
				}
				new(temp + m_size++) T(item);
				free(m_buffer);
				m_buffer = temp;
			}
		}

		template <class T>
		void Vector<T>::resize(unsigned num, const T& val) {
			if (num < m_size) {
				for (unsigned i = num; i < m_size; ++i)
					(*(m_buffer + i)).~T();
				m_size = num;
			}
			else {
				if (num > m_capacity) {
					if (m_capacity == 0) {
						m_buffer = static_cast<T*>(malloc(sizeof(T) * num));
					}
					else {
						T* temp = static_cast<T*>(malloc(sizeof(T) * num));
						for (unsigned i = 0; i < m_size; ++i) {
							new(temp + i) T(m_buffer[i]);
							(*(m_buffer + i)).~T();
						}
					}

					for (unsigned i = m_size; i < num; ++i)
						new(m_buffer + i) T(val);

					m_size = num;
					m_capacity = num;
				}
			}
		}

		template <>
		unsigned Hash(const Subcase& in) {
			return hashStr(reinterpret_cast<unsigned const char*>(in.m_file)) ^ in.m_line;
		}

		template <>
		unsigned Hash(const String& in) {
			return hashStr(reinterpret_cast<unsigned const char*>(in.c_str()));
		}

		template <>
		unsigned Hash(const int& in) {
			return in;
		}

		// the current ContextState with which tests are being executed
		ContextState*& getContextState() {
			static ContextState* data = 0;
			return data;
		}

		TestAccessibleContextState* getTestsContextState() { return getContextState(); }

		Subcase::Subcase(const char* name, const char* file, int line)
			: m_name(name)
			, m_file(file)
			, m_line(line)
			, m_entered(false) {
			ContextState* s = getContextState();

			// if we have already completed it
			if (s->subcasesPassed.has(*this))
				return;

			// if a Subcase on the same level has already been entered
			if (s->subcasesEnteredLevels.has(s->subcasesCurrentLevel)) {
				s->subcasesHasSkipped = true;
				return;
			}

			s->subcasesStack.push_back(*this);
			s->hasLoggedCurrentTestStart = false;
			if (s->hasLoggedCurrentTestStart)
				logTestEnd();

			s->subcasesEnteredLevels.insert(s->subcasesCurrentLevel++);
			m_entered = true;
		}

		Subcase::~Subcase() {
			if (m_entered) {
				ContextState* s = getContextState();

				s->subcasesCurrentLevel--;
				// only mark the subcase as passed if no subcases have been skipped
				if (s->subcasesHasSkipped == false)
					s->subcasesPassed.insert(*this);

				s->subcasesStack.pop_back();
				s->hasLoggedCurrentTestStart = false;
				if (s->hasLoggedCurrentTestStart)
					logTestEnd();
			}
		}

		Subcase::Subcase(const Subcase& other)
			: m_name(other.m_name)
			, m_file(other.m_file)
			, m_line(other.m_line)
			, m_entered(other.m_entered) {}

		Subcase& Subcase::operator=(const Subcase& other) {
			m_name = other.m_name;
			m_file = other.m_file;
			m_line = other.m_line;
			m_entered = other.m_entered;
			return *this;
		}

		bool Subcase::operator==(const Subcase& other) const {
			return m_line == other.m_line && strcmp(m_file, other.m_file) == 0;
		}

		unsigned Hash(const TestData& in) {
			return hashStr(reinterpret_cast<unsigned const char*>(in.m_file)) ^ in.m_line;
		}

		// for sorting tests by file/line
		int fileOrderComparator(const void* a, const void* b) {
			const TestData* lhs = *static_cast<TestData* const*>(a);
			const TestData* rhs = *static_cast<TestData* const*>(b);
#ifdef _MSC_VER
			// this is needed because MSVC gives different case for drive letters
			// for __FILE__ when evaluated in a header and a source file
			int res = stricmp(lhs->m_file, rhs->m_file);
#else  // _MSC_VER
			int res = strcmp(lhs->m_file, rhs->m_file);
#endif // _MSC_VER
			if (res != 0)
				return res;
			return static_cast<int>(lhs->m_line - rhs->m_line);
		}

		// for sorting tests by suite/file/line
		int suiteOrderComparator(const void* a, const void* b) {
			const TestData* lhs = *static_cast<TestData* const*>(a);
			const TestData* rhs = *static_cast<TestData* const*>(b);

			int res = strcmp(lhs->m_suite, rhs->m_suite);
			if (res != 0)
				return res;
			return fileOrderComparator(a, b);
		}

		// for sorting tests by name/suite/file/line
		int nameOrderComparator(const void* a, const void* b) {
			const TestData* lhs = *static_cast<TestData* const*>(a);
			const TestData* rhs = *static_cast<TestData* const*>(b);

			int res = strcmp(lhs->m_name, rhs->m_name);
			if (res != 0)
				return res;
			return suiteOrderComparator(a, b);
		}

		// holds the current test suite
		const char*& getCurrentTestSuite() {
			static const char* data = 0;
			return data;
		}

		// sets the current test suite
		int setTestSuiteName(const char* name) {
			getCurrentTestSuite() = name;
			return 0;
		}

		// all the registered tests
		HashTable<TestData>& getRegisteredTests() {
			static HashTable<TestData> data(DOCTEST_HASH_TABLE_NUM_BUCKETS);
			return data;
		}

		// used by the macros for registering tests
		int regTest(funcType f, unsigned line, const char* file, const char* name) {
			getRegisteredTests().insert(TestData(getCurrentTestSuite(), name, f, file, line));
			return 0;
		}

		struct Color
		{
			enum Code
			{
				None = 0,
				White,
				Red,
				Green,
				Blue,
				Cyan,
				Yellow,
				Grey,

				Bright = 0x10,

				BrightRed = Bright | Red,
				BrightGreen = Bright | Green,
				LightGrey = Bright | Grey,
				BrightWhite = Bright | White
			};
			Color(Code code) { use(code); }
			~Color() { use(None); }

			void use(Code code);

		private:
			Color(Color const& other);
		};

		void Color::use(Code
#ifndef DOCTEST_CONFIG_COLORS_NONE
			code
#endif // DOCTEST_CONFIG_COLORS_NONE
			) {
			ContextState* p = getContextState();
			if (p->no_colors)
				return;
#ifdef DOCTEST_CONFIG_COLORS_ANSI
			if (isatty(STDOUT_FILENO)) {
				const char* col = "";
				// clang-format off
				switch (code) {
				case Color::Red:         col = "[0;31m"; break;
				case Color::Green:       col = "[0;32m"; break;
				case Color::Blue:        col = "[0:34m"; break;
				case Color::Cyan:        col = "[0;36m"; break;
				case Color::Yellow:      col = "[0;33m"; break;
				case Color::Grey:        col = "[1;30m"; break;
				case Color::LightGrey:   col = "[0;37m"; break;
				case Color::BrightRed:   col = "[1;31m"; break;
				case Color::BrightGreen: col = "[1;32m"; break;
				case Color::BrightWhite: col = "[1;37m"; break;
				case Color::Bright: // invalid
				case Color::None:
				case Color::White:
				default:                 col = "[0m";
				}
				// clang-format on
				printf("\033%s", col);
			}
#endif // DOCTEST_CONFIG_COLORS_ANSI

#ifdef DOCTEST_CONFIG_COLORS_WINDOWS
			static HANDLE stdoutHandle(GetStdHandle(STD_OUTPUT_HANDLE));
			static WORD   originalForegroundAttributes;
			static WORD   originalBackgroundAttributes;
			static bool   attrsInitted = false;
			if (!attrsInitted) {
				attrsInitted = true;
				CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
				GetConsoleScreenBufferInfo(stdoutHandle, &csbiInfo);
				originalForegroundAttributes =
					csbiInfo.wAttributes &
					~(BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
				originalBackgroundAttributes =
					csbiInfo.wAttributes &
					~(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			}

#define DOCTEST_SET_ATTR(x) SetConsoleTextAttribute(stdoutHandle, x | originalBackgroundAttributes)

			// clang-format off
			switch (code) {
			case Color::White:       DOCTEST_SET_ATTR(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); break;
			case Color::Red:         DOCTEST_SET_ATTR(FOREGROUND_RED);                                      break;
			case Color::Green:       DOCTEST_SET_ATTR(FOREGROUND_GREEN);                                    break;
			case Color::Blue:        DOCTEST_SET_ATTR(FOREGROUND_BLUE);                                     break;
			case Color::Cyan:        DOCTEST_SET_ATTR(FOREGROUND_BLUE | FOREGROUND_GREEN);                  break;
			case Color::Yellow:      DOCTEST_SET_ATTR(FOREGROUND_RED | FOREGROUND_GREEN);                   break;
			case Color::Grey:        DOCTEST_SET_ATTR(0);                                                   break;
			case Color::LightGrey:   DOCTEST_SET_ATTR(FOREGROUND_INTENSITY);                                break;
			case Color::BrightRed:   DOCTEST_SET_ATTR(FOREGROUND_INTENSITY | FOREGROUND_RED);               break;
			case Color::BrightGreen: DOCTEST_SET_ATTR(FOREGROUND_INTENSITY | FOREGROUND_GREEN);             break;
			case Color::BrightWhite: DOCTEST_SET_ATTR(FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); break;
			case Color::None:
			case Color::Bright: // invalid
			default:                 DOCTEST_SET_ATTR(originalForegroundAttributes);
			}
			// clang-format on
#undef DOCTEST_SET_ATTR
#endif // DOCTEST_CONFIG_COLORS_WINDOWS
		}

		// this is needed because MSVC does not permit mixing 2 exception handling schemes in a function
		int callTestFunc(funcType f) {
			int res = EXIT_SUCCESS;
			try {
				f();
				if (getContextState()->numFailedAssertionsForCurrentTestcase)
					res = EXIT_FAILURE;
			}
			catch (const TestFailureException&) { res = EXIT_FAILURE; }
			catch (...) {
				DOCTEST_LOG_START();
				logTestCrashed();
				res = EXIT_FAILURE;
			}
			return res;
		}

		// depending on the current options this will remove the path of filenames
		const char* fileForOutput(const char* file) {
			if (getContextState()->no_path_in_filenames) {
				const char* back = strrchr(file, '\\');
				const char* forward = strrchr(file, '/');
				if (back || forward) {
					if (back > forward)
						forward = back;
					return forward + 1;
				}
			}
			return file;
		}

#ifdef DOCTEST_PLATFORM_MAC
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>
		// The following function is taken directly from the following technical note:
		// http://developer.apple.com/library/mac/#qa/qa2004/qa1361.html
		// Returns true if the current process is being debugged (either
		// running under the debugger or has a debugger attached post facto).
		bool isDebuggerActive() {
			int               mib[4];
			struct kinfo_proc info;
			size_t            size;
			// Initialize the flags so that, if sysctl fails for some bizarre
			// reason, we get a predictable result.
			info.kp_proc.p_flag = 0;
			// Initialize mib, which tells sysctl the info we want, in this case
			// we're looking for information about a specific process ID.
			mib[0] = CTL_KERN;
			mib[1] = KERN_PROC;
			mib[2] = KERN_PROC_PID;
			mib[3] = getpid();
			// Call sysctl.
			size = sizeof(info);
			if (sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, 0, 0) != 0) {
				fprintf(stderr, "\n** Call to sysctl failed - unable to determine if debugger is "
					"active **\n\n");
				return false;
			}
			// We're being debugged if the P_TRACED flag is set.
			return ((info.kp_proc.p_flag & P_TRACED) != 0);
		}
#elif defined(_MSC_VER) || defined(__MINGW32__)
		bool    isDebuggerActive() { return ::IsDebuggerPresent() != 0; }
#else
		bool isDebuggerActive() { return false; }
#endif // Platform

#ifdef DOCTEST_PLATFORM_WINDOWS
		void myOutputDebugString(const String& text) { ::OutputDebugStringA(text.c_str()); }
#else
		// TODO: integration with XCode and other IDEs
		void myOutputDebugString(const String&) {}
#endif // Platform

		const char* getSeparator() {
			return "===============================================================================\n";
		}

		void printToDebugConsole(const String& text) {
			if (isDebuggerActive())
				myOutputDebugString(text.c_str());
		}

		void addFailedAssert(const char* assert_name) {
			if (strncmp(assert_name, "WARN", 4) != 0) {
				getContextState()->numFailedAssertionsForCurrentTestcase++;
				getContextState()->numFailedAssertions++;
			}
		}

		void logTestStart(const char* name, const char* file, unsigned line) {
			const char* newLine = "\n";

			char loc[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(loc, DOCTEST_COUNTOF(loc), "%s(%d)\n", fileForOutput(file), line);

			char msg[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), "%s\n", name);

			DOCTEST_PRINTF_COLORED(getSeparator(), Color::Yellow);
			DOCTEST_PRINTF_COLORED(loc, Color::LightGrey);
			DOCTEST_PRINTF_COLORED(msg, Color::None);

			String           subcaseStuff = "";
			Vector<Subcase>& subcasesStack = getContextState()->subcasesStack;
			String           tabulation;
			for (unsigned i = 0; i < subcasesStack.size(); ++i) {
				tabulation += "  ";
				char subcase[DOCTEST_SNPRINTF_BUFFER_LENGTH];
				DOCTEST_SNPRINTF(subcase, DOCTEST_COUNTOF(loc), "%s%s\n", tabulation.c_str(),
					subcasesStack[i].m_name);
				DOCTEST_PRINTF_COLORED(subcase, Color::None);
				subcaseStuff += subcase;
			}

			DOCTEST_PRINTF_COLORED(newLine, Color::None);

			printToDebugConsole(String(getSeparator()) + loc + msg + subcaseStuff.c_str() + newLine);
		}

		void logTestEnd() {}

		void logTestCrashed() {
			char msg[DOCTEST_SNPRINTF_BUFFER_LENGTH];

			DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), "  TEST CASE FAILED! (threw exception)\n\n");

			DOCTEST_PRINTF_COLORED(msg, Color::Red);

			printToDebugConsole(String(msg));
		}

		void logAssert(bool passed, const char* decomposition, bool threw, const char* expr,
			const char* assert_name, const char* file, int line) {
			char loc[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(loc, DOCTEST_COUNTOF(loc), "%s(%d)", fileForOutput(file), line);

			char msg[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			if (passed)
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " PASSED!\n");
			else
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " FAILED! %s\n",
					(threw ? "(threw exception)" : ""));

			char info1[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(info1, DOCTEST_COUNTOF(info1), "  %s( %s )\n", assert_name, expr);

			char info2[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			char info3[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			info2[0] = 0;
			info3[0] = 0;
			if (!threw) {
				DOCTEST_SNPRINTF(info2, DOCTEST_COUNTOF(info2), "with expansion:\n");
				DOCTEST_SNPRINTF(info3, DOCTEST_COUNTOF(info3), "  %s( %s )\n", assert_name,
					decomposition);
			}

			DOCTEST_PRINTF_COLORED(loc, Color::LightGrey);
			DOCTEST_PRINTF_COLORED(msg, passed ? Color::BrightGreen : Color::Red);
			DOCTEST_PRINTF_COLORED(info1, Color::Green);
			DOCTEST_PRINTF_COLORED(info2, Color::None);
			DOCTEST_PRINTF_COLORED(info3, Color::Green);
			DOCTEST_PRINTF_COLORED("\n", Color::None);

			printToDebugConsole(String(loc) + msg + info1 + info2 + info3 + "\n");
		}

		void logAssertThrows(bool threw, const char* expr, const char* assert_name, const char* file,
			int line) {
			char loc[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(loc, DOCTEST_COUNTOF(loc), "%s(%d)", fileForOutput(file), line);

			char msg[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			if (threw)
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " PASSED!\n");
			else
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " FAILED!\n");

			char info1[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(info1, DOCTEST_COUNTOF(info1), "  %s( %s )\n\n", assert_name, expr);

			DOCTEST_PRINTF_COLORED(loc, Color::LightGrey);
			DOCTEST_PRINTF_COLORED(msg, threw ? Color::BrightGreen : Color::Red);
			DOCTEST_PRINTF_COLORED(info1, Color::Green);

			printToDebugConsole(String(loc) + msg + info1);
		}

		void logAssertThrowsAs(bool threw, bool threw_as, const char* as, const char* expr,
			const char* assert_name, const char* file, int line) {
			char loc[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(loc, DOCTEST_COUNTOF(loc), "%s(%d)", fileForOutput(file), line);

			char msg[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			if (threw_as)
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " PASSED!\n");
			else
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " FAILED! %s\n",
					(threw ? "(threw something else)" : "(didn't throw at all)"));

			char info1[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(info1, DOCTEST_COUNTOF(info1), "  %s( %s, %s )\n\n", assert_name, expr,
				as);

			DOCTEST_PRINTF_COLORED(loc, Color::LightGrey);
			DOCTEST_PRINTF_COLORED(msg, threw_as ? Color::BrightGreen : Color::Red);
			DOCTEST_PRINTF_COLORED(info1, Color::Green);

			printToDebugConsole(String(loc) + msg + info1);
		}

		void logAssertNothrow(bool threw, const char* expr, const char* assert_name, const char* file,
			int line) {
			char loc[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(loc, DOCTEST_COUNTOF(loc), "%s(%d)", fileForOutput(file), line);

			char msg[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			if (!threw)
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " PASSED!\n");
			else
				DOCTEST_SNPRINTF(msg, DOCTEST_COUNTOF(msg), " FAILED!\n");

			char info1[DOCTEST_SNPRINTF_BUFFER_LENGTH];
			DOCTEST_SNPRINTF(info1, DOCTEST_COUNTOF(info1), "  %s( %s )\n\n", assert_name, expr);

			DOCTEST_PRINTF_COLORED(loc, Color::LightGrey);
			DOCTEST_PRINTF_COLORED(msg, !threw ? Color::BrightGreen : Color::Red);
			DOCTEST_PRINTF_COLORED(info1, Color::Green);

			printToDebugConsole(String(loc) + msg + info1);
		}

		// the implementation of parseFlag()
		bool parseFlagImpl(int argc, const char* const* argv, const char* pattern) {
			for (int i = argc - 1; i >= 0; --i) {
				const char* temp = strstr(argv[i], pattern);
				if (temp && my_strlen(temp) == my_strlen(pattern)) {
					// eliminate strings in which the chars before the option are not '-'
					bool noBadCharsFound = true;
					while (temp != argv[i]) {
						if (*--temp != '-') {
							noBadCharsFound = false;
							break;
						}
					}
					if (noBadCharsFound)
						return true;
				}
			}
			return false;
		}

		// locates a flag on the command line
		bool parseFlag(int argc, const char* const* argv, const char* pattern) {
#ifndef DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
			if (!parseFlagImpl(argc, argv, pattern))
				return parseFlagImpl(argc, argv, pattern + 3); // 3 for "dt-"
			return true;
#else  // DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
			return parseFlagImpl(argc, argv, pattern);
#endif // DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
		}

		// the implementation of parseOption()
		bool parseOptionImpl(int argc, const char* const* argv, const char* pattern, String& res) {
			for (int i = argc - 1; i >= 0; --i) {
				const char* temp = strstr(argv[i], pattern);
				if (temp) {
					// eliminate matches in which the chars before the option are not '-'
					bool        noBadCharsFound = true;
					const char* curr = argv[i];
					while (curr != temp) {
						if (*curr++ != '-') {
							noBadCharsFound = false;
							break;
						}
					}
					if (noBadCharsFound) {
						temp += my_strlen(pattern);
						unsigned len = my_strlen(temp);
						if (len) {
							res = temp;
							return true;
						}
					}
				}
			}
			return false;
		}

		// parses an option and returns the string after the '=' character
		bool parseOption(int argc, const char* const* argv, const char* pattern, String& res,
			const String& defaultVal = String()) {
			res = defaultVal;
#ifndef DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
			if (!parseOptionImpl(argc, argv, pattern, res))
				return parseOptionImpl(argc, argv, pattern + 3, res); // 3 for "dt-"
			return true;
#else // DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
			return parseOptionImpl(argc, argv, pattern, res);
#endif // DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
		}

		// parses a comma separated list of words after a pattern in one of the arguments in argv
		bool parseCommaSepArgs(int argc, const char* const* argv, const char* pattern,
			Vector<String>& res) {
			String filtersString;
			if (parseOption(argc, argv, pattern, filtersString)) {
				// tokenize with "," as a separator
				char* pch = strtok(filtersString.c_str(), ","); // modifies the string
				while (pch != 0) {
					if (my_strlen(pch))
						res.push_back(pch);
					pch = strtok(0, ","); // uses the strtok() internal state to go to the next token
				}
				return true;
			}
			return false;
		}

		enum optionType
		{
			option_bool,
			option_int
		};

		// parses an int/bool option from the command line
		bool parseIntOption(int argc, const char* const* argv, const char* pattern, optionType type,
			int& res) {
			String parsedValue;
			if (parseOption(argc, argv, pattern, parsedValue)) {
				if (type == 0) {
					// boolean
					const char positive[][5] = { "1", "true", "on", "yes" };  // 5 - strlen("true") + 1
					const char negative[][6] = { "0", "false", "off", "no" }; // 6 - strlen("false") + 1

																			  // if the value matches any of the positive/negative possibilities
					for (unsigned i = 0; i < 4; i++) {
						if (parsedValue.compare(positive[i], true) == 0) {
							res = 1;
							return true;
						}
						if (parsedValue.compare(negative[i], true) == 0) {
							res = 0;
							return true;
						}
					}
				}
				else {
					// integer
					int theInt = atoi(parsedValue.c_str());
					if (theInt != 0) {
						res = theInt;
						return true;
					}
				}
			}
			return false;
		}

		void printVersion() {
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("doctest version is \"%s\"\n", DOCTEST_VERSION);
		}

		void printHelp() {
			printVersion();
			DOCTEST_PRINTF_COLORED("[doctest]\n", Color::Cyan);
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("boolean values: \"1/on/yes/true\" or \"0/off/no/false\"\n");
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("filter  values: \"str1,str2,str3\" (comma separated strings)\n");
			DOCTEST_PRINTF_COLORED("[doctest]\n", Color::Cyan);
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("filters use wildcards for matching strings\n");
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("something passes a filter if any of the strings in a filter matches\n");
			DOCTEST_PRINTF_COLORED("[doctest]\n", Color::Cyan);
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("ALL FLAGS, OPTIONS AND FILTERS ALSO AVAILABLE WITH A \"dt-\" PREFIX!!!\n");
			DOCTEST_PRINTF_COLORED("[doctest]\n", Color::Cyan);
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("Query flags - the program quits after them. Available:\n\n");
			printf(" -?,   --help, -h                      prints this message\n");
			printf(" -v,   --version                       prints the version\n");
			printf(" -c,   --count                         prints the number of matching tests\n");
			printf(" -ltc, --list-test-cases               lists all matching tests by name\n");
			printf(" -lts, --list-test-suites              lists all matching test suites\n\n");
			//printf(" -hth, --hash-table-histogram          undocumented\n");
			// ==================================================================================== << 79
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("The available <int>/<string> options/filters are:\n\n");
			printf(" -tc,  --test-case=<filters>           filters     tests by their name\n");
			printf(" -tce, --test-case-exclude=<filters>   filters OUT tests by their name\n");
			printf(" -sf,  --source-file=<filters>         filters     tests by their file\n");
			printf(" -sfe, --source-file-exclude=<filters> filters OUT tests by their file\n");
			printf(" -ts,  --test-suite=<filters>          filters     tests by their test suite\n");
			printf(" -tse, --test-suite-exclude=<filters>  filters OUT tests by their test suite\n");
			printf(" -ob,  --order-by=<string>             how the tests should be ordered\n");
			printf("                                       <string> - by [file/suite/name/rand]\n");
			printf(" -rs,  --rand-seed=<int>               seed for random ordering\n");
			printf(" -f,   --first=<int>                   the first test passing the filters to\n");
			printf("                                       execute - for range-based execution\n");
			printf(" -l,   --last=<int>                    the last test passing the filters to\n");
			printf("                                       execute - for range-based execution\n");
			printf(" -aa,  --abort-after=<int>             stop after <int> failed assertions\n\n");
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("Bool options - can be used like flags and true is assumed. Available:\n\n");
			printf(" -s,   --success=<bool>                include successful assertions in output\n");
			printf(" -cs,  --case-sensitive=<bool>         filters being treated as case sensitive\n");
			printf(" -e,   --exit=<bool>                   exits after the tests finish\n");
			printf(" -no,  --no-overrides=<bool>           disables procedural overrides of options\n");
			printf(" -nt,  --no-throw=<bool>               skips exceptions-related assert checks\n");
			printf(" -ne,  --no-exitcode=<bool>            returns (or exits) always with success\n");
			printf(" -nr,  --no-run=<bool>                 skips all runtime doctest operations\n");
			printf(" -nc,  --no-colors=<bool>              disables colors in output\n");
			printf(" -nb,  --no-breaks=<bool>              disables breakpoints in debuggers\n");
			printf(" -npf, --no-path-filenames=<bool>      only filenames and no paths in output\n\n");
			// ==================================================================================== << 79

			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("for more information visit the project documentation\n\n");
		}
	} // namespace detail

	Context::Context(int argc, const char* const* argv)
		: p(new detail::ContextState) {
		using namespace detail;

		parseArgs(argc, argv, true);

		p->help = false;
		p->version = false;
		p->count = false;
		p->list_test_cases = false;
		p->list_test_suites = false;
		p->hash_table_histogram = false;
		if (parseFlag(argc, argv, "dt-help") || parseFlag(argc, argv, "dt-h") ||
			parseFlag(argc, argv, "dt-?")) {
			p->help = true;
			p->exit = true;
		}
		if (parseFlag(argc, argv, "dt-version") || parseFlag(argc, argv, "dt-v")) {
			p->version = true;
			p->exit = true;
		}
		if (parseFlag(argc, argv, "dt-count") || parseFlag(argc, argv, "dt-c")) {
			p->count = true;
			p->exit = true;
		}
		if (parseFlag(argc, argv, "dt-list-test-cases") || parseFlag(argc, argv, "dt-ltc")) {
			p->list_test_cases = true;
			p->exit = true;
		}
		if (parseFlag(argc, argv, "dt-list-test-suites") || parseFlag(argc, argv, "dt-lts")) {
			p->list_test_suites = true;
			p->exit = true;
		}
		if (parseFlag(argc, argv, "dt-hash-table-histogram") || parseFlag(argc, argv, "dt-hth")) {
			p->hash_table_histogram = true;
			p->exit = true;
		}
	}

	Context::~Context() { delete p; }

	// parses args
	void Context::parseArgs(int argc, const char* const* argv, bool withDefaults) {
		using namespace detail;

		// clang-format off
		parseCommaSepArgs(argc, argv, "dt-source-file=", p->filters[0]);
		parseCommaSepArgs(argc, argv, "dt-sf=", p->filters[0]);
		parseCommaSepArgs(argc, argv, "dt-source-file-exclude=", p->filters[1]);
		parseCommaSepArgs(argc, argv, "dt-sfe=", p->filters[1]);
		parseCommaSepArgs(argc, argv, "dt-test-suite=", p->filters[2]);
		parseCommaSepArgs(argc, argv, "dt-ts=", p->filters[2]);
		parseCommaSepArgs(argc, argv, "dt-test-suite-exclude=", p->filters[3]);
		parseCommaSepArgs(argc, argv, "dt-tse=", p->filters[3]);
		parseCommaSepArgs(argc, argv, "dt-test-case=", p->filters[4]);
		parseCommaSepArgs(argc, argv, "dt-tc=", p->filters[4]);
		parseCommaSepArgs(argc, argv, "dt-test-case-exclude=", p->filters[5]);
		parseCommaSepArgs(argc, argv, "dt-tce=", p->filters[5]);
		// clang-format on

		int    intRes = 0;
		String strRes;

#define DOCTEST_PARSE_AS_BOOL_OR_FLAG(name, sname, var, default)                                   \
    if(parseIntOption(argc, argv, DOCTEST_STR_CONCAT_TOSTR(name, =), option_bool, intRes) ||       \
       parseIntOption(argc, argv, DOCTEST_STR_CONCAT_TOSTR(sname, =), option_bool, intRes))        \
        p->var = !!intRes;                                                                         \
    else if(parseFlag(argc, argv, #name) || parseFlag(argc, argv, #sname))                         \
        p->var = 1;                                                                                \
    else if(withDefaults)                                                                          \
    p->var = default

#define DOCTEST_PARSE_INT_OPTION(name, sname, var, default)                                        \
    if(parseIntOption(argc, argv, DOCTEST_STR_CONCAT_TOSTR(name, =), option_int, intRes) ||        \
       parseIntOption(argc, argv, DOCTEST_STR_CONCAT_TOSTR(sname, =), option_int, intRes))         \
        p->var = intRes;                                                                           \
    else if(withDefaults)                                                                          \
    p->var = default

#define DOCTEST_PARSE_STR_OPTION(name, sname, var, default)                                        \
    if(parseOption(argc, argv, DOCTEST_STR_CONCAT_TOSTR(name, =), strRes, default) ||              \
       parseOption(argc, argv, DOCTEST_STR_CONCAT_TOSTR(sname, =), strRes, default) ||             \
       withDefaults)                                                                               \
    p->var = strRes

		// clang-format off
		DOCTEST_PARSE_STR_OPTION(dt - order - by, dt - ob, order_by, "file");
		DOCTEST_PARSE_INT_OPTION(dt - rand - seed, dt - rs, rand_seed, 0);

		DOCTEST_PARSE_INT_OPTION(dt - first, dt - f, first, 1);
		DOCTEST_PARSE_INT_OPTION(dt - last, dt - l, last, 0);

		DOCTEST_PARSE_INT_OPTION(dt - abort - after, dt - aa, abort_after, 0);

		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - success, dt - s, success, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - case-sensitive, dt - cs, case_sensitive, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - exit, dt - e, exit, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - no - overrides, dt - no, no_overrides, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - no - throw, dt - nt, no_throw, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - no - exitcode, dt - ne, no_exitcode, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - no - run, dt - nr, no_run, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - no - colors, dt - nc, no_colors, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - no - breaks, dt - nb, no_breaks, 0);
		DOCTEST_PARSE_AS_BOOL_OR_FLAG(dt - no - path - filenames, dt - npf, no_path_in_filenames, 0);
		// clang-format on

#undef DOCTEST_PARSE_STR_OPTION
#undef DOCTEST_PARSE_INT_OPTION
#undef DOCTEST_PARSE_AS_BOOL_OR_FLAG
	}

	// allows the user to add procedurally to the filters from the command line
	void Context::addFilter(const char* filter, const char* value) { setOption(filter, value); }

	// allows the user to override procedurally the int/bool options from the command line
	void Context::setOption(const char* option, int value) {
		setOption(option, toString(value).c_str());
	}

	// allows the user to override procedurally the string options from the command line
	void Context::setOption(const char* option, const char* value) {
		using namespace detail;

		if (!p->no_overrides) {
			String      argv = String(option) + "=" + value;
			const char* lvalue = argv.c_str();
			parseArgs(1, &lvalue);
		}
	}

	// users should query this in their main() and exit the program if true
	bool Context::shouldExit() { return p->exit; }

	// the main function that does all the filtering and test running
	int Context::run() {
		using namespace detail;

		getContextState() = p;
		p->resetRunData();

		// handle version, help and no_run
		if (p->no_run || p->version || p->help) {
			if (p->version)
				printVersion();
			if (p->help)
				printHelp();

			return EXIT_SUCCESS;
		}

		printVersion();
		DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
		printf("run with \"-dt-help\" for options\n");

		unsigned                         i = 0; // counter used for loops - here for VC6
		const Vector<Vector<TestData> >& buckets = getRegisteredTests().getBuckets();

		Vector<const TestData*> testArray;
		for (i = 0; i < buckets.size(); i++)
			for (unsigned k = 0; k < buckets[i].size(); k++)
				testArray.push_back(&buckets[i][k]);

		if (p->hash_table_histogram) {
			// find the most full bucket
			unsigned maxInBucket = 1;
			for (i = 0; i < buckets.size(); i++)
				if (buckets[i].size() > maxInBucket)
					maxInBucket = buckets[i].size();

			// print a prettified histogram
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("hash table bucket histogram\n");
			printf("============================================================\n");
			printf("#bucket     |count| relative count\n");
			printf("============================================================\n");
			for (i = 0; i < buckets.size(); i++) {
				printf("bucket %4d |%4d |", static_cast<int>(i), buckets[i].size());

				float ratio = static_cast<float>(buckets[i].size()) / static_cast<float>(maxInBucket);
				unsigned numStars = static_cast<unsigned>(ratio * 41);
				for (unsigned k = 0; k < numStars; ++k)
					printf("*");
				printf("\n");
			}
			printf("\n");
			return EXIT_SUCCESS;
		}

		// sort the collected records
		if (p->order_by.compare("file", true) == 0) {
			qsort(testArray.data(), testArray.size(), sizeof(TestData*), fileOrderComparator);
		}
		else if (p->order_by.compare("suite", true) == 0) {
			qsort(testArray.data(), testArray.size(), sizeof(TestData*), suiteOrderComparator);
		}
		else if (p->order_by.compare("name", true) == 0) {
			qsort(testArray.data(), testArray.size(), sizeof(TestData*), nameOrderComparator);
		}
		else if (p->order_by.compare("rand", true) == 0) {
			srand(p->rand_seed);

			// random_shuffle implementation
			const TestData** first = testArray.data();
			for (i = testArray.size() - 1; i > 0; --i) {
				int idxToSwap = rand() % (i + 1);

				const TestData* temp = first[i];

				first[i] = first[idxToSwap];
				first[idxToSwap] = temp;
			}
		}

		if (p->list_test_cases) {
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("listing all test case names\n");
		}

		HashTable<String> testSuitesPassingFilters(100);
		if (p->list_test_suites) {
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("listing all test suites\n");
		}

		unsigned numTestsPassingFilters = 0;
		unsigned numFailed = 0;
		// invoke the registered functions if they match the filter criteria (or just count them)
		for (i = 0; i < testArray.size(); i++) {
			const TestData& data = *testArray[i];
			if (!matchesAny(data.m_file, p->filters[0], 1, p->case_sensitive))
				continue;
			if (matchesAny(data.m_file, p->filters[1], 0, p->case_sensitive))
				continue;
			if (!matchesAny(data.m_suite, p->filters[2], 1, p->case_sensitive))
				continue;
			if (matchesAny(data.m_suite, p->filters[3], 0, p->case_sensitive))
				continue;
			if (!matchesAny(data.m_name, p->filters[4], 1, p->case_sensitive))
				continue;
			if (matchesAny(data.m_name, p->filters[5], 0, p->case_sensitive))
				continue;

			numTestsPassingFilters++;

			// do not execute the test if we are to only count the number of filter passing tests
			if (p->count)
				continue;

			// print the name of the test and don't execute it
			if (p->list_test_cases) {
				printf("%s\n", data.m_name);
				continue;
			}

			// print the name of the test suite if not done already and don't execute it
			if (p->list_test_suites) {
				if (!testSuitesPassingFilters.has(data.m_suite)) {
					printf("%s\n", data.m_suite);
					testSuitesPassingFilters.insert(data.m_suite);
				}
				continue;
			}

			// skip the test if it is not in the execution range
			if ((p->last < numTestsPassingFilters && p->first <= p->last) ||
				(p->first > numTestsPassingFilters))
				continue;

			// execute the test if it passes all the filtering
			{
#ifdef _MSC_VER
				//__try {
#endif // _MSC_VER

				p->currentTest = &data;

				// if logging successful tests - force the start log
				p->hasLoggedCurrentTestStart = false;
				if (p->success)
					DOCTEST_LOG_START();

				unsigned didFail = 0;
				p->subcasesPassed.clear();
				do {
					// reset the assertion state
					p->numAssertionsForCurrentTestcase = 0;
					p->numFailedAssertionsForCurrentTestcase = 0;

					// reset some of the fields for subcases (except for the set of fully passed ones)
					p->subcasesHasSkipped = false;
					p->subcasesCurrentLevel = 0;
					p->subcasesEnteredLevels.clear();

					// execute the test
					didFail += callTestFunc(data.m_f);
					p->numAssertions += p->numAssertionsForCurrentTestcase;

					// exit this loop if enough assertions have failed
					if (p->abort_after > 0 && p->numFailedAssertions >= p->abort_after)
						p->subcasesHasSkipped = false;

					// if the start has been logged
					if (p->hasLoggedCurrentTestStart)
						logTestEnd();
					p->hasLoggedCurrentTestStart = false;

				} while (p->subcasesHasSkipped == true);

				if (didFail > 0)
					numFailed++;

				// stop executing tests if enough assertions have failed
				if (p->abort_after > 0 && p->numFailedAssertions >= p->abort_after)
					break;

#ifdef _MSC_VER
				//} __except(1) {
				//    printf("Unknown SEH exception caught!\n");
				//    numFailed++;
				//}
#endif // _MSC_VER
			}
		}

		DOCTEST_PRINTF_COLORED(getSeparator(), Color::Yellow);
		if (p->count || p->list_test_cases || p->list_test_suites) {
			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);
			printf("number of tests passing the current filters: %d\n", numTestsPassingFilters);
		}
		else {
			char buff[DOCTEST_SNPRINTF_BUFFER_LENGTH];

			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);

			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), "test cases: %4d", numTestsPassingFilters);
			DOCTEST_PRINTF_COLORED(buff, Color::None);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), " | ");
			DOCTEST_PRINTF_COLORED(buff, Color::None);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), "%4d passed",
				numTestsPassingFilters - numFailed);
			DOCTEST_PRINTF_COLORED(buff, Color::Green);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), " | ");
			DOCTEST_PRINTF_COLORED(buff, Color::None);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), "%4d failed\n", numFailed);
			DOCTEST_PRINTF_COLORED(buff, Color::Red);

			DOCTEST_PRINTF_COLORED("[doctest] ", Color::Cyan);

			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), "assertions: %4d", p->numAssertions);
			DOCTEST_PRINTF_COLORED(buff, Color::None);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), " | ");
			DOCTEST_PRINTF_COLORED(buff, Color::None);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), "%4d passed",
				p->numAssertions - p->numFailedAssertions);
			DOCTEST_PRINTF_COLORED(buff, Color::Green);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), " | ");
			DOCTEST_PRINTF_COLORED(buff, Color::None);
			DOCTEST_SNPRINTF(buff, DOCTEST_COUNTOF(buff), "%4d failed\n", p->numFailedAssertions);
			DOCTEST_PRINTF_COLORED(buff, Color::Red);
		}

		if (numFailed && !p->no_exitcode)
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}
} // namespace doctest

#endif // DOCTEST_CONFIG_DISABLE
#endif // DOCTEST_LIBRARY_IMPLEMENTATION
#endif // DOCTEST_CONFIG_IMPLEMENT

  // == THIS SUPPLIES A MAIN FUNCTION AND SHOULD BE DONE ONLY IN ONE TRANSLATION UNIT
#if defined(DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN) && !defined(DOCTEST_MAIN_CONFIGURED)
#define DOCTEST_MAIN_CONFIGURED
int main(int argc, char** argv) { return doctest::Context(argc, argv).run(); }
#endif // DOCTEST_MAIN_CONFIGURED

#if defined(__clang__)
#pragma clang diagnostic pop
#endif // __clang__

#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
#pragma GCC diagnostic pop
#endif // > gcc 4.6
#endif // __GNUC__

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER