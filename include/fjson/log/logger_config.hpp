#pragma once
#include <fjson/log/logger.hpp>

namespace fjson {
   class path;
   struct appender_config {
      appender_config(const string& name = "",
                      const string& type = "", 
                      variant args = variant()) :
        name(name),
        type(type),
        args(fjson::move(args)),
        enabled(true)
      {}
      string   name;
      string   type;
      variant  args;
      bool     enabled;
   };

   struct logger_config {
      logger_config(const fjson::string& name = ""):name(name),enabled(true),additivity(false){}
      string                           name;
      ostring                          parent;
      /// if not set, then parents level is used.
      fjson::optional<log_level>          level;
      bool                             enabled;
      /// if any appenders are sepecified, then parent's appenders are not set.
      bool                             additivity;
      std::vector<string>               appenders;

      logger_config& add_appender( const string& s );
   };

   struct logging_config {
      static logging_config default_config();
      std::vector<string>          includes;
      std::vector<appender_config> appenders;
      std::vector<logger_config>   loggers;
   };

   bool configure_logging( const logging_config& l );
}

#include <fjson/reflect/reflect.hpp>
FJSON_REFLECT( fjson::appender_config, (name)(type)(args)(enabled) )
FJSON_REFLECT( fjson::logger_config, (name)(parent)(level)(enabled)(additivity)(appenders) )
FJSON_REFLECT( fjson::logging_config, (includes)(appenders)(loggers) )
