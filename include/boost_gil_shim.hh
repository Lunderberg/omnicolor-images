#ifndef _BOOST_GIL_SHIM_H_
#define _BOOST_GIL_SHIM_H_

#include <boost/version.hpp>

#if (BOOST_VERSION < 106800)
  #include <boost/gil/gil_all.hpp>
#else
  #include <boost/gil.hpp>
#endif

#endif /* _BOOST_GIL_SHIM_H_ */
