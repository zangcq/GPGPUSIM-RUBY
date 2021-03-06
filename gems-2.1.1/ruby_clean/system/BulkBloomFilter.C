
/*
    Copyright (C) 1999-2008 by Mark D. Hill and David A. Wood for the
    Wisconsin Multifacet Project.  Contact: gems@cs.wisc.edu
    http://www.cs.wisc.edu/gems/

    --------------------------------------------------------------------

    This file is part of the Ruby Multiprocessor Memory System Simulator, 
    a component of the Multifacet GEMS (General Execution-driven 
    Multiprocessor Simulator) software toolset originally developed at 
    the University of Wisconsin-Madison.

    Ruby was originally developed primarily by Milo Martin and Daniel
    Sorin with contributions from Ross Dickson, Carl Mauer, and Manoj
    Plakal.

    Substantial further development of Multifacet GEMS at the
    University of Wisconsin was performed by Alaa Alameldeen, Brad
    Beckmann, Jayaram Bobba, Ross Dickson, Dan Gibson, Pacia Harper,
    Derek Hower, Milo Martin, Michael Marty, Carl Mauer, Michelle Moravan,
    Kevin Moore, Andrew Phelps, Manoj Plakal, Daniel Sorin, Haris Volos, 
    Min Xu, and Luke Yen.
    --------------------------------------------------------------------

    If your use of this software contributes to a published paper, we
    request that you (1) cite our summary paper that appears on our
    website (http://www.cs.wisc.edu/gems/) and (2) e-mail a citation
    for your published paper to gems@cs.wisc.edu.

    If you redistribute derivatives of this software, we request that
    you notify us and either (1) ask people to register with us at our
    website (http://www.cs.wisc.edu/gems/) or (2) collect registration
    information and periodically send it to us.

    --------------------------------------------------------------------

    Multifacet GEMS is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    Multifacet GEMS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Multifacet GEMS; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307, USA

    The GNU General Public License is contained in the file LICENSE.

### END HEADER ###
*/

/*
 * BulkBloomFilter.C
 * 
 * Description: 
 *
 *
 */

#include "BulkBloomFilter.h"
#include "Map.h"
#include "Address.h"

BulkBloomFilter::BulkBloomFilter(string str) 
{
  string tail(str);
  string head = string_split(tail, '_');
  
  int smt_threads = RubyConfig::numberofSMTThreads();
  m_filter_size = atoi(head.c_str());
  m_filter_size_bits = log_int(m_filter_size);
  // split the filter bits in half, c0 and c1
  m_sector_bits = m_filter_size_bits - 1;

  m_temp_filter.setSize(m_filter_size);
  m_filter.setSize(m_filter_size);
  clear();

  // clear temp filter
  for(int i=0; i < m_filter_size; ++i){
    m_temp_filter[i] = 0;
  }
}

BulkBloomFilter::~BulkBloomFilter(){

}

void BulkBloomFilter::clear()
{
  for (int i = 0; i < m_filter_size; i++) {
    m_filter[i] = 0;
  }
}

void BulkBloomFilter::increment(const Address& addr)
{
  // Not used
}


void BulkBloomFilter::decrement(const Address& addr)
{
  // Not used
}

void BulkBloomFilter::merge(AbstractBloomFilter * other_filter)
{
  // TODO
}

void BulkBloomFilter::set(const Address& addr)
{
  // c0 contains the cache index bits
  int set_bits = m_sector_bits;
  int block_bits = RubyConfig::dataBlockBits();
  int c0 = addr.bitSelect( block_bits, block_bits + set_bits - 1);
  // c1 contains the lower m_sector_bits permuted bits
  //Address permuted_bits = permute(addr);
  //int c1 = permuted_bits.bitSelect(0, set_bits-1);
  int c1 = addr.bitSelect( block_bits+set_bits, (block_bits+2*set_bits) - 1);
  //ASSERT(c0 < (m_filter_size/2));
  //ASSERT(c0 + (m_filter_size/2) < m_filter_size);
  //ASSERT(c1 < (m_filter_size/2));
  // set v0 bit
  m_filter[c0 + (m_filter_size/2)] = 1;
  // set v1 bit
  m_filter[c1] = 1;
}

void BulkBloomFilter::unset(const Address& addr)
{
  // not used
}

bool BulkBloomFilter::isSet(const Address& addr) 
{
  // c0 contains the cache index bits
  int set_bits = m_sector_bits;
  int block_bits = RubyConfig::dataBlockBits();  
  int c0 = addr.bitSelect( block_bits, block_bits + set_bits - 1);
  // c1 contains the lower 10 permuted bits
  //Address permuted_bits = permute(addr);
  //int c1 = permuted_bits.bitSelect(0, set_bits-1);
  int c1 = addr.bitSelect( block_bits+set_bits, (block_bits+2*set_bits) - 1);
  //ASSERT(c0 < (m_filter_size/2));
  //ASSERT(c0 + (m_filter_size/2) < m_filter_size);
  //ASSERT(c1 < (m_filter_size/2));
  // set v0 bit
  m_temp_filter[c0 + (m_filter_size/2)] = 1;
  // set v1 bit
  m_temp_filter[c1] = 1;

  // perform filter intersection. If any c part is 0, no possibility of address being in signature. 
  // get first c intersection part
  bool zero = false;
  for(int i=0; i < m_filter_size/2; ++i){
    // get intersection of signatures
    m_temp_filter[i] = m_temp_filter[i] && m_filter[i];
    zero = zero || m_temp_filter[i];
  }
  zero = !zero;
  if(zero){
    // one section is zero, no possiblility of address in signature
    // reset bits we just set
    m_temp_filter[c0 + (m_filter_size/2)] = 0;
    m_temp_filter[c1] = 0;
    return false;
  }

  // check second section
  zero = false;
  for(int i=m_filter_size/2; i < m_filter_size; ++i){
    // get intersection of signatures
    m_temp_filter[i] =  m_temp_filter[i] && m_filter[i];
    zero = zero || m_temp_filter[i];
  }
  zero = !zero;
  if(zero){
    // one section is zero, no possiblility of address in signature
    m_temp_filter[c0 + (m_filter_size/2)] = 0;
    m_temp_filter[c1] = 0;
    return false;
  }
  // one section has at least one bit set
  m_temp_filter[c0 + (m_filter_size/2)] = 0;
  m_temp_filter[c1] = 0;
  return true;
}


int BulkBloomFilter::getCount(const Address& addr)
{
  // not used
  return 0;
}

int BulkBloomFilter::getTotalCount()
{
  int count = 0;
  for (int i = 0; i < m_filter_size; i++) {
    if (m_filter[i]) {
      count++;
    }
  }
  return count;
}

int BulkBloomFilter::getIndex(const Address& addr)
{
  return get_index(addr); 
}

int BulkBloomFilter::readBit(const int index) {
  return 0;
  // TODO
}

void BulkBloomFilter::writeBit(const int index, const int value) {
  // TODO
}

void BulkBloomFilter::print(ostream& out) const
{
}

int BulkBloomFilter::get_index(const Address& addr) 
{
  return addr.bitSelect( RubyConfig::dataBlockBits(), RubyConfig::dataBlockBits() + m_filter_size_bits - 1);
}

Address BulkBloomFilter::permute(const Address & addr){
  // permutes the original address bits according to Table 5
  int block_offset = RubyConfig::dataBlockBits();
  physical_address_t part1 = addr.bitSelect( block_offset, block_offset + 6 );
  physical_address_t part2 = addr.bitSelect( block_offset + 9, block_offset + 9 );
  physical_address_t part3 = addr.bitSelect( block_offset + 11, block_offset + 11 );
  physical_address_t part4 = addr.bitSelect( block_offset + 17, block_offset + 17 );
  physical_address_t part5 = addr.bitSelect( block_offset + 7, block_offset + 8 );
  physical_address_t part6 = addr.bitSelect( block_offset + 10, block_offset + 10 );
  physical_address_t part7 = addr.bitSelect( block_offset + 12, block_offset + 12 );
  physical_address_t part8 = addr.bitSelect( block_offset + 13, block_offset + 13 );
  physical_address_t part9 = addr.bitSelect( block_offset + 15, block_offset + 16 );
  physical_address_t part10 = addr.bitSelect( block_offset + 18, block_offset + 20 );
  physical_address_t part11 = addr.bitSelect( block_offset + 14, block_offset + 14 );

  physical_address_t result = (part1 << 14 ) | (part2 << 13 ) | (part3 << 12 ) | (part4 << 11 ) | (part5 << 9) | (part6 << 8)
    | (part7 << 7) | (part8 << 6) | (part9 << 4) | (part10 << 1) | (part11);
  // assume 32 bit addresses (both virtual and physical)
  // select the remaining high-order 11 bits
  physical_address_t remaining_bits = (addr.bitSelect( block_offset + 21, 31 )) << 21;
  result = result | remaining_bits;

  return Address(result);
}
