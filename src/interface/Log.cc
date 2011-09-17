#include "Log.h"

psi::LEnd psi::endl;

// _LogDebug
std::auto_ptr<psi::_LogDebug> psi::_LogDebug::ginstance;

psi::_LogDebug &psi::_LogDebug::instance()
{
  if( !ginstance.get() ) ginstance.reset( new _LogDebug() );

  return *ginstance;
}

psi::_LogDebug &psi::_LogDebug::operator <<( const Color &_color)
{
  // Do nothing in Debug

  return *this;
}

psi::_LogDebug &psi::_LogDebug::operator <<( const LEnd &_endl)
{
  newline = true; 
  log( _endl);
  
  return *this;
}

psi::_LogDebug &psi::_LogDebug::operator()( const std::string &_HEAD)
{
  loghead = setHead( _HEAD);

  return *this;
}

psi::_LogDebug &psi::_LogDebug::operator()( std::string _head,
                                            const unsigned int &_width)
{
  if( 0 < _width && _width > _head.size() )
    _head.resize( _width, ' ');

  return operator()( _head);
}



// _LogInfo
std::auto_ptr<psi::_LogInfo> psi::_LogInfo::ginstance;

psi::_LogInfo &psi::_LogInfo::instance()
{
  if( !ginstance.get() ) ginstance.reset( new _LogInfo() );

  return *ginstance;
}

psi::_LogInfo &psi::_LogInfo::operator <<( const Color &_color)
{
  switch( _color)
  {
    case Black  : std::cout << "\033[30m"   ; break;    
    case BlackB : std::cout << "\033[1;30m" ; break;    
    case Red    : std::cout << "\033[31m"   ; break;    
    case RedB   : std::cout << "\033[1;31m" ; break;    
    case Green  : std::cout << "\033[32m"   ; break;    
    case GreenB : std::cout << "\033[1;32m" ; break;    
    case Yellow : std::cout << "\033[33m"   ; break;    
    case YellowB: std::cout << "\033[1;33m" ; break;    
    case Blue   : std::cout << "\033[34m"   ; break;    
    case BlueB  : std::cout << "\033[1;34m" ; break;    
    case Pink   : std::cout << "\033[35m"   ; break;    
    case PinkB  : std::cout << "\033[1;35m" ; break;    
    case Cyan   : std::cout << "\033[36m"   ; break;    
    case CyanB  : std::cout << "\033[1;36m" ; break;    
    case White  : std::cout << "\033[37m"   ; break;    
    case WhiteB : std::cout << "\033[1;37m" ; break;    
    case Default: std::cout << "\033[0m"    ; break;

    // Do nothing if color is not found
    default     :  break;
  }

  return *this;
}

psi::_LogInfo &psi::_LogInfo::operator <<( const LEnd &_endl)
{
  newline = true; 

  // Put end line into Log output
  log( _endl);

  LogDebug << endl;

  std::cout << "\033[0m" << std::endl;
  
  return *this;
}

psi::_LogInfo &psi::_LogInfo::operator()( const std::string &_HEAD)
{
  // Dump into Debug
  LogDebug( _HEAD);

  loghead = setHead( _HEAD);

  return *this;
}

psi::_LogInfo &psi::_LogInfo::operator()( std::string _head,
                                          const unsigned int &_width)
{
  if( 0 < _width && _width > _head.size() )
    _head.resize( _width, ' ');

  return operator()( _head);
}



// _LogError
std::auto_ptr<psi::_LogError> psi::_LogError::ginstance;

psi::_LogError &psi::_LogError::instance()
{
  if( !ginstance.get() ) ginstance.reset( new _LogError() );

  return *ginstance;
}

psi::_LogError &psi::_LogError::operator <<( const Color &_color)
{
  switch( _color)
  {
    case Black  : std::cout << "\033[30m"   ; break;    
    case BlackB : std::cout << "\033[1;30m" ; break;    
    case Red    : std::cout << "\033[31m"   ; break;    
    case RedB   : std::cout << "\033[1;31m" ; break;    
    case Green  : std::cout << "\033[32m"   ; break;    
    case GreenB : std::cout << "\033[1;32m" ; break;    
    case Yellow : std::cout << "\033[33m"   ; break;    
    case YellowB: std::cout << "\033[1;33m" ; break;    
    case Blue   : std::cout << "\033[34m"   ; break;    
    case BlueB  : std::cout << "\033[1;34m" ; break;    
    case Pink   : std::cout << "\033[35m"   ; break;    
    case PinkB  : std::cout << "\033[1;35m" ; break;    
    case Cyan   : std::cout << "\033[36m"   ; break;    
    case CyanB  : std::cout << "\033[1;36m" ; break;    
    case White  : std::cout << "\033[37m"   ; break;    
    case WhiteB : std::cout << "\033[1;37m" ; break;    
    case Default: std::cout << "\033[0m"    ; break;

    // Do nothing if color is not found
    default     :  break;
  }

  return *this;
}

psi::_LogError &psi::_LogError::operator <<( const LEnd &_endl)
{
  newline = true; 

  // Put end line into Log output
  log( _endl); 

  LogDebug << endl;

  std::cerr << std::endl;
  
  return *this;
}

psi::_LogError &psi::_LogError::operator()( const std::string &_HEAD)
{
  // Dump into Debug
  LogDebug( _HEAD);

  loghead = setHead( _HEAD);

  return *this;
}

psi::_LogError &psi::_LogError::operator()( std::string _head,
                                            const unsigned int &_width)
{
  if( 0 < _width && _width > _head.size() )
    _head.resize( _width, ' ');

  return operator()( _head);
}


// WARNING: should be kept at the bottom of file b/c on top instance() is not
//          implemented.
psi::_LogDebug &psi::LogDebug = psi::_LogDebug::instance();
psi::_LogInfo  &psi::LogInfo  = psi::_LogInfo::instance();
psi::_LogError &psi::LogError = psi::_LogError::instance();
