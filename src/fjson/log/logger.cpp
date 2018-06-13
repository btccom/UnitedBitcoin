#include <fjson/log/logger.hpp>
#include <fjson/log/log_message.hpp>
#include <fjson/thread/spin_lock.hpp>
#include <fjson/thread/scoped_lock.hpp>
#include <fjson/log/appender.hpp>
#include <unordered_map>
#include <string>
#include <fjson/log/logger_config.hpp>

namespace fjson {

    class logger::impl : public fjson::retainable {
      public:
         impl()
         :_parent(nullptr),_enabled(true),_additivity(false),_level(log_level::warn){}
         fjson::string       _name;
         logger           _parent;
         bool             _enabled;
         bool             _additivity;
         log_level        _level;

         std::vector<appender::ptr> _appenders;
    };


    logger::logger()
    :my( new impl() ){}

    logger::logger(nullptr_t){}

    logger::logger( const string& name, const logger& parent )
    :my( new impl() )
    {
       my->_name = name;
       my->_parent = parent;
    }


    logger::logger( const logger& l )
    :my(l.my){}

    logger::logger( logger&& l )
    :my(fjson::move(l.my)){}

    logger::~logger(){}

    logger& logger::operator=( const logger& l ){
       my = l.my;
       return *this;
    }
    logger& logger::operator=( logger&& l ){
       fjson_swap(my,l.my);
       return *this;
    }
    bool operator==( const logger& l, std::nullptr_t ) { return !l.my; }
    bool operator!=( const logger& l, std::nullptr_t ) { return l.my;  }

    bool logger::is_enabled( log_level e )const {
       return e >= my->_level;
    }

    void logger::log( log_message m ) {
       m.get_context().append_context( my->_name );

       for( auto itr = my->_appenders.begin(); itr != my->_appenders.end(); ++itr )
          (*itr)->log( m );

       if( my->_additivity && my->_parent != nullptr) {
          my->_parent.log(m);
       }
    }
    void logger::set_name( const fjson::string& n ) { my->_name = n; }
    const fjson::string& logger::name()const { return my->_name; }

    extern bool do_default_config;

    std::unordered_map<std::string,logger>& get_logger_map() {
      static bool force_link_default_config = fjson::do_default_config;
      //TODO: Atomic compare/swap set
      static std::unordered_map<std::string,logger>* lm = new std::unordered_map<std::string, logger>();
      (void)force_link_default_config; // hide warning;
      return *lm;
    }

    logger logger::get( const fjson::string& s ) {
       static fjson::spin_lock logger_spinlock;
       scoped_lock<spin_lock> lock(logger_spinlock);
       return get_logger_map()[s];
    }

    logger  logger::get_parent()const { return my->_parent; }
    logger& logger::set_parent(const logger& p) { my->_parent = p; return *this; }

    log_level logger::get_log_level()const { return my->_level; }
    logger& logger::set_log_level(log_level ll) { my->_level = ll; return *this; }

    void logger::add_appender( const fjson::shared_ptr<appender>& a )
    { my->_appenders.push_back(a); }
    
//    void logger::remove_appender( const fjson::shared_ptr<appender>& a )
 //   { my->_appenders.erase(a); }

    std::vector<fjson::shared_ptr<appender> > logger::get_appenders()const
    {
        return my->_appenders;
    }

   bool configure_logging( const logging_config& cfg );
   bool do_default_config      = configure_logging( logging_config::default_config() );

} // namespace fjson
