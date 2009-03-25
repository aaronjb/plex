#pragma once

/*
 *      Copyright (C) 2005-2009 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
 
#include <string>
#include <map>

class CCriticalSection;
/// this class provides support for zeroconf
/// while the different zeroconf implementations have asynchronous APIs
/// this class hides it and provides only few ways to interact
/// with the services. If more control is needed, feel
/// free to add it. The main purpose currently is to provide an easy
/// way to publish services in the different StartXXX/StopXXX methods
/// in CApplication
/// TODO: Make me safe for use in static initialization. CritSec is a static member :/
///       use e.g. loki's singleton implementation to make do it properly
class CZeroconf
{
public:
  
  //tries to publish this service via zeroconf
  //fcr_identifier can be used to stop this service later
  //fcr_type is the zeroconf service type to publish (e.g. _http._tcp for webserver)
  //fcr_name is the name of the service to publish. The hostname is currently automatically appended
  //         and used for name collisions. e.g. XBMC would get published as fcr_name@Martn or, after collision fcr_name@Martn-2
  //f_port port of the service to publish
  // returns false if fcr_identifier was already present
  bool PublishService(const std::string& fcr_identifier,
                      const std::string& fcr_type,
                      const std::string& fcr_name,
                      unsigned int f_port);
  
  ///removes the specified service
  ///returns false if fcr_identifier does not exist
  bool RemoveService(const std::string& fcr_identifier);
  
  ///returns true if fcr_identifier exists
  bool HasService(const std::string& fcr_identifier) const;
  
  //starts publishing
  //services that were added with PublishService(...) while Zeroconf wasn't
  //started, get published now.
  void Start();
  
  // unpublishs all services (but keeps them stored in this class)
  // a call to Start() will republish them
  void Stop();
  
  // class methods
  // access to singleton; singleton gets created on call if not existent
  // if zeroconf is disabled (!HAS_ZEROCONF), this will return a dummy implementation that 
  // just does nothings, otherwise the platform specific one
  // Start() is called in construction if servers.zeroconf is enabled in guisettings
  static CZeroconf* GetInstance();
  // release the singleton; (save to call multiple times)
  static void   ReleaseInstance();
  // returns false if ReleaseInstance() was called befores
  static bool   IsInstantiated() { return  GetInstance() != 0; }
  
protected:
  //methods to implement for concrete implementations
  //publishs this service
  virtual bool doPublishService(const std::string& fcr_identifier,
                                const std::string& fcr_type,
                                const std::string& fcr_name,
                                unsigned int f_port) = 0;
  //removes the service if published
  virtual bool doRemoveService(const std::string& fcr_ident) = 0;

  //removes all services (short hand for "for i in m_service_map doRemoveService(i)")
  virtual void doStop() = 0;
  
protected:
  //singleton: we don't want to get instantiated nor copied or deleted from outside 
  CZeroconf();
  CZeroconf(const CZeroconf&);
  virtual ~CZeroconf();    
  
private:
  struct PublishInfo{
    std::string type;
    std::string name;
    unsigned int port; 
  };
  
  typedef std::map<std::string, PublishInfo> tServiceMap;
  tServiceMap m_service_map;
  bool m_started;

  static CZeroconf*& GetrInternalRef();
  //protects the data/singleton creation
  static CCriticalSection* smp_crit_sec;
};