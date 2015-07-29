/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of Sussex
 * Copyright (c) 2015 Université Pierre et Marie Curie (UPMC)
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
 * Author:  Matthieu Coudron <matthieu.coudron@lip6.fr>
 *          Morteza Kheirkhah <m.kheirkhah@sussex.ac.uk>
 */
#ifndef MPTCP_SUBFLOW_H
#define MPTCP_SUBFLOW_H

#include <stdint.h>
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <map>
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/sequence-number.h"
#include "ns3/rtt-estimator.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/tcp-socket.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/ipv4-address.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-header.h"
#include "ns3/mptcp-mapping.h"
//#include "ns3/tcp-option-mptcp.h"

using namespace std;

namespace ns3
{

class MpTcpSocketBase;
class MpTcpPathIdManager;
class TcpOptionMpTcpDSS;

/**
 * \class MpTcpSubflow
*/
class MpTcpSubflow : public TcpSocketBase
{
public:

  /**
  these 2 functions are temporary, because it's faster to implement like this
  but in the long term we should do as with single TCP and  replicate code in
  subclasses
  **/
//  virtual void OpenCwndInCA(uint32_t acked) = 0;
//  virtual void ReduceCwnd() = 0;

  /**
   * \return Value defined by meta socket GetSSThresh
   */
//  virtual uint32_t
//  GetSSThresh(void) const;

  static TypeId
  GetTypeId(void);

  // TODO pass it as virtual ?
  virtual TypeId GetInstanceTypeId(void) const;


  /**
  * \brief
  */
  uint32_t
  GetTxAvailable() const;


  /**
  the metasocket is the socket the application is talking to.
  Every subflow is linked to that socket.
  \param The metasocket it is linked to
  **/
  MpTcpSubflow();

  MpTcpSubflow(const TcpSocketBase& sock);
  MpTcpSubflow(const MpTcpSubflow&);
  virtual ~MpTcpSubflow();

//  TcpStates_t
//  GetState() const;

  virtual uint32_t
  UnAckDataCount();
  virtual uint32_t
  BytesInFlight();
  virtual uint32_t
  AvailableWindow();
  virtual uint32_t
  Window(void);               // Return the max possible number of unacked bytes
  // Undefined for now
//  virtual uint32_t
//  AvailableWindow(void);      // Return unfilled portion of window
  /**
  \return Value advertised by the meta socket
  */
  virtual uint16_t
  AdvertisedWindowSize(void);

  virtual void
  SetMeta(Ptr<MpTcpSocketBase> metaSocket);
//  virtual int
//  Connect(const Address &address);      // Setup endpoint and call ProcessAction() to connect

  static uint32_t
  GenerateTokenForKey(uint64_t key);

//  uint8_t addrId,
  /**
  \warning for prototyping purposes, we let the user free to advertise an IP that doesn't belong to the node
  (in reference to MPTCP connection agility).
  \note Maybe we should change this behavior ?
  TODO convert to Address to work with IPv6
  */
  virtual void
  AdvertiseAddress(Ipv4Address , uint16_t port);


  /**
  \param dack left edge window at connection level
  \param ack
  \param mapping
  **/
  virtual bool
  DiscardAtMostOneTxMapping(SequenceNumber64 const& dack,
    MpTcpMapping& mapping);

  /**
  \brief Send a REM_ADDR for the specific address.
  \see AdvertiseAddress
  \return false if no id associated with the address which likely means it was never advertised in the first place
  */
  virtual bool
  StopAdvertisingAddress(Ipv4Address);

  /**
   * \brief This is important. This should first request data from the meta
   */
  virtual void
  NotifySend (uint32_t spaceAvailable);

  /**
   * for debug
   */
  void
  DumpInfo() const;

  /**
  Would be nice to fit somewhere else. Even in global scope ?
  **/

//  template<class T>
//  static bool
//  GetMpTcpOption(const TcpHeader& header, Ptr<T> ret)
//  {
//    TcpHeader::TcpOptionList l;
//    header.GetOptions(l);
//    for(TcpHeader::TcpOptionList::const_iterator it = l.begin(); it != l.end(); ++it)
//    {
//      if( (*it)->GetKind() == TcpOption::MPTCP)
//      {
//        Ptr<TcpOptionMpTcpMain> opt = DynamicCast<TcpOptionMpTcpMain>(*it);
//        NS_ASSERT(opt);
//        T temp;
//        if( opt->GetSubType() == temp.GetSubType()  )
//        {
//          //!
//          ret = DynamicCast<T>(opt);
//          return true;
//        }
//      }
//    }
//    return false;
//  }


  /**
  \brief
  \note A Master socket is the first to initiate the connection, thus it will use the option MP_CAPABLE
      during the 3WHS while any additionnal subflow must resort to the MP_JOIN option
  \return True if this subflow is the first (should be unique) subflow attempting to connect
  **/
  virtual bool
  IsMaster() const;

  /**
  \return True if this subflow shall be used only when all the regular ones failed
  */
  virtual bool
  BackupSubflow() const;

  /**
  @brief According to rfc6824, the token is used to identify the MPTCP connection and is a
   cryptographic hash of the receiver's key, as exchanged in the initial
   MP_CAPABLE handshake (Section 3.1).  In this specification, the
   tokens presented in this option are generated by the SHA-1 algorithm, truncated to the most significant 32 bits.
  */
//  virtual uint32_t
//  GetLocalToken() const;

  virtual void
  DoForwardUp(Ptr<Packet> packet, Ipv4Header header, uint16_t port, Ptr<Ipv4Interface> incomingInterface);

  virtual bool
  SendPendingData(bool withAck = false);


  /**
  Disabled for now.
  SendMapping should be used instead.
  **/
  int
  Send(Ptr<Packet> p, uint32_t flags);



  //! disabled
  Ptr<Packet>
  RecvFrom(uint32_t maxSize, uint32_t flags, Address &fromAddress);

  //! disabled
  Ptr<Packet>
  Recv(uint32_t maxSize, uint32_t flags);

  //! Disabled
  Ptr<Packet>
  Recv(void);

  /**
  * \param dsn will set the dsn of the beginning of the data
  * \param only_full_mappings Set to true if you want to extract only packets that match a whole mapping
  * \param dsn returns the head DSN of the returned packet
  *
  * \return this can return an EmptyPacket if on close
  * Use a maxsize param ? if buffers linked then useless ?
  *
  */
  virtual Ptr<Packet>
//  RecvWithMapping(uint32_t maxSize, bool only_full_mappings, SequenceNumber32 &dsn);
//  Ptr<Packet>
  ExtractAtMostOneMapping(uint32_t maxSize, bool only_full_mappings, SequenceNumber64& dsn);

  //! TODO should notify upper layer
//  virtual void
//  PeerClose(Ptr<Packet>, const TcpHeader&); // Received a FIN from peer, notify rx buffer
//  virtual void
//  DoPeerClose(void); // FIN is in sequence, notify app and respond with a FIN
  virtual void
  ClosingOnEmpty(TcpHeader& header);

  virtual void
  DeallocateEndPoint(void);

  /*
  TODO move to meta.
  This should generate an *absolute*
  mapping with 64bits DSN etc...

  */
  virtual void
  ParseDSS(Ptr<Packet> p, const TcpHeader& header, Ptr<TcpOptionMpTcpDSS> dss);

    // State transition functions
  virtual void
  ProcessEstablished(Ptr<Packet>, const TcpHeader&); // Received a packet upon ESTABLISHED state

  /**
  * \
  * Why do I need this already :/ ?
  */
//  virtual Ptr<MpTcpSubflow>
//  ForkAsSubflow(void) = 0;

  /**
  * This should
  */
  virtual void NewAck(SequenceNumber32 const& ack);

  virtual void TimeWait();

  virtual void DoRetransmit();

//  virtual void
//  SetRemoteWindow(uint32_t );

  /**
  TODO move this up to TcpSocketBase
  **/
//  virtual uint32_t
//  RemoteWindow();

  virtual int Listen(void);
  /**
  TODO some options should be forwarded to the meta socket
  */
//  bool ReadOptions(Ptr<Packet> pkt, const TcpHeader& mptcpHeader);

  // TODO ? Moved from meta
  //  void ProcessListen  (uint8_t sFlowIdx, Ptr<Packet>, const TcpHeader&, const Address&, const Address&);
  void
  ProcessListen(Ptr<Packet> packet, const TcpHeader& tcpHeader, const Address& fromAddress, const Address& toAddress);

  /**
   * Will send MP_JOIN or MP_CAPABLE depending on if it is master or not
   * Updates the meta endpoint
   *
   * \see TcpSocketBase::CompleteFork
   */
  virtual void
  CompleteFork(Ptr<Packet> p, const TcpHeader& h, const Address& fromAddress, const Address& toAddress);

  virtual void
  ProcessSynRcvd(Ptr<Packet> packet, const TcpHeader& tcpHeader, const Address& fromAddress,
    const Address& toAddress);

  virtual void ProcessSynSent(Ptr<Packet> packet, const TcpHeader& tcpHeader);
  virtual void ProcessWait(Ptr<Packet> packet, const TcpHeader& tcpHeader);


  /**
  */
  virtual void Retransmit(void);


  Ptr<MpTcpPathIdManager> GetIdManager();

  /**
  Temporary, for debug
  **/
//  void
//  SetupTracing(const std::string prefix);
//  MpTcpMapping getSegmentOfACK( uint32_t ack);


protected:
  friend class MpTcpSocketBase;

  //

  /**
  * This is a public function in TcpSocketBase but it shouldn't be public here !
  **/
  virtual int
  Close(void);   // Close by app: Kill socket upon tx buffer emptied


  /**
  TODO mvoe to TcpSocketBase. Split  SendDataPacket into several functions ?
  */
  void
  GenerateDataPacketHeader(TcpHeader& header, SequenceNumber32 seq, uint32_t maxSize, bool withAck);


  virtual void
  CloseAndNotify(void);

  ///// Mappings related
  /**
  * \param mapping
  * \todo should check if mappings intersect, already exist etc...
  */
  virtual bool AddPeerMapping(const MpTcpMapping& mapping);


  /**
   * Depending on if this subflow is master or not, we want to
   * trigger
   * Callbacks being private members
   * Overrides parent in order to warn meta
   **/
  virtual void ConnectionSucceeded(void);

//
//  /**
//   * \brief NOOP. Initial cwnd should be set by meta.
//   */
//  virtual void
//  SetSSThresh(uint32_t threshold);
//
//
//  /**
//   * \brief NOOP. Initial cwnd should be set by meta.
//   */
//  virtual void
//  SetInitialCwnd(uint32_t cwnd);


  /**
   * \return Value defined by meta socket GetInitialCwnd
   */
//  virtual uint32_t
//  GetInitialCwnd(void) const;

  int
  DoConnect();

  /**
  * TODO in fact, instead of relying on IsMaster etc...
  * this should depend on meta's state , if it is wait or not
  * and thus it should be pushed in meta (would also remove the need for crypto accessors)
  */
  virtual void
  AddOptionMpTcp3WHS(TcpHeader& hdr) const;

  // TO remove hopefully
  virtual void
  ProcessClosing(Ptr<Packet> packet, const TcpHeader& tcpHeader);

//  virtual void ProcessOptionMpTcp (const Ptr<const TcpOption> option);
  virtual int ProcessOptionMpTcpSynSent(const Ptr<const TcpOption> option);

  /**
    GetMeta()->m_rxBuffer.NextRxSequence().GetValue()
  **/
//  virtual void
//  AppendDataAck(TcpHeader& hdr) const;
public:
  Ptr<MpTcpSocketBase>
  GetMeta() const;

protected:
  virtual void
  ReceivedAck(Ptr<Packet>, const TcpHeader&); // Received an ACK packet
  virtual void
  ReceivedData(Ptr<Packet>, const TcpHeader&);

  /**
  */
  uint32_t
  SendDataPacket(SequenceNumber32 seq, uint32_t maxSize, bool withAck);

  /**

  */
  uint32_t
//  SendDataPacket(SequenceNumber32 seq, uint32_t maxSize, bool withAck); // Send a data packet
  SendDataPacket(TcpHeader& header, const SequenceNumber32& ssn, uint32_t maxSize);

  /**
  * Like send, but pass on the global seq number associated with
  * \see Send
  **/
  virtual int
  SendMapping(Ptr<Packet> p,
              //SequenceNumber32 seq
              MpTcpMapping& mapping
              );


  virtual void
  ReTxTimeout();
  /**
  This one overridesprevious one, adding MPTCP options when needed
  */
//  virtual void
//  SendEmptyPacket(uint8_t flags);
  virtual void
  SendEmptyPacket(uint8_t flags); // Send a empty packet that carries a flag, e.g. ACK

  virtual void
  SendEmptyPacket(TcpHeader& header);

  virtual void AddMpTcpOptions (TcpHeader& header);
//  virtual Ptr<TcpSocketBase>
//  Fork(void); // Call CopyObject<> to clone me


// N'existe plus dans TcpSocketBase
//  virtual void
//  DupAck(const TcpHeader& t, uint32_t count); // Received dupack

  /* TODO should be able to use parent's one little by little
  */
  virtual void
  CancelAllTimers(void); // Cancel all timer when endpoint is deleted

//  virtual void AddDSNMapping(uint8_t sFlowIdx, uint64_t dSeqNum, uint16_t dLvlLen, uint32_t sflowSeqNum, uint32_t ack, Ptr<Packet> pkt);
//  virtual void StartTracing(string traced);
//  virtual void CwndTracer(uint32_t oldval, uint32_t newval);

//  virtual void SetFinSequence(const SequenceNumber32& s);
//  virtual bool Finished();
//  DSNMapping *GetunAckPkt();

//  void InitializeCwnd (void);

  uint16_t m_routeId;   //!< Subflow's ID (TODO rename into subflowId ). Position of this subflow in MetaSock's subflows std::vector


//  EventId m_retxEvent;          // Retransmission timer
//  EventId m_lastAckEvent;     // Timer for last ACK
//  EventId m_timewaitEvent;    // Timer for closing connection at sender side


  // TODO replace by parent's m_ m_cWnd
  TracedValue<uint32_t> m_cWnd; // Congestion window (in bytes)


  TracedValue<uint32_t> m_ssThresh;          //!< Slow start threshold
//  uint32_t maxSeqNb;          // Highest sequence number of a sent byte. Equal to (TxSeqNumber - 1) until a retransmission occurs
//  uint32_t highestAck;        // Highest received ACK for the subflow level sequence number
  uint32_t m_initialCWnd;     //!< Initial cWnd value
  SequenceNumber32 m_recover; //!< Previous highest Tx seqNb for fast recovery
  uint32_t m_retxThresh;      //!< Fast Retransmit threshold
  bool m_inFastRec;           // Currently in fast recovery
  bool m_limitedTx;           // perform limited transmit
//  uint32_t m_dupAckCount;     // DupACK counter TO REMOVE exist in parent

  // Use Ptr here so that we don't have to unallocate memory manually ?
//  typedef std::list<MpTcpMapping> MappingList
//  MappingList
  MpTcpMappingContainer m_TxMappings;  //!< List of mappings to send
  MpTcpMappingContainer m_RxMappings;  //!< List of mappings to receive



protected:
  Ptr<MpTcpSocketBase> m_metaSocket;


//private:

private:
  virtual void
  SendPacket(TcpHeader header, Ptr<Packet> p);

  bool m_backupSubflow; //!< Priority
  bool m_masterSocket;  //!< True if this is the first subflow established (with MP_CAPABLE)

  uint32_t m_localNonce;  //!< Store local host token, generated during the 3-way handshake

  int m_prefixCounter;  //!< Temporary variable to help with prefix generation . To remove later
//  uint32_t m_remoteToken; //!< Store remote host token

};

}
#endif /* MP_TCP_SUBFLOW */