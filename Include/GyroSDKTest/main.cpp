#include "NGS/NGS.h"

template<typename Child>
struct Base {
	bool operator==(const Child& child)const { return false; }
	operator Child& () { return *reinterpret_cast<Child*>(this); }
	operator const Child& ()const { return *reinterpret_cast<Child*>(this); }
};

struct Child : public Base<Child> {
	using Base<Child>::operator==;
};

int main2() {
	Child child;
	child != (Child());
	return 0;
}
