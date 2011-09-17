// Logging System Implementation. Defined Logs:
//    psi::LogDebug
//    psi::LogInfo
//    psi::LogError
//    psi::endl
//
// Each of them dumps log messages into output file. It is possible to set
// message head (for example class name where message was posted from).
//
// Differences:
//   LogDebug   Dump all messages into file
//   LogInfo    print messages on screen via std::cout, dump into output
//              Info file, dump into Debug
//   LogError   print messages on screen via std::cerr, dump into output
//              Errorr file, dump into Debug
//
// Use psi::endl with all Loggers. std::endl does not work. Nothing will be
// dummped into file unless it is opened. File will be automatically closed
// at the end of program.
//
// Examples using LogInfo. LogDebug and LogError work in the same way:
//
//   psi::LogInfo.setOutput( "info.log"); // set output filename: works only once
//
//   psi::LogInfo << "This is a message into Info Log" << psi::endl;
//
//   psi::LogInfo() << "Analog of previous line: not head is output"
//                  << psi::endl;
//
//   psi::LogInfo( "Head") << "This is a message with head" << psi::endl;
//
//   psi::LogInfo( __func__) << "Message from some function" << psi::endl;
//
//   psi::LogInfo( "Test1") << "Voltage: " << _voltage << psi::endl;

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace psi
{
  // Colors
  enum Color { Default,
               Black , Red , Green , Yellow , Blue , Pink , Cyan , White , 
               BlackB, RedB, GreenB, YellowB, BlueB, PinkB, CyanB, WhiteB };

  class LEnd {};

  extern LEnd endl;

  // Logging Base Class. It's tasks:
  //   - Open Logging file (only once)
  //   - Perform actual logging. Can not be called directly. Use inheritance to
  //     define custom logging system. See examples below.
  template<class L>
    class Log
    {
      public:
        explicit Log() {} // force explicit use of Logging system
        virtual ~Log() {}

        // WARNING: Log file can be opened only once
        void setOutput( const std::string &file);

      protected:
        // Actual logging is done here via template method. Nothing will
        // be logged unless file is opened.
        template<class T> void log( const T &_val);

        void log( const LEnd &_endl);

        bool setHead( const std::string &_head);

        inline const std::string getHead() const { return head; }

        void logHead();

      private:
        // Prevent Copying including children
        Log( const Log &);
        Log &operator =( const Log &);

        // File will be automatically closed when AutoPointer gets released 
        // resulting in deleting object it refers to and calling
        // std::ofstream::~ofstream this way which in its turn calls
        // std::ofstream::close.
        static std::auto_ptr<std::ofstream> output;

        std::string head;
    };

  // Plugs (empty classes). They are used to Instantiate Log Services
  class Info;
  class Debug;
  class Error;

  // LogDebug should be used for all debugging (intermediate) information:
  // voltages, currents, 'hello world's, 'I am here', etc. Its output is
  // stored in separate file and not displayed on Monitor. Very useful for
  // later review by experts.
  class _LogDebug: public Log<Debug>
  {
    public:
      static _LogDebug &instance();

      template<class T>
        _LogDebug &operator <<( const T &_val);

      _LogDebug &operator <<( const Color &_color);
      _LogDebug &operator <<( const LEnd  &_endl);

      // Set Head for message. It will only be output for new lines.
      _LogDebug &operator()( const std::string &_head = "");
      _LogDebug &operator()(       std::string  _head,
                             const unsigned int &_width);

    private:
      _LogDebug(): loghead( false), newline( true) {}

      bool loghead;
      bool newline;

      static std::auto_ptr<_LogDebug> ginstance;
  };

  class _LogInfo: public Log<Info>
  {
    public:
      static _LogInfo &instance();

      template<class T>
        _LogInfo &operator <<( const T &_val);

      _LogInfo &operator <<( const Color &_color);
      _LogInfo &operator <<( const LEnd  &_endl);

      // Set Head for message. It will only be output for new lines.
      _LogInfo &operator()( const std::string &_head = "");
      _LogInfo &operator()(       std::string  _head,
                            const unsigned int &_width);

    private:
      _LogInfo(): loghead( false), newline( true) {}

      bool loghead;
      bool newline;

      static std::auto_ptr<_LogInfo> ginstance;
  };

  class _LogError: public Log<Error>
  {
    public:
      static _LogError &instance();

      template<class T>
        _LogError &operator <<( const T &_val);

      _LogError &operator <<( const Color &_color);
      _LogError &operator <<( const LEnd  &_endl);

      // Set Head for message. It will only be output for new lines.
      _LogError &operator()( const std::string &_head = "");
      _LogError &operator()(       std::string  _head,
                             const unsigned int &_width);

    private:
      _LogError(): loghead( false), newline( true) {}

      bool loghead;
      bool newline;

      static std::auto_ptr<_LogError> ginstance;
  };

  extern _LogDebug &LogDebug;
  extern _LogInfo  &LogInfo;
  extern _LogError &LogError;
}

// ----------------------------------------------------------------------------
// Log
template<class L> std::auto_ptr<std::ofstream> psi::Log<L>::output;

// Definitions
template<class L>
  template<class T>
    void psi::Log<L>::log( const T &_val)
    {
      if( output.get() ) *output << _val;
    }

template<class L>
  void psi::Log<L>::log( const LEnd &_endl)
  {
    if( output.get() ) *output << std::endl;
  }

template<class L>
  void psi::Log<L>::setOutput( const std::string &_file)
  {
    // Set output file only once
    if( output.get() || _file.empty() ) return;

    output.reset( new std::ofstream( _file.c_str() ) );
  }

template<class L>
  bool psi::Log<L>::setHead( const std::string &_head)
  {
    return _head.size() ? ( head = _head, true) : false; 
  }

template<class L>
  void psi::Log<L>::logHead()
  {
    if( head.size() ) log( ( head + "] ").insert( 0, "[") );
  }



// _LogDebug templates
template<class T>
  psi::_LogDebug &psi::_LogDebug::operator <<( const T &_val)
  {
    // Dump message into Debug file
    if( newline && loghead) {
      logHead();

      loghead = false;
    }

    newline = false;

    log( _val); return *this;
  }



// _LogInfo templates
template<class T>
  psi::_LogInfo &psi::_LogInfo::operator <<( const T &_val)
  {
    // Dump message into Info file
    if( newline) 
    { 
      if( loghead) {
        logHead();

        std::cout << ( getHead() + "\033[0m] ").insert( 0, "[\033[1;30m");

        loghead = false;
      }

      newline = false;
    }

    log( _val); LogDebug << _val;

    std::cout << _val;

    return *this;
  }



// _LogError templates
template<class T>
  psi::_LogError &psi::_LogError::operator <<( const T &_val)
  {
    // Dump message into Error file
    if( newline) 
    { 
      if( loghead) {
        logHead();

        std::cerr << ( getHead() + "\033[0m] ").insert( 0, "[\033[1;31mERROR: ");

        loghead = false;
      }

      newline = false;
    }

    log( _val); LogDebug << _val;

    std::cerr << _val;

    return *this;
  }

#endif // End LOG_H
