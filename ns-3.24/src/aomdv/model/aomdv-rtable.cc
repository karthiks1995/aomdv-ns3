/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on 
 *      NS-2 AOMDV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 * 
 *      AOMDV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AOMDV-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */

#include "aomdv-rtable.h"
#include <algorithm>
#include <iomanip>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <iostream>

NS_LOG_COMPONENT_DEFINE ("AomdvRoutingTable");

namespace ns3
{
namespace aomdv
{
/* 
Our contribution 
*/

void RoutingTableEntry::printPaths() {
	for (std::vector<Path>::const_iterator i = m_path.begin(); i != m_path.end(); ++i) {
	    	std::cout<<i->nexthop<<" "<<i->hopcount<<" "<<i->lasthop;  
	}
}

struct RoutingTableEntry::Path* RoutingTableEntry::PathInsert(Ipv4Address nh, uint16_t h, Time expire_time, Ipv4Address lh) {

  struct RoutingTableEntry::Path path = RoutingTableEntry::Path(nh, h, expire_time, lh);
  //assert(path);
  #ifdef DEBUG
  fprintf(stderr, "%s: (%d\t%d)\n", __FUNCTION__, path.nh, path.h);
  #endif // DEBUG
  m_path.push_back (path);
  rt_num_paths += 1;     //TODO
  RoutingTableEntry::Path *p = (struct RoutingTableEntry::Path*)malloc(sizeof(struct RoutingTableEntry::Path));
  p = &path;
  return p;
}

struct RoutingTableEntry::Path* RoutingTableEntry::PathLookup(Ipv4Address id) {

  //Path *path = rt_path_list.lh_first;
  NS_LOG_FUNCTION (this << id);
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
      if (i->nexthop == id)
      {
        NS_LOG_LOGIC ("Path " << id << " found");
        struct RoutingTableEntry::Path *path = &(*i);
        return path;
      }
  }
  NS_LOG_LOGIC ("Path " << id << " not found");
  return NULL;
}


struct RoutingTableEntry::Path* RoutingTableEntry::PathLookupDisjoint (Ipv4Address nh, Ipv4Address lh) {

  //Path *path = rt_path_list.lh_first;
  NS_LOG_FUNCTION (this << nh << lh);
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
      if (i->nexthop == nh && i->lasthop == lh)
      {
        NS_LOG_LOGIC ("Disjoint Path " << nh << lh << " found");
        struct RoutingTableEntry::Path *path = &(*i);
        return path;
      }
  }
  NS_LOG_LOGIC ("Disjoint Path " << nh << lh <<" not found");
  return NULL;
}

bool RoutingTableEntry::PathNewDisjoint (Ipv4Address nh, Ipv4Address lh) {

  //Path *path = rt_path_list.lh_first;
  NS_LOG_FUNCTION (this << nh << lh);
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
      if (i->nexthop == nh || i->lasthop == lh)
      {
        NS_LOG_LOGIC ("Disjoint New Path " << nh << lh << " found");
        return false;
      }
  }
  NS_LOG_LOGIC ("Disjoint New Path " << nh << lh <<" not found");
  return true;
}

struct RoutingTableEntry::Path* RoutingTableEntry::PathLookupLastHop (Ipv4Address id) {

  //Path *path = rt_path_list.lh_first;
  NS_LOG_FUNCTION (this << id);
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
      if (i->lasthop == id)
      {
        NS_LOG_LOGIC ("Path " << id << " found");
        struct RoutingTableEntry::Path *path = &(*i);
        return path;
      }
  }
  NS_LOG_LOGIC ("Path " << id << " not found");
  return NULL;
}

//TODO
void RoutingTableEntry::PathDelete (Ipv4Address id)
{
  NS_LOG_FUNCTION (this << id);
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
    if (i->nexthop == id)
    {
      NS_LOG_LOGIC ("Path " << id << " not found");
      m_path.erase(i);
      rt_num_paths -= 1;   //TODO
    }
  }
}

void RoutingTableEntry::PathAllDelete(void)
{
  NS_LOG_FUNCTION (this);
  m_path.clear (); 
  rt_num_paths = 0;
}

void RoutingTableEntry::PathDeleteLongest(void)
{
  struct RoutingTableEntry::Path *path = NULL;
  std::vector<Path>::iterator j;
  uint16_t max_hopcount = 0;
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
    if (i->hopcount > max_hopcount)
    {
      assert (i->hopcount != INFINITY2); //TODO
      path = &(*i);
      j = i;
      max_hopcount = i->hopcount;
    }
  }
  if(path)
  {
    m_path.erase(j);
    rt_num_paths -= 1;
  }
}

bool RoutingTableEntry::PathEmpty(void)
{
  struct RoutingTableEntry::Path *path = NULL;
  std::vector<Path>::iterator i = m_path.begin();
  path = &(*i);
  if (path) {
    assert(rt_num_paths > 0);
    return false;
  }
  else {
    assert(rt_num_paths == 0);
    return true;
  }

}

struct RoutingTableEntry::Path* RoutingTableEntry::PathFind(void)
{
  struct RoutingTableEntry::Path *path = NULL;
  std::vector<Path>::iterator i = m_path.begin();
  path = &(*i);
  return path;
}

struct RoutingTableEntry::Path* RoutingTableEntry::PathFindMinHop(void)
{
  struct RoutingTableEntry::Path *path = NULL;
  uint16_t min_hopcount = 0xffff;
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
    if (i->hopcount < min_hopcount)
    {
      path = &(*i);
      min_hopcount = i->hopcount;
    }
  }
  return path;
}

uint16_t RoutingTableEntry::PathGetMaxHopcount(void)
{
  uint16_t max_hopcount = 0;
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
    if (i->hopcount > max_hopcount)
    {
      max_hopcount = i->hopcount;
    }
  }
  if(max_hopcount == 0) return INFINITY2;
  else return max_hopcount;
} 

uint16_t RoutingTableEntry::PathGetMinHopcount(void)
{
  uint16_t min_hopcount = INFINITY2;
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
    if (i->hopcount < min_hopcount)
    {
      min_hopcount = i->hopcount;
    }
  }
  return min_hopcount;
}  

Time RoutingTableEntry::PathGetMaxExpirationTime(void)
{
  Time max_expire_time = Time();
  for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
  {
    if (i->expire > max_expire_time)
    {
      max_expire_time = i->expire;
    }
  }
  return max_expire_time;
} 

void RoutingTableEntry::PathPurge(void)
{
  Time now = Simulator::Now();
  bool cond;
  do{
    cond = false;
    for (std::vector<Path>::iterator i = m_path.begin(); i!= m_path.end(); ++i)
    {
      if (i->expire < now)
      {
       cond = true;
       m_path.erase(i);
       rt_num_paths -= 1;
       break;
      }
    }
  } while(cond);
}


/*
 The Routing Table
 */

RoutingTableEntry::RoutingTableEntry (Ptr<NetDevice> dev, Ipv4Address dst, bool vSeqNo, uint32_t seqNo,
                                      Ipv4InterfaceAddress iface, uint16_t hops, Ipv4Address nextHop, Time lifetime) :
  m_ackTimer (Timer::CANCEL_ON_DESTROY),
  rt_advertised_hops (INFINITY2), rt_highest_seqno_heard (0), rt_num_paths (0), rt_error (false),
  m_lifeTime (lifetime + Simulator::Now ()), m_iface (iface), m_flag (VALID),
  m_reqCount (0), m_blackListState (false), m_blackListTimeout (Simulator::Now ())
{
  m_ipv4Route = Create<Ipv4Route> ();
  m_ipv4Route->SetDestination (dst);
  m_ipv4Route->SetGateway (nextHop);
  m_ipv4Route->SetSource (m_iface.GetLocal ());
  m_ipv4Route->SetOutputDevice (dev);
}

RoutingTableEntry::~RoutingTableEntry ()
{
}

bool
RoutingTableEntry::InsertPrecursor (Ipv4Address id)
{
  NS_LOG_FUNCTION (this << id);
  if (!LookupPrecursor (id))
    {
      m_precursorList.push_back (id);
      return true;
    }
  else
    return false;
}

bool
RoutingTableEntry::LookupPrecursor (Ipv4Address id)
{
  NS_LOG_FUNCTION (this << id);
  for (std::vector<Ipv4Address>::const_iterator i = m_precursorList.begin (); i
       != m_precursorList.end (); ++i)
    {
      if (*i == id)
        {
          NS_LOG_LOGIC ("Precursor " << id << " found");
          return true;
        }
    }
  NS_LOG_LOGIC ("Precursor " << id << " not found");
  return false;
}

bool
RoutingTableEntry::DeletePrecursor (Ipv4Address id)
{
  NS_LOG_FUNCTION (this << id);
  std::vector<Ipv4Address>::iterator i = std::remove (m_precursorList.begin (),
                                                      m_precursorList.end (), id);
  if (i == m_precursorList.end ())
    {
      NS_LOG_LOGIC ("Precursor " << id << " not found");
      return false;
    }
  else
    {
      NS_LOG_LOGIC ("Precursor " << id << " found");
      m_precursorList.erase (i, m_precursorList.end ());
    }
  return true;
}

void
RoutingTableEntry::DeleteAllPrecursors ()
{
  NS_LOG_FUNCTION (this);
  m_precursorList.clear ();
}

bool
RoutingTableEntry::IsPrecursorListEmpty () const
{
  return m_precursorList.empty ();
}

void
RoutingTableEntry::GetPrecursors (std::vector<Ipv4Address> & prec) const
{
  NS_LOG_FUNCTION (this);
  if (IsPrecursorListEmpty ())
    return;
  for (std::vector<Ipv4Address>::const_iterator i = m_precursorList.begin (); i
       != m_precursorList.end (); ++i)
    {
      bool result = true;
      for (std::vector<Ipv4Address>::const_iterator j = prec.begin (); j
           != prec.end (); ++j)
        {
          if (*j == *i)
            result = false;
        }
      if (result)
        prec.push_back (*i);
    }
}

void
RoutingTableEntry::Invalidate (Time badLinkLifetime)
{
  NS_LOG_FUNCTION (this << badLinkLifetime.GetSeconds ());
  if (m_flag == INVALID)
    return;
  m_flag = INVALID;
  m_reqCount = 0;
  m_lifeTime = badLinkLifetime + Simulator::Now ();
}

void
RoutingTableEntry::Print (Ptr<OutputStreamWrapper> stream) const
{
  std::ostream* os = stream->GetStream ();
  *os << m_ipv4Route->GetDestination () << "\t" << m_ipv4Route->GetGateway ()
      << "\t" << m_iface.GetLocal () << "\t";
  switch (m_flag)
    {
    case VALID:
      {
        *os << "UP";
        break;
      }
    case INVALID:
      {
        *os << "DOWN";
        break;
      }
    case IN_SEARCH:
      {
        *os << "IN_SEARCH";
        break;
      }
    }
  *os << "\t";
  *os << std::setiosflags (std::ios::fixed) << 
  std::setiosflags (std::ios::left) << std::setprecision (2) <<
  std::setw (14) << (m_lifeTime - Simulator::Now ()).GetSeconds ();
  *os << "\t" << m_hops << "\n";
}

/*
 The Routing Table
 */

RoutingTable::RoutingTable (Time t) : 
  m_badLinkLifetime (t)
{
}

bool
RoutingTable::LookupRoute (Ipv4Address id, RoutingTableEntry & rt)
{
  NS_LOG_FUNCTION (this << id);
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found; m_ipv4AddressEntry is empty");
      return false;
    }
  std::map<Ipv4Address, RoutingTableEntry>::const_iterator i =
    m_ipv4AddressEntry.find (id);
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found");
      return false;
    }
  rt = i->second;
  NS_LOG_LOGIC ("Route to " << id << " found");
  return true;
}

bool
RoutingTable::LookupValidRoute (Ipv4Address id, RoutingTableEntry & rt)
{
  NS_LOG_FUNCTION (this << id);
  if (!LookupRoute (id, rt))
    {
      NS_LOG_LOGIC ("Route to " << id << " not found");
      return false;
    }
  NS_LOG_LOGIC ("Route to " << id << " flag is " << ((rt.GetFlag () == VALID) ? "valid" : "not valid"));
  return (rt.GetFlag () == VALID);
}

bool
RoutingTable::DeleteRoute (Ipv4Address dst)
{
  NS_LOG_FUNCTION (this << dst);
  Purge ();
  if (m_ipv4AddressEntry.erase (dst) != 0)
    {
      NS_LOG_LOGIC ("Route deletion to " << dst << " successful");
      return true;
    }
  NS_LOG_LOGIC ("Route deletion to " << dst << " not successful");
  return false;
}

bool
RoutingTable::AddRoute (RoutingTableEntry & rt)
{
  NS_LOG_FUNCTION (this);
  Purge ();
  if (rt.GetFlag () != IN_SEARCH)
    rt.SetRreqCnt (0);
  std::pair<std::map<Ipv4Address, RoutingTableEntry>::iterator, bool> result =
    m_ipv4AddressEntry.insert (std::make_pair (rt.GetDestination (), rt));
  return result.second;
}

bool
RoutingTable::Update (RoutingTableEntry & rt)
{
  NS_LOG_FUNCTION (this);
  std::map<Ipv4Address, RoutingTableEntry>::iterator i =
    m_ipv4AddressEntry.find (rt.GetDestination ());
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Route update to " << rt.GetDestination () << " fails; not found");
      return false;
    }
  i->second = rt;
  if (i->second.GetFlag () != IN_SEARCH)
    {
      NS_LOG_LOGIC ("Route update to " << rt.GetDestination () << " set RreqCnt to 0");
      i->second.SetRreqCnt (0);
    }
  return true;
}

bool
RoutingTable::SetEntryState (Ipv4Address id, RouteFlags state)
{
  NS_LOG_FUNCTION (this);
  std::map<Ipv4Address, RoutingTableEntry>::iterator i =
    m_ipv4AddressEntry.find (id);
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Route set entry state to " << id << " fails; not found");
      return false;
    }
  i->second.SetFlag (state);
  i->second.SetRreqCnt (0);
  NS_LOG_LOGIC ("Route set entry state to " << id << ": new state is " << state);
  return true;
}

void
RoutingTable::GetListOfDestinationWithNextHop (Ipv4Address nextHop, std::map<Ipv4Address, uint32_t> & unreachable )
{
  //contribution
  NS_LOG_FUNCTION (this);
  Purge ();
  unreachable.clear ();
  for (std::map<Ipv4Address, RoutingTableEntry>::iterator i = m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); ++i)
    {
      i->second.PathDelete(nextHop);
    }
}

/*
void
RoutingTable::InvalidateRoutesWithDst (const std::map<Ipv4Address, uint32_t> & unreachable)
{
  NS_LOG_FUNCTION (this);
  Purge ();
  for (std::map<Ipv4Address, RoutingTableEntry>::iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); ++i)
    {
      for (std::map<Ipv4Address, uint32_t>::const_iterator j =
             unreachable.begin (); j != unreachable.end (); ++j)
        {
          if ((i->first == j->first) && (i->second.GetFlag () == VALID))
            {
              NS_LOG_LOGIC ("Invalidate route with destination address " << i->first);
              i->second.Invalidate (m_badLinkLifetime);
            }
        }
    }
}
*/

void
RoutingTable::DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface)
{
  NS_LOG_FUNCTION (this);
  if (m_ipv4AddressEntry.empty ())
    return;
  for (std::map<Ipv4Address, RoutingTableEntry>::iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end ();)
    {
      if (i->second.GetInterface () == iface)
        {
          std::map<Ipv4Address, RoutingTableEntry>::iterator tmp = i;
          ++i;
          m_ipv4AddressEntry.erase (tmp);
        }
      else
        ++i;
    }
}

void
RoutingTable::Purge ()
{
  NS_LOG_FUNCTION (this);
  if (m_ipv4AddressEntry.empty ())
    return;
  for (std::map<Ipv4Address, RoutingTableEntry>::iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end ();)
    {
      if (i->second.GetLifeTime () < Seconds (0))
        {
          if (i->second.GetFlag () == INVALID)
            {
              std::map<Ipv4Address, RoutingTableEntry>::iterator tmp = i;
              ++i;
              m_ipv4AddressEntry.erase (tmp);
            }
          else if (i->second.GetFlag () == VALID)
            {
              NS_LOG_LOGIC ("Invalidate route with destination address " << i->first);
              i->second.Invalidate (m_badLinkLifetime);
              ++i;
            }
          else
            ++i;
        }
      else 
        {
          ++i;
        }
    }
}

void
RoutingTable::Purge (std::map<Ipv4Address, RoutingTableEntry> &table) const
{
  NS_LOG_FUNCTION (this);
  if (table.empty ())
    return;
  for (std::map<Ipv4Address, RoutingTableEntry>::iterator i =
         table.begin (); i != table.end ();)
    {
      if (i->second.GetLifeTime () < Seconds (0))
        {
          if (i->second.GetFlag () == INVALID)
            {
              std::map<Ipv4Address, RoutingTableEntry>::iterator tmp = i;
              ++i;
              table.erase (tmp);
            }
          else if (i->second.GetFlag () == VALID)
            {
              NS_LOG_LOGIC ("Invalidate route with destination address " << i->first);
              i->second.Invalidate (m_badLinkLifetime);
              ++i;
            }
          else
            ++i;
        }
      else 
        {
          ++i;
        }
    }
}

//contribution
bool RoutingTable::HasActiveRoutes() {
  for (std::map<Ipv4Address, RoutingTableEntry>::const_iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); ++i)
       {
        if(i->second.GetFlag() == VALID)
        {
          return true;
        }
       }
       return false;
}

bool
RoutingTable::MarkLinkAsUnidirectional (Ipv4Address neighbor, Time blacklistTimeout)
{
  NS_LOG_FUNCTION (this << neighbor << blacklistTimeout.GetSeconds ());
  std::map<Ipv4Address, RoutingTableEntry>::iterator i =
    m_ipv4AddressEntry.find (neighbor);
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Mark link unidirectional to  " << neighbor << " fails; not found");
      return false;
    }
  i->second.SetUnidirectional (true);
  i->second.SetBalcklistTimeout (blacklistTimeout);
  i->second.SetRreqCnt (0);
  NS_LOG_LOGIC ("Set link to " << neighbor << " to unidirectional");
  return true;
}

void
RoutingTable::Print (Ptr<OutputStreamWrapper> stream) const
{
  std::map<Ipv4Address, RoutingTableEntry> table = m_ipv4AddressEntry;
  Purge (table);
  *stream->GetStream () << "\nAOMDV Routing table\n"
                        << "Destination\tGateway\t\tInterface\tFlag\tExpire\t\tHops\n";
  for (std::map<Ipv4Address, RoutingTableEntry>::const_iterator i =
         table.begin (); i != table.end (); ++i)
    {
      i->second.Print (stream);
    }
  *stream->GetStream () << "\n";
}

}
}
