/*
 * expected.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Christopher Nelson
 */

#ifndef __LATTICE_EXPECTED_H_
#define __LATTICE_EXPECTED_H_

#include <exception>

namespace lattice {

/**
 * Holds either a value of the expected type, or an exception. If the valid()
 * call returns true, the value is not an error. If you do not check the value
 * and the expected() object holds an error, on destruction it will throw the
 * error.
 *
 * To avoid automatically throwing, check the value.
 *
 */
template<class T> class expected {
	union {
		T value;
		std::exception_ptr error;
	};
	bool got_value;
	bool checked_value;
	expected() :
			got_value(false), checked_value(false) {
	} // used internally
public:

	expected(const T& rhs) :
			value(rhs), got_value(true), checked_value(false) {
	}

	expected(T&& rhs): value(std::move(rhs)),
	got_value(true), checked_value(false) {}

	expected(const expected& rhs) : got_value(rhs.got_value),
	checked_value(rhs.checked_value) {
		if (got_value) new(&value) T(rhs.value);
		else new(&error) std::exception_ptr(rhs.error);
	}

	expected(expected&& rhs) : got_value(rhs.got_value),
	checked_value(rhs.checked_value) {
		rhs.checked_value = true;
		if (got_value) new(&value) T(std::move(rhs.value));
		else new(&error)
		std::exception_ptr(std::move(rhs.error));
	}

	~expected() {
		if (!got_value && !checked_value) {
			std::rethrow_exception(error);
		}
	}

	template <class E>
	static expected<T> from_exception(const E& exception) {
		if (typeid(exception) != typeid(E)) {
			throw std::invalid_argument(
					"slicing detected");
		}
		return from_exception(
				std::make_exception_ptr(exception));
	}

	static expected<T> from_exception(std::exception_ptr p) {
		expected<T> result;
		result.got_value = false;
		new(&result.error) std::exception_ptr(std::move(p));
		return result;
	}

	static expected<T> from_exception() {
		return from_exception(std::current_exception());
	}

	template <class F>
	static expected from_code(F fun) {
		try {
			return expected(fun());
		} catch (...) {
			return from_exception();
		}
	}

	/** Returns true if the expected value type was returned, false
	 * if this object holds an error. Calling this function will also
	 * keep this object from throwing the held error on destruction.
	 */
	bool valid() {
		checked_value = true;
		return got_value;
	}

	/** Returns the expected value type if there is one, or throws
	 * the teleported exception if not. Calling this function will
	 * keep this object from throwing the held error on destruction.
	 */
	T& get() {
		if (!valid()) std::rethrow_exception(error);
		return value;
	}
}
;

} // end namespace lattice

#endif /* __LATTICE_EXPECTED_H_ */
