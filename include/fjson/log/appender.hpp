#pragma once
#include <fjson/shared_ptr.hpp>
#include <fjson/string.hpp>

namespace fjson {
   class appender;
   class log_message;
   class variant;

   class appender_factory : public fjson::retainable {
      public:
       typedef fjson::shared_ptr<appender_factory> ptr;

       virtual ~appender_factory(){};
       virtual fjson::shared_ptr<appender> create( const variant& args ) = 0;
   };

   namespace detail {
      template<typename T>
      class appender_factory_impl : public appender_factory {
        public:
           virtual fjson::shared_ptr<appender> create( const variant& args ) {
              return fjson::shared_ptr<appender>(new T(args));
           }
      };
   }

   class appender : public fjson::retainable {
      public:
         typedef fjson::shared_ptr<appender> ptr;

         template<typename T>
         static bool register_appender(const fjson::string& type) {
            return register_appender( type, new detail::appender_factory_impl<T>() );
         }

         static appender::ptr create( const fjson::string& name, const fjson::string& type, const variant& args  );
         static appender::ptr get( const fjson::string& name );
         static bool          register_appender( const fjson::string& type, const appender_factory::ptr& f );

         virtual void log( const log_message& m ) = 0;
   };
}
