#pragma once
#include <fjson/string.hpp>
#include <fjson/time.hpp>
#include <fjson/shared_ptr.hpp>
#include <fjson/log/log_message.hpp>

namespace fjson  
{

   class appender;

   /**
    *
    *
    @code
      void my_class::func() 
      {
         FJSON_dlog( my_class_logger, "Format four: ${arg}  five: ${five}", ("arg",4)("five",5) );
      }
    @endcode
    */
   class logger 
   {
      public:
         static logger get( const fjson::string& name = "default");

         logger();
         logger( const string& name, const logger& parent = nullptr );
         logger( std::nullptr_t );
         logger( const logger& c );
         logger( logger&& c );
         ~logger();
         logger& operator=(const logger&);
         logger& operator=(logger&&);
         friend bool operator==( const logger&, nullptr_t );
         friend bool operator!=( const logger&, nullptr_t );

         logger&    set_log_level( log_level e );
         log_level  get_log_level()const;
         logger&    set_parent( const logger& l );
         logger     get_parent()const;

         void  set_name( const fjson::string& n );
         const fjson::string& name()const;

         void add_appender( const fjson::shared_ptr<appender>& a );
         std::vector<fjson::shared_ptr<appender> > get_appenders()const;
         void remove_appender( const fjson::shared_ptr<appender>& a );

         bool is_enabled( log_level e )const;
         void log( log_message m );

      private:
         class impl;
		 fjson::shared_ptr<impl> my;
   };

} // namespace fjson

#ifndef DEFAULT_LOGGER
#define DEFAULT_LOGGER
#endif

// suppress warning "conditional expression is constant" in the while(0) for visual c++
// http://cnicholson.net/2009/03/stupid-c-tricks-dowhile0-and-c4127/
#define FJSON_MULTILINE_MACRO_BEGIN do {
#ifdef _MSC_VER
# define FJSON_MULTILINE_MACRO_END \
    __pragma(warning(push)) \
    __pragma(warning(disable:4127)) \
    } while (0) \
    __pragma(warning(pop))
#else
# define FJSON_MULTILINE_MACRO_END  } while (0)
#endif

#define FJSON_dlog( LOGGER, FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fjson::log_level::debug ) ) \
      (LOGGER).log( FJSON_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

#define FJSON_ilog( LOGGER, FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fjson::log_level::info ) ) \
      (LOGGER).log( FJSON_LOG_MESSAGE( info, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

#define FJSON_wlog( LOGGER, FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fjson::log_level::warn ) ) \
      (LOGGER).log( FJSON_LOG_MESSAGE( warn, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

#define FJSON_elog( LOGGER, FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fjson::log_level::error ) ) \
      (LOGGER).log( FJSON_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

#define dlog( FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (fjson::logger::get(DEFAULT_LOGGER)).is_enabled( fjson::log_level::debug ) ) \
      (fjson::logger::get(DEFAULT_LOGGER)).log( FJSON_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

/**
 * Sends the log message to a special 'user' log stream designed for messages that
 * the end user may like to see.
 */
#define ulog( FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (fjson::logger::get("user")).is_enabled( fjson::log_level::debug ) ) \
      (fjson::logger::get("user")).log( FJSON_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END


#define ilog( FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (fjson::logger::get(DEFAULT_LOGGER)).is_enabled( fjson::log_level::info ) ) \
      (fjson::logger::get(DEFAULT_LOGGER)).log( FJSON_LOG_MESSAGE( info, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

#define wlog( FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (fjson::logger::get(DEFAULT_LOGGER)).is_enabled( fjson::log_level::warn ) ) \
      (fjson::logger::get(DEFAULT_LOGGER)).log( FJSON_LOG_MESSAGE( warn, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

#define elog( FORMAT, ... ) \
  FJSON_MULTILINE_MACRO_BEGIN \
   if( (fjson::logger::get(DEFAULT_LOGGER)).is_enabled( fjson::log_level::error ) ) \
      (fjson::logger::get(DEFAULT_LOGGER)).log( FJSON_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
  FJSON_MULTILINE_MACRO_END

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>


#define FJSON_FORMAT_ARG(r, unused, base) \
  BOOST_PP_STRINGIZE(base) ": ${" BOOST_PP_STRINGIZE( base ) "} "

#define FJSON_FORMAT_ARGS(r, unused, base) \
  BOOST_PP_LPAREN() BOOST_PP_STRINGIZE(base),fjson::variant(base) BOOST_PP_RPAREN()

#define FJSON_FORMAT( SEQ )\
    BOOST_PP_SEQ_FOR_EACH( FJSON_FORMAT_ARG, v, SEQ ) 

// takes a ... instead of a SEQ arg because it can be called with an empty SEQ 
// from FJSON_CAPTURE_AND_THROW()
#define FJSON_FORMAT_ARG_PARAMS( ... )\
    BOOST_PP_SEQ_FOR_EACH( FJSON_FORMAT_ARGS, v, __VA_ARGS__ ) 

#define idump( SEQ ) \
    ilog( FJSON_FORMAT(SEQ), FJSON_FORMAT_ARG_PARAMS(SEQ) )  
#define wdump( SEQ ) \
    wlog( FJSON_FORMAT(SEQ), FJSON_FORMAT_ARG_PARAMS(SEQ) )  
#define edump( SEQ ) \
    elog( FJSON_FORMAT(SEQ), FJSON_FORMAT_ARG_PARAMS(SEQ) )  

// this disables all normal logging statements -- not something you'd normally want to do,
// but it's useful if you're benchmarking something and suspect logging is causing
// a slowdown.
#ifdef FJSON_DISABLE_LOGGING
# undef ulog
# define ulog(...) FJSON_MULTILINE_MACRO_BEGIN FJSON_MULTILINE_MACRO_END
# undef elog
# define elog(...) FJSON_MULTILINE_MACRO_BEGIN FJSON_MULTILINE_MACRO_END
# undef wlog
# define wlog(...) FJSON_MULTILINE_MACRO_BEGIN FJSON_MULTILINE_MACRO_END
# undef ilog
# define ilog(...) FJSON_MULTILINE_MACRO_BEGIN FJSON_MULTILINE_MACRO_END
# undef dlog
# define dlog(...) FJSON_MULTILINE_MACRO_BEGIN FJSON_MULTILINE_MACRO_END
#endif