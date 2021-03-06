
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
 * $Id$
 */

#include "MessageBuffer.h"
#include "RubyConfig.h"

MessageBuffer::MessageBuffer()
{ 
  m_msg_counter = 0;
  m_consumer_ptr = NULL;
  m_ordering_set = false;
  m_strict_fifo = true;
  m_size = 0;
  m_max_size = -1;
  m_last_arrival_time = 0;
  m_randomization = true;
  m_size_last_time_size_checked = 0;
  m_time_last_time_size_checked = 0;
  m_time_last_time_enqueue = 0;
  m_time_last_time_pop = 0;
  m_size_at_cycle_start = 0;
  m_msgs_this_cycle = 0;
  m_not_avail_count = 0;
  m_priority_rank = 0;
}

MessageBuffer::MessageBuffer(const Chip* chip_ptr)  // The chip_ptr is ignored, but could be used for extra debugging
{ 
  m_msg_counter = 0;
  m_consumer_ptr = NULL;
  m_ordering_set = false;
  m_strict_fifo = true;
  m_size = 0;
  m_max_size = -1;
  m_last_arrival_time = 0;
  m_randomization = true;
  m_size_last_time_size_checked = 0;
  m_time_last_time_size_checked = 0;
  m_time_last_time_enqueue = 0;
  m_time_last_time_pop = 0;
  m_size_at_cycle_start = 0;
  m_msgs_this_cycle = 0;
  m_not_avail_count = 0;
  m_priority_rank = 0;
}

int MessageBuffer::getSize()
{
  if(m_time_last_time_size_checked == g_eventQueue_ptr->getTime()){
    return m_size_last_time_size_checked;
  } else {
    m_time_last_time_size_checked = g_eventQueue_ptr->getTime();
    m_size_last_time_size_checked = m_size;
    return m_size;
  }
}

bool MessageBuffer::areNSlotsAvailable(int n)
{ 

  // fast path when message buffers have infinite size
  if(m_max_size == -1) {
    return true;
  }

  // determine my correct size for the current cycle
  // pop operations shouldn't effect the network's visible size until next cycle,
  // but enqueue operations effect the visible size immediately
  int current_size = max(m_size_at_cycle_start, m_size);
  if (m_time_last_time_pop < g_eventQueue_ptr->getTime()) {  // no pops this cycle - m_size is correct
    current_size = m_size;
  } else {
    if (m_time_last_time_enqueue < g_eventQueue_ptr->getTime()) {  // no enqueues this cycle - m_size_at_cycle_start is correct
      current_size = m_size_at_cycle_start;
    } else {  // both pops and enqueues occured this cycle - add new enqueued msgs to m_size_at_cycle_start
      current_size = m_size_at_cycle_start+m_msgs_this_cycle;
    }
  }

  // now compare the new size with our max size
  if(current_size+n <= m_max_size){ 
    return true; 
  } else {
    DEBUG_MSG(QUEUE_COMP,MedPrio,n);
    DEBUG_MSG(QUEUE_COMP,MedPrio,current_size);
    DEBUG_MSG(QUEUE_COMP,MedPrio,m_size);
    DEBUG_MSG(QUEUE_COMP,MedPrio,m_max_size);
    m_not_avail_count++;
    return false;
  }
}

const MsgPtr MessageBuffer::getMsgPtrCopy() const
{ 
  assert(isReady()); 
  
  MsgPtr temp_msg;
  temp_msg = *(m_prio_heap.peekMin().m_msgptr.ref());
  assert(temp_msg.ref() != NULL);
  return temp_msg; 
}

const Message* MessageBuffer::peekAtHeadOfQueue() const
{
  const Message* msg_ptr;
  DEBUG_NEWLINE(QUEUE_COMP,MedPrio);
    
  DEBUG_MSG(QUEUE_COMP,MedPrio,"Peeking at head of queue " + m_name + " time: " 
            + int_to_string(g_eventQueue_ptr->getTime()) + ".");
  assert(isReady());

  msg_ptr = m_prio_heap.peekMin().m_msgptr.ref();
  assert(msg_ptr != NULL);

  DEBUG_EXPR(QUEUE_COMP,MedPrio,*msg_ptr);
  DEBUG_NEWLINE(QUEUE_COMP,MedPrio);
  return msg_ptr;
}

// FIXME - move me somewhere else
int random_time() 
{
  int time = 1;
  time += random() & 0x3;  // [0...3]
  if ((random() & 0x7) == 0) {  // 1 in 8 chance
    time += 100 + (random() % 0xf); // 100 + [1...15]
  }
  return time;
}

void MessageBuffer::enqueue(const MsgPtr& message, Time delta)
{
  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);
  DEBUG_MSG(QUEUE_COMP,HighPrio,"enqueue " + m_name + " time: " 
            + int_to_string(g_eventQueue_ptr->getTime()) + ".");
  DEBUG_EXPR(QUEUE_COMP,MedPrio,message);
  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);

  m_msg_counter++;
  m_size++;
  m_buffer_size.add(m_size);

  // record current time incase we have a pop that also adjusts my size
  if (m_time_last_time_enqueue < g_eventQueue_ptr->getTime()) {
    m_msgs_this_cycle = 0;  // first msg this cycle
    m_time_last_time_enqueue = g_eventQueue_ptr->getTime();
  }
  m_msgs_this_cycle++;

  //ASSERT(m_max_size == -1 || m_size <= m_max_size + 1);
  // the plus one is a kluge because of a SLICC issue

  if (!m_ordering_set) {
    WARN_EXPR(*this);
    WARN_EXPR(m_name);
    ERROR_MSG("Ordering property of this queue has not been set");
  }

  // Calculate the arrival time of the message, that is, the first
  // cycle the message can be dequeued.
  assert(delta>0);
  Time current_time = g_eventQueue_ptr->getTime();
  Time arrival_time = 0;
  if (!RANDOMIZATION || (m_randomization == false)) {
    // No randomization
    arrival_time = current_time + delta;
    
  } else {
    // Randomization - ignore delta
    if (m_strict_fifo) {
      if (m_last_arrival_time < current_time) {
        m_last_arrival_time = current_time;
      }
      arrival_time = m_last_arrival_time + random_time();
    } else {
      arrival_time = current_time + random_time();
    }
  }

  // Check the arrival time
  assert(arrival_time > current_time);
  if (m_strict_fifo) {
    if (arrival_time >= m_last_arrival_time) {
      
    } else {
      WARN_EXPR(*this);
      WARN_EXPR(m_name);
      WARN_EXPR(current_time);
      WARN_EXPR(delta);
      WARN_EXPR(arrival_time);
      WARN_EXPR(m_last_arrival_time);
      ERROR_MSG("FIFO ordering violated");
    }
  }
  m_last_arrival_time = arrival_time;

  // compute the delay cycles and set enqueue time
  Message* msg_ptr = NULL;
  msg_ptr = message.mod_ref();
  assert(msg_ptr != NULL);
  assert(g_eventQueue_ptr->getTime() >= msg_ptr->getLastEnqueueTime());  // ensure we aren't dequeued early
  msg_ptr->setDelayedCycles((g_eventQueue_ptr->getTime() - msg_ptr->getLastEnqueueTime())+msg_ptr->getDelayedCycles());
  msg_ptr->setLastEnqueueTime(arrival_time);

  // Insert the message into the priority heap
  MessageBufferNode thisNode(arrival_time, m_msg_counter, message);
  m_prio_heap.insert(thisNode);

  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);
  DEBUG_MSG(QUEUE_COMP,HighPrio,"enqueue " + m_name 
            + " with arrival_time " + int_to_string(arrival_time)
            + " cur_time: " + int_to_string(g_eventQueue_ptr->getTime()) + ".");
  DEBUG_EXPR(QUEUE_COMP,MedPrio,message);
  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);

  // Schedule the wakeup
  if (m_consumer_ptr != NULL) {
    g_eventQueue_ptr->scheduleEventAbsolute(m_consumer_ptr, arrival_time);
  } else {
    WARN_EXPR(*this);
    WARN_EXPR(m_name);
    ERROR_MSG("No consumer");
  }
}

int MessageBuffer::dequeue_getDelayCycles(MsgPtr& message)
{ 
  int delay_cycles = -1;  // null value

  dequeue(message);

  // get the delay cycles
  delay_cycles = setAndReturnDelayCycles(message);

  assert(delay_cycles >= 0);
  return delay_cycles;
}

void MessageBuffer::dequeue(MsgPtr& message)
{ 
  DEBUG_MSG(QUEUE_COMP,MedPrio,"dequeue from " + m_name);
  message = m_prio_heap.peekMin().m_msgptr;

  pop();
  DEBUG_EXPR(QUEUE_COMP,MedPrio,message);
}

int MessageBuffer::dequeue_getDelayCycles()
{ 
  int delay_cycles = -1;  // null value

  // get MsgPtr of the message about to be dequeued
  MsgPtr message = m_prio_heap.peekMin().m_msgptr;

  // get the delay cycles
  delay_cycles = setAndReturnDelayCycles(message);

  dequeue();

  assert(delay_cycles >= 0);
  return delay_cycles;
}
 
void MessageBuffer::pop()
{
  DEBUG_MSG(QUEUE_COMP,MedPrio,"pop from " + m_name);
  assert(isReady());
  Time ready_time = m_prio_heap.extractMin().m_time;
  // record previous size and time so the current buffer size isn't adjusted until next cycle
  if (m_time_last_time_pop < g_eventQueue_ptr->getTime()) {
    m_size_at_cycle_start = m_size;
    m_time_last_time_pop = g_eventQueue_ptr->getTime();
  }
  m_size--;
}

void MessageBuffer::clear()
{
  while(m_prio_heap.size() > 0){
    m_prio_heap.extractMin();
  }

  ASSERT(m_prio_heap.size() == 0);

  m_msg_counter = 0;
  m_size = 0;
  m_time_last_time_enqueue = 0;
  m_time_last_time_pop = 0;
  m_size_at_cycle_start = 0;
  m_msgs_this_cycle = 0;
}

void MessageBuffer::recycle()
{
  //  const int RECYCLE_LATENCY = 3;
  DEBUG_MSG(QUEUE_COMP,MedPrio,"recycling " + m_name);
  assert(isReady());
  MessageBufferNode node = m_prio_heap.extractMin();
  node.m_time = g_eventQueue_ptr->getTime() + RECYCLE_LATENCY;
  m_prio_heap.insert(node);
  g_eventQueue_ptr->scheduleEventAbsolute(m_consumer_ptr, g_eventQueue_ptr->getTime() + RECYCLE_LATENCY);
}

int MessageBuffer::setAndReturnDelayCycles(MsgPtr& message)
{ 
  int delay_cycles = -1;  // null value

  // get the delay cycles of the message at the top of the queue
  Message* msg_ptr = message.ref();
  
  // this function should only be called on dequeue
  // ensure the msg hasn't been enqueued
  assert(msg_ptr->getLastEnqueueTime() <= g_eventQueue_ptr->getTime()); 
  msg_ptr->setDelayedCycles((g_eventQueue_ptr->getTime() - msg_ptr->getLastEnqueueTime())+msg_ptr->getDelayedCycles());
  delay_cycles = msg_ptr->getDelayedCycles();

  assert(delay_cycles >= 0);
  return delay_cycles;
}
 
void MessageBuffer::print(ostream& out) const
{
  out << "[MessageBuffer: ";
  if (m_consumer_ptr != NULL) {
    out << " consumer-yes ";
  }
  out << m_prio_heap << "] " << m_name << endl;
}

void MessageBuffer::printStats(ostream& out)
{
  out << "MessageBuffer: " << m_name << " stats - msgs:" << m_msg_counter << " full:" << m_not_avail_count << " size:" << m_buffer_size << endl;
}

