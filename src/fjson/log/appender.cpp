#include <fjson/log/appender.hpp>
#include <fjson/log/logger.hpp>
#include <fjson/thread/unique_lock.hpp>
#include <unordered_map>
#include <string>
#include <fjson/thread/spin_lock.hpp>
#include <fjson/thread/scoped_lock.hpp>
#include <fjson/log/console_appender.hpp>
#include <fjson/variant.hpp>
#include "console_defines.h"


namespace fjson {

   std::unordered_map<std::string,appender::ptr>& get_appender_map() {
     static std::unordered_map<std::string,appender::ptr> lm;
     return lm;
   }
   std::unordered_map<std::string,appender_factory::ptr>& get_appender_factory_map() {
     static std::unordered_map<std::string,appender_factory::ptr> lm;
     return lm;
   }
   appender::ptr appender::get( const fjson::string& s ) {
     static fjson::spin_lock appender_spinlock;
      scoped_lock<spin_lock> lock(appender_spinlock);
      return get_appender_map()[s];
   }
   bool  appender::register_appender( const fjson::string& type, const appender_factory::ptr& f )
   {
      get_appender_factory_map()[type] = f;
      return true;
   }
   appender::ptr appender::create( const fjson::string& name, const fjson::string& type, const variant& args  )
   {
      auto fact_itr = get_appender_factory_map().find(type);
      if( fact_itr == get_appender_factory_map().end() ) {
         //wlog( "Unknown appender type '%s'", type.c_str() );
         return appender::ptr();
      }
      auto ap = fact_itr->second->create( args );
      get_appender_map()[name] = ap;
      return ap;
   }
   
   static bool reg_console_appender = appender::register_appender<console_appender>( "console" );

} // namespace fjson
