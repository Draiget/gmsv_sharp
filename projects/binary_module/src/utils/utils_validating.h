#ifndef ZONTWELG_GM_SHARP_VALIDATING_H
#define ZONTWELG_GM_SHARP_VALIDATING_H

template< class T >
static bool IsValidPtr(T ptr);

template < class T >
bool IsValidPtr(T ptr) {
	const auto pointer = reinterpret_cast<unsigned long>(ptr);
	return pointer && pointer != 0xcccccccc;
}

#endif