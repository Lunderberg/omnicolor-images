#ifndef _SMARTENUM_H_
#define _SMARTENUM_H_

#include <istream>
#include <ostream>
#include <string>

#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>


#define SmartEnum(EnumName,...)																					\
	class EnumName{																												\
	public:																																\
	enum internal_enum {__VA_ARGS__};																			\
																																				\
	EnumName() {}																													\
	EnumName(internal_enum e) : value(e){}																\
																																				\
	friend std::istream& operator>>(std::istream& in, EnumName& e){				\
		std::string str;																										\
		in >> str;																													\
		if(false){																													\
		}																																		\
		BOOST_PP_SEQ_FOR_EACH(SMART_ENUM_UNPACK_ELSEIF,,										\
													BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))				\
		else {																															\
			throw boost::program_options::invalid_option_value								\
				("EnumName must be one of " #__VA_ARGS__);											\
		}																																		\
		return in;																													\
	}																																			\
																																				\
	std::string toString() const {																				\
		switch(value){																											\
			BOOST_PP_SEQ_FOR_EACH(SMART_ENUM_STRING_CASE,,										\
														BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))			\
				}																																\
	}																																			\
																																				\
	friend std::ostream& operator<<(std::ostream& out, const EnumName& e){ \
		out << e.toString();																								\
		return out;																													\
	}																																			\
																																				\
	operator int(){return value;}																					\
																																				\
	private:																															\
	internal_enum value;																									\
	};

#define SMART_ENUM_UNPACK_ELSEIF(_,__,name)			\
	else if(str==BOOST_PP_STRINGIZE(name)){				\
		e = name;																		\
	}

#define SMART_ENUM_STRING_CASE(_,__,name)				\
	case name:																		\
	return BOOST_PP_STRINGIZE(name);

#endif /* _SMARTENUM_H_ */
