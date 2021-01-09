#ifndef RHMQ_H
#define RHMQ_H

#ifdef PRAGMA_DISABLE_UNDEFINED_IDENTIFIER_WARNINGS
PRAGMA_DISABLE_UNDEFINED_IDENTIFIER_WARNINGS
#endif

#ifdef _WIN32
#define NOMINMAX
#undef APIENTRY
#else
// NOTE: Remove these after migration to std::string
#define strncpy_s(a, b, c, d) strncpy(a, c, d)
#define strcpy_s(a, b, c) strcpy(a, c)
#define strncat_s(a, b, c, d) strncat(a, c, d)
#define strcat_s(a, b, c) strcat(a, c)
#endif

// Deprecation for different platforms
#ifdef _WIN32
#define RHMQDEPRECATE_BEGIN(message) __declspec(deprecated(message))
#define RHMQDEPRECATE_END(message)
#else
#define RHMQDEPRECATE_BEGIN(message)
#define RHMQDEPRECATE_END(message) __attribute__((deprecated(message)))
#endif

// Unreal Engine detection from UBT defines
#if (defined(UE_BUILD_DEBUG) || defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_SHIPPING) || defined(UE_BUILD_TEST))
#include "CoreGlobals.h"
#endif

// Guard for Unreal Engine under Windows to patch WinSock2.h residue
#if PLATFORM_DESKTOP
# if PLATFORM_WINDOWS
#  include "Windows/WindowsHWrapper.h"
# endif
  THIRD_PARTY_INCLUDES_START
#endif

#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>

#if PLATFORM_DESKTOP
  THIRD_PARTY_INCLUDES_END
#elif defined(_WIN32)
#undef far
#undef near
#endif

#include <string.h>

#include <algorithm>
#include <map>
#include <cmath>
#include <set>
#include <string>
#include <vector>
#include <chrono>

// Platform specific logging functions

// Unreal Engine
#if PLATFORM_DESKTOP
#define RHMQ_LOG_(Rhmq, Format, ...) \
{ \
	char msg[512]; \
	snprintf(msg, 512, "RHMQ[%s]: ", Rhmq->GetLabel()); \
	int plen = strlen(msg); \
	snprintf(&msg[plen], 512 - plen, Format, ##__VA_ARGS__); \
	auto ue_msg = StringCast<TCHAR>(static_cast<const ANSICHAR*>(msg)); \
	UE_LOG(LogTemp, Error, TEXT("%s"), ue_msg.Get()); \
}
#endif

// Default to stderr output
#ifndef RHMQ_LOG_
#define RHMQ_LOG_(Rhmq, Format, ...) \
{ \
	fprintf(stderr,"RHMQ[%s]: ", Rhmq->GetLabel()); \
	fprintf(stderr, Format, ##__VA_ARGS__); \
	fputs("\n",stderr); \
	fflush(stderr); \
}
#endif

#define RHMQ_LOG(Format, ...) RHMQ_LOG_(this, Format, ##__VA_ARGS__)

// Infinite timeout value passed to Receive() or IsConnected() to block
#define RHMQ_TIMEOUT_INF -1

// Default timeout (in seconds) for first send command to wait for a connection
#ifndef RHMQ_TIMEOUT
#define RHMQ_TIMEOUT 60
#endif

// Default timeout (in milliseconds) for Send() calls
#ifndef RHMQ_SEND_TIMEOUT_MILLIS
#define RHMQ_SEND_TIMEOUT_MILLIS 3000
#endif

// Default timeout (in seconds) for Receive() calls
#ifndef RHMQ_RECV_TIMEOUT_MILLIS
#define RHMQ_RECV_TIMEOUT_MILLIS 0
#endif

// Default number of messsages to queue up for Send() calls
#ifndef RHMQ_SEND_BUFFER_MSGS
#define RHMQ_SEND_BUFFER_MSGS 10
#endif

// Default number of messsages to queue up for Receive() calls
#ifndef RHMQ_RECV_BUFFER_MSGS
#define RHMQ_RECV_BUFFER_MSGS 10
#endif

class RHMQ_Socket;

//////////////////////////////////////////
// RHMQ handles ZMQ creation/management of sockets
class RHMQ
{
private:
	friend class RHMQ_Socket;

public:
	static RHMQ* GetInstance(const char* name = DefaultName)
	{
		if( !name || !*name ) { name = DefaultName; }
		return GetSetNamedInstance(name);
	}

	// Used only to share contexts between shared libraries with different address spaces
	static void SetInstance(RHMQ* context)
	{
		if (context) GetSetNamedInstance(context->Name, context);
	}

	// While RHMQ will facilitate creating the socket, it does not own it.
	// Who ever owns it takes care to memory manage the socket so
	// that it is cleaned up correctly.  The Socket will take care of
	// cleaning up reference to itself in the instance.
	static RHMQ_Socket* CreateSocket(const char* instanceName = nullptr);

	// Returns ZMQ context (zmq_ctx_new)
	void *GetZmqContext() const { return ZmqContext; }

	const std::vector<class RHMQ_Socket*> &GetSockets() const { return Sockets; }

	inline int GetUnconnectedCount();

	// NOTE: ZMQ calls zmq_ctx_destroy during atexit()
	// Everything in RHMQ self cleans.
	~RHMQ() {};

	// Slight misnomer to be compatible with logging macros
	const char* GetLabel() { return Name; }
private:
	RHMQ(const char* name = nullptr) : ZmqContext(zmq_ctx_new())
	{
		//zmq_ctx_set(ZmqContext, ZMQ_IO_THREADS, 1);
		// override default number of sockets
		zmq_ctx_set(ZmqContext, ZMQ_MAX_SOCKETS, 65535);
		if (name == nullptr)
			name = DefaultName;
		SetLabel(name);
	}

	static RHMQ* GetSetNamedInstance(const char* name, RHMQ* instance = nullptr)
	{
		static std::map<std::string, RHMQ*> Instances;
		auto find_it = Instances.find(name);
		auto existing = (find_it != Instances.end()) ? find_it->second : nullptr;

		if (instance && existing) {
			if (instance != existing)
				RHMQ_LOG_(existing, "SetInstance: An instance named \"%s\" already exists!", name);
			find_it->second = instance;
		}
		else if (instance && !existing) {
			Instances[name] = instance;
		}
		else if (existing) {
			instance = existing;
		}
		else {
			instance = new RHMQ(name);
			Instances[name] = instance;
		}
		return instance;
	}

	// Future work:
	// Add ZmqPoller functionality.
	// Add: 
	// bool PollAll(int timeout_ms, std::vector<void*> *ready) {
	//     ...
	//     auto rc = zmq_poller_wait_all(ZmqPoller, events.data(), events.size(), timeout_ms);
	//     <error check>
	//     for (auto &&event, events) { <populate ready with event's userdata> }
	//     return eventsRecieved == Sockets.size();
	// }
	//
	// Also would need to add functionality to RHMQ_Socket:
	// CreateSocket(const char* instance name = nullptr, void* userData)

	void *ZmqContext;

	std::vector<class RHMQ_Socket*> Sockets;
	// The Socket class itself takes care of Adding and Removing from this list.
	// When the Socket is detroyed, it will clean up it's own inclusion here.
	void AddSocket(class RHMQ_Socket* socket);
	void RemoveSocket(class RHMQ_Socket* s);

	static const unsigned int MaxNameLen = 64;
	static constexpr const char* DefaultName = "DEFAULT";
	char Name[MaxNameLen];
	void SetLabel(const char* label) { strncpy_s(Name, MaxNameLen, label, MaxNameLen - 1); }
};


//////////////////////////////////////////
// RHMQ_Context - Deprecated context wrapper
class RHMQ_Context
{
public:
	RHMQ_Context() : rhmqInstance(RHMQ::GetInstance()) {}

	// Needed by deprecated RHMQ_Socket constructor
	static RHMQ* GetRHMQInstance() {
		return GetSetInstance()->rhmqInstance;
	}

	RHMQDEPRECATE_BEGIN("** Deprecated Class: Use RHMQ instead **")
	static RHMQ_Context* GetInstance()
	RHMQDEPRECATE_END("** Deprecated Class: Use RHMQ instead **")
	{
		return GetSetInstance();
	}

	static void SetInstance(RHMQ_Context* instance) {
		GetSetInstance(instance);
	}

	uint32_t GetUnconnectedCount() {
		return GetSetInstance()->rhmqInstance->GetUnconnectedCount();
	}

	const std::vector<class RHMQ_Socket*>& GetSockets() const {
		return GetSetInstance()->rhmqInstance->GetSockets();
	}

private:
	RHMQ* rhmqInstance;
	static RHMQ_Context* GetSetInstance(RHMQ_Context* existing = nullptr)
	{
		static RHMQ_Context * StaticInstance;
		// This never happens, should go away when we remove deprecated code
		if (existing) {
			StaticInstance = existing;
		}
		if (!StaticInstance) {
			StaticInstance = new RHMQ_Context();
		}
		return StaticInstance;
	}
};

//////////////////////////////////////////
// RHMQ_Socket - ZMQ socket wrapper
// Must be created by RHMQ management
class RHMQ_Socket
{
private:
	RHMQ_Socket(const char* instanceName) :
		ZmqSocketPtr(0), ZmqSocketMonitor(0), ZmqFlags(0), InitFlags(NoFlags), LastAutoBuffer(0),
		Opened(false), Connected(false), TimeoutExpired(false), Timeout(RHMQ_TIMEOUT)
	{
		Context = RHMQ::GetInstance(instanceName);
		Address[0] = Address[MaxAddrLen - 1] = '\0';
		Label[0] = Label[MaxLabelLen - 1] = '\0';
	}

public:
	RHMQDEPRECATE_BEGIN("** Deprecated Constructor: use RHMQ::CreateSocket **")
	RHMQ_Socket(RHMQ_Context* InContext = nullptr, void* DEPRECATED = nullptr)
	RHMQDEPRECATE_END("** Deprecated Constructor: use RHMQ::CreateSocket **")
		: ZmqSocketPtr(0), ZmqSocketMonitor(0), ZmqFlags(0), InitFlags(NoFlags), LastAutoBuffer(0),
		Opened(false), Connected(false), TimeoutExpired(false), Timeout(RHMQ_TIMEOUT)
	{
		Context = InContext ? InContext->GetRHMQInstance() : RHMQ::GetInstance();
		Address[0] = Address[MaxAddrLen - 1] = '\0';
		Label[0] = Label[MaxLabelLen - 1] = '\0';
	}

	~RHMQ_Socket()
	{
		Close();
	}

	void Close()
	{
		if (ZmqSocketMonitor)
		{
			zmq_socket_monitor(ZmqSocketPtr, NULL, 0);
			zmq_close(ZmqSocketMonitor);
			ZmqSocketMonitor = NULL;
		}
		if (ZmqSocketPtr)
		{
			Context->RemoveSocket(this);
			if (Opened)
			{
				zmq_close(ZmqSocketPtr);
				zmq_msg_close(&ZmqMsg);
			}
			ZmqSocketPtr = NULL;
		}
		Opened = Connected = TimeoutExpired = false;
	}

	// Override default connection timeout for this instance only.
	// Needs to be called before first Send() to take effect.
	void SetTimeout(int timeout_seconds) { Timeout = timeout_seconds; }

	enum InitFlagsType
	{
		NoFlags     = 0,
		NoMonitor   = 1 << 0, // disable use of socket connection monitor (flag used by async services)
		DelayedOpen = 1 << 1, // connection is delayed until the first IsConnected() call
		PairClient  = 1 << 2, // Treat ZMQ_PAIR as a client instead of server
	};
	
	// Returns true on successful socket creation -- false, otherwise.
	// Does not wait for connection success. IsConnected(), or Send() are used for that.
	bool Init(int zmqType, const char *address, uint32_t flags = NoFlags)
	{
		if (ZmqSocketPtr || !address || !*address) { return false; }

		ZmqSocketType = zmqType;
		IsSendSocketType = (zmqType == ZMQ_PUB || zmqType == ZMQ_PUSH || zmqType == ZMQ_PAIR);
		strncpy_s(Address, MaxAddrLen, address, MaxAddrLen - 1);

		// Use address as default label
		if (!*Label)
		{
			strncpy_s(Label, MaxAddrLen, address, MaxLabelLen - 1);
		}
		
		InitFlags = flags;

		ZmqSocketPtr = zmq_socket(Context->GetZmqContext(), ZmqSocketType);
		if (!ZmqSocketPtr)
		{
			RHMQ_LOG("%s", zmq_strerror(zmq_errno()));
			return false;
		}

		// track errors in setting socket options
		int rc_sockopts = -1;

		// amount of time before send throws an UNCAUGHT error
		// (presume this will be picked up on error stream
		// and result in terminating sim)
		int send_timeout_millis = RHMQ_SEND_TIMEOUT_MILLIS;
		rc_sockopts = zmq_setsockopt(ZmqSocketPtr, ZMQ_SNDTIMEO,
						&send_timeout_millis, sizeof(send_timeout_millis));
		if(rc_sockopts < 0) { RHMQ_LOG("Could not set send timeout."); }

		// amount of time before receive throws an error which is ignored
		// and not treated as an error
		int recv_timeout_millis = RHMQ_RECV_TIMEOUT_MILLIS;
		rc_sockopts = zmq_setsockopt(ZmqSocketPtr, ZMQ_RCVTIMEO,
						&recv_timeout_millis, sizeof(recv_timeout_millis));
		if(rc_sockopts < 0) { RHMQ_LOG("Could not set receive timeout."); }

		// limit number of messages in outbound queue
		int send_buffer_msgs = RHMQ_SEND_BUFFER_MSGS;
		rc_sockopts = zmq_setsockopt(ZmqSocketPtr, ZMQ_SNDHWM, &send_buffer_msgs, sizeof(int));
		if(rc_sockopts < 0) { RHMQ_LOG("Could not set send message queue limit."); }

		// limit number of messages in inbound queue
		int recv_buffer_msgs = RHMQ_RECV_BUFFER_MSGS;
		rc_sockopts = zmq_setsockopt(ZmqSocketPtr, ZMQ_RCVHWM, &recv_buffer_msgs, sizeof(int));
		if(rc_sockopts < 0) { RHMQ_LOG("Could not set receive message queue limit."); }

		Context->AddSocket(this);

		return (InitFlags & DelayedOpen) ? true : Open();
	}

	bool SetMessageCount(uint32_t count)
	{
		if (!ZmqSocketPtr) {
			return false;
		}
		else {
			int rc_sockopts;

			int send_buffer_msgs = count;
			rc_sockopts = zmq_setsockopt(ZmqSocketPtr, ZMQ_SNDHWM, &send_buffer_msgs, sizeof(int));
			if (rc_sockopts < 0) { RHMQ_LOG("Could not set send message queue limit."); }

			// limit number of messages in inbound queue
			int recv_buffer_msgs = count;
			rc_sockopts = zmq_setsockopt(ZmqSocketPtr, ZMQ_RCVHWM, &recv_buffer_msgs, sizeof(int));
			if (rc_sockopts < 0) { RHMQ_LOG("Could not set receive message queue limit."); }
			return true;
		}
	}

	// Normally called during Init() to bind sockets, but may be postponed until first connection attempt
	bool Open()
	{
		if (!ZmqSocketPtr || Opened ) { return false; }
		Opened = true;

		// Create socket monitor inproc socket
		if (!(InitFlags & NoMonitor)) {
			InitSocketMonitor();
		}

		// Initialize an empty message buffer
		zmq_msg_init(&ZmqMsg);
		int rc = -1;
		switch (ZmqSocketType)
		{
		case ZMQ_SUB:
			zmq_setsockopt(ZmqSocketPtr, ZMQ_SUBSCRIBE, "", 0);
			//no break
		case ZMQ_PULL:
		case ZMQ_REQ:
			rc = zmq_connect(ZmqSocketPtr, Address);
			break;
		case ZMQ_PUB:
		case ZMQ_PUSH:
		case ZMQ_REP:
			rc = zmq_bind(ZmqSocketPtr, Address);
			break;
		default:
			break;
		}

		if (ZmqSocketType == ZMQ_PAIR)
		{
			if (InitFlags & PairClient)
				rc = zmq_connect(ZmqSocketPtr, Address);
			else
				rc = zmq_bind(ZmqSocketPtr, Address);
		}

		if (rc != 0)
		{
			RHMQ_LOG("%s", zmq_strerror(zmq_errno()));
			Close();
		}
		else
		{
			InitTime = std::chrono::system_clock::now();
		}
		return (rc == 0);
	}

	// Reinitialize socket - Useful to reset state of disconnected REQ/REP types
	bool ReInit()
	{
		if (!ZmqSocketPtr) { return false; }
		if (!Opened) { return (InitFlags & DelayedOpen) ? true : Open(); }
		Close();
		char addr[MaxAddrLen];
		strncpy_s(addr, MaxAddrLen, Address, MaxAddrLen);
		return Init(ZmqSocketType, addr, InitFlags);
	}

	const bool IsConnected(int timeout_ms = 0)
	{
		if (!ZmqSocketPtr) { return false; }
		if (!Connected && Timeout == -1) { return false; } // Never connect when Timeout is -1

		// Delay socket binding until connection is attempted
		if (!Opened && !Open()) { return false; }

		// Use socket monitoring API to get connection status
		if (!ProcessSocketMonitor(Connected ? 0 : timeout_ms) && !Connected)
		{
			// Otherwise, poll for sending sockets, and assume receive types are connected
			Connected = IsSendSocketType ? ZmqPoll(ZmqSocketPtr, timeout_ms, true) : true;
		}
		return Connected;
	}

	// Sends the packet using the supplied buffer.
	// Returns number of bytes sent successfully.
	// -1 is returned for fatal errors (Not connecting within timeout is considered fatal)
	int Send(const unsigned char *buf, int len)
	{
		if (!ZmqSocketPtr) { return -1; }

		// Use polling to test connection before calling send
		if (!Connected)
		{
			int timeout_ms = TimeoutExpired ? 0 : Timeout * 1000;
			if (!TimeoutExpired && Opened)
			{
				auto t = std::chrono::system_clock::now();
				auto ms = int(std::chrono::duration_cast<std::chrono::milliseconds>(t - InitTime).count());
				timeout_ms -= ms;
				if (timeout_ms < 0)
					timeout_ms = 0;
			}
			if (!IsConnected(timeout_ms)  && !TimeoutExpired)
			{
				RHMQ_LOG("Timeout waiting for socket \"%s\"", Address);
			}
			TimeoutExpired = true;
			if( !Connected )
			{
				// TODO: Set our errno for GetLastError to a timeout code
				return -1;
			}
		}

		InitMsgSend(buf, len);
		int nbytes = zmq_msg_send(&ZmqMsg, ZmqSocketPtr, ZmqFlags);
		if (nbytes == -1)
		{
			RHMQ_LOG("%s", zmq_strerror(zmq_errno()));
		}
		return nbytes;
	}

	// Receives the queued message into a preallocated buffer from the caller.
	// Returns the number of bytes received, or -1 for any fatal errors.
	// If BufferSize is too small, the remaining uncopied portion is lost.
	int Receive(unsigned char *buf, int len, int timeout_ms)
	{
		if (!ZmqSocketPtr) { return -1; }

		// Test for a valid connection, first
		if (!Connected && !IsConnected(timeout_ms)) { return 0; }

		// Poll for pending input data before calling receive
		if (timeout_ms != 0 && !ZmqPoll(ZmqSocketPtr, timeout_ms)) { return 0; }

		InitMsgReceive();
		int nbytes = zmq_msg_recv(&ZmqMsg, ZmqSocketPtr, ZmqFlags);
		if (nbytes > 0)
		{
			LastAutoBuffer = zmq_msg_data(&ZmqMsg);
			if (len > 0 && buf)
			{
				int bytesCopied = (nbytes > len) ? len : nbytes;
				memcpy((void*)buf, zmq_msg_data(&ZmqMsg), bytesCopied);
				if (bytesCopied < nbytes)
				{
					RHMQ_LOG("Receive buffer size too small. Needs to be %d bytes", nbytes);
					return -1;
				}
			}
		}
		if (nbytes == -1)
		{
			int err = zmq_errno();
			if(err == EAGAIN) {
				// timeout "error" which we do not treat as an error, just a non-receive
				nbytes = 0;
			} else {
				RHMQ_LOG("%s", zmq_strerror(zmq_errno()));
			}
		}
		return nbytes;
	}

	// Version of receive that allocates a receive buffer automatically.
	// Updates reference of supplied input buffer pointer. Do not free this buffer.
	int Receive(unsigned char* &bufPtr, int timeout_ms = 0)
	{
		int nbytes = Receive(0, 0, timeout_ms);
		// Update buffer pointer for callers not providing a preallocated one
		if (nbytes > 0)
		{
			bufPtr = (unsigned char*)LastAutoBuffer;
		}
		return nbytes;
	}

	// Buffer may have multiple messages, trash all but the last one
	// and return it.  Recieve() will trash the auto_buffer, so don't 
	// call it unless we know there is something there to get.
	int ReceiveLast(unsigned char* buffer, uint32_t length, int timeout_ms = 0)
	{
		int byteCount = 0;
		int finalByteCount = timeout_ms == 0 ? 0 : -1;// Receive(0, 0, 0);
		auto startTime = std::chrono::high_resolution_clock().now();
		auto currentTime = startTime;
		uint32_t currentTimeout_ms = timeout_ms;
		do {
			byteCount = 0;
			int events = 0;
			size_t sizeof_events = sizeof(int);
			int poll = zmq_getsockopt(ZmqSocketPtr, ZMQ_EVENTS, &events, &sizeof_events);
			if (poll == -1)
			{
				int error = zmq_errno();
				switch (error)
				{
				case EINVAL:
					RHMQ_LOG("Poll failed: option name, length, value, or size is invalid."); break;
				case ETERM:
					RHMQ_LOG("Poll failed: context was terminated."); break;
				case ENOTSOCK:
					RHMQ_LOG("Poll failed: socket invalid."); break;
				case EINTR:
					RHMQ_LOG("Poll failed: interrupted by signal."); break;
				default:
					RHMQ_LOG("Poll failed: unknown(%d)", error);
				}
			}
			if(events & ZMQ_POLLIN || currentTimeout_ms > 0)
			{
				byteCount = Receive(0, 0, currentTimeout_ms);
			}
			if (byteCount == (int)length) {
				finalByteCount = byteCount;
				// Look into avoiding this, maybe add a double buffer pattern to the Recieve function
				memcpy((void*)buffer, zmq_msg_data(&ZmqMsg), length);
			}
			if (timeout_ms) {
				currentTime = std::chrono::high_resolution_clock().now();
				int elapsed = int(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count());
				currentTimeout_ms = std::max(0, timeout_ms - elapsed);
			}
			// No timeout:
			//  If we got something, check for another
			// Yes timeout:
			//  If we don't have anything, wait for something
		} while ((currentTimeout_ms == 0 && byteCount > 0) || (currentTimeout_ms > 0 && finalByteCount < 0));
		return finalByteCount;
	}

	const char *GetAddress() const { return Address; }

	// Label socket with a descriptive name for log messages
	void SetLabel(const char *label) { strncpy_s(Label, MaxAddrLen, label, MaxLabelLen - 1); }
	const char *GetLabel() const { return Label; }

private:

	// Poll zmq socket for existence of any data ready to read.
	// If poll_out is true, return "ready to send" state.
	bool ZmqPoll(void *socket, int timeout_ms = 0, bool poll_out = false)
	{
		int pollEvent = poll_out ? ZMQ_POLLOUT : ZMQ_POLLIN;
		zmq_pollitem_t zmqPollItem;
		zmqPollItem.socket = socket;
		zmqPollItem.events = pollEvent;
		return zmq_poll(&zmqPollItem, 1, timeout_ms) == 1 && zmqPollItem.revents == pollEvent;
	}

	void InitMsgSend(const unsigned char *buf, int len)
	{
		if (len <= 0) { return; }
		zmq_msg_close(&ZmqMsg);
		zmq_msg_init_size(&ZmqMsg, len);
		memcpy(zmq_msg_data(&ZmqMsg), (void*)buf, len);
	}

	void InitMsgReceive()
	{
		if(zmq_msg_data(&ZmqMsg) != LastAutoBuffer)
		{
			zmq_msg_close(&ZmqMsg);
			zmq_msg_init(&ZmqMsg);
			LastAutoBuffer = zmq_msg_data(&ZmqMsg);
		}
	}

	static const unsigned int MaxAddrLen = 128;
	char Address[MaxAddrLen];
	static const unsigned int MaxLabelLen = 64;
	char Label[MaxLabelLen];
	int ZmqSocketType;
	bool IsSendSocketType;

	friend RHMQ;
	RHMQ* Context;
	void *ZmqSocketPtr;
	void *ZmqSocketMonitor;
	zmq_msg_t ZmqMsg;
	int ZmqFlags;
	uint32_t InitFlags; // Flags passed to Init() call
	void *LastAutoBuffer; // Avoid reallocation of auto-allocated zmq_msg_t

	bool Opened; // Socket has been opened (zmq_bind or zmq_connect called)
	bool Connected; // Initial connection happened (never goes back to false)
	bool TimeoutExpired; // Connection timeout occured, don't wait ever again
	int Timeout; // The initial timeout (in seconds) for this connection
	std::chrono::time_point<std::chrono::system_clock> InitTime;

	bool InitSocketMonitor()
	{
		ZmqSocketMonitor = NULL;
		char addr[MaxAddrLen];
		strcpy_s(addr, MaxAddrLen, "inproc");
		char *tcp_addr = strchr(Address, ':');
		if (!tcp_addr) { return false; }
		strcat_s(addr, MaxAddrLen - strlen(addr), tcp_addr);
		char *cp = strrchr(addr, ':');
		if (cp && (cp - addr > 6)) { *cp = '_'; }

		int rc = zmq_socket_monitor(ZmqSocketPtr, addr, ZMQ_EVENT_ALL);
		if (rc != 0) { return false; }

		ZmqSocketMonitor = zmq_socket(Context->GetZmqContext(), ZMQ_PAIR);
		if (!ZmqSocketMonitor) { return false; }

		rc = zmq_connect(ZmqSocketMonitor, addr);
		if (rc != 0) {
			zmq_close(ZmqSocketMonitor);
			ZmqSocketMonitor = 0;
		}

		return ZmqSocketMonitor != 0;
	}

	// Monitor socket for connection status
	bool ProcessSocketMonitor(int timeout_ms = 0)
	{
		if (!ZmqSocketMonitor) { return false; }

		// Make repeated message reuse easier
		class AutoZmqMsg
		{
		public:
			~AutoZmqMsg() { if (msg_data) zmq_msg_close(&msg); }

			int recv(void *socket, int flags = 0)
			{
				if (!msg_data || msg_data != zmq_msg_data(&msg))
				{
					if (msg_data) zmq_msg_close(&msg);
					zmq_msg_init(&msg);
				}
				msg_data = 0;
				int rc = zmq_msg_recv(&msg, socket, flags);
				if (rc <= 0)
					zmq_msg_close(&msg);
				else
					msg_data = static_cast<const char*>(zmq_msg_data(&msg));
				return rc;
			}

			const char *data() const { return msg_data; }

		private:
			zmq_msg_t msg;
			const char *msg_data = 0;

		};

		AutoZmqMsg eventMsg, addrMsg;

		auto t0 = std::chrono::system_clock::now();

		while (ZmqPoll(ZmqSocketMonitor, timeout_ms))
		{
			int rc = eventMsg.recv(ZmqSocketMonitor);
			if (rc >= 0)
				rc = addrMsg.recv(ZmqSocketMonitor);
			if (rc == -1 && zmq_errno() == ETERM)
			{
				ZmqSocketMonitor = 0;
				break;
			}

			const char *data = eventMsg.data();
			unsigned short monitor_event;
			int monitor_value;
			memcpy(&monitor_event, data, sizeof(uint16_t)); data += sizeof(uint16_t);
			memcpy(&monitor_value, data, sizeof(int32_t));
			//RHMQ_LOG("SocketMonitor event address: %s", addrMsg.data());

			switch (monitor_event)
			{
			case ZMQ_EVENT_ACCEPTED:
			case ZMQ_EVENT_CONNECTED:
				Connected = true;
				timeout_ms = 0;
				break;

			case ZMQ_EVENT_DISCONNECTED:
				Connected = false;
				break;

			case ZMQ_EVENT_MONITOR_STOPPED:
				ZmqSocketMonitor = 0;
				return false;
				break;

			default:
				RHMQ_LOG("Unknown event: %d",monitor_event);

			case ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL:
			case ZMQ_EVENT_HANDSHAKE_SUCCEEDED:
			case ZMQ_EVENT_LISTENING:
			case ZMQ_EVENT_CLOSED:
			case ZMQ_EVENT_CONNECT_RETRIED:
			case ZMQ_EVENT_CONNECT_DELAYED:
				break;
			}

			// Adjust timeout value for next poll
			if (timeout_ms > 0)
			{
				auto t = std::chrono::system_clock::now();
				auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count();
				timeout_ms -= int(ms);
				if (timeout_ms < 0)
					timeout_ms = 0;
				t0 = t;
			}
		}

		return ZmqSocketMonitor != 0;
	}

};


//////////////////////////////////////////
// Further RHMQ implementation

inline int RHMQ::GetUnconnectedCount()
{
	int num_unconnected = 0;
	for (auto const &s : Sockets)
	{
		if (!s->IsConnected()) { ++num_unconnected; }
	}
	return num_unconnected;
}

inline RHMQ_Socket* RHMQ::CreateSocket(const char* instanceName)
{
	return new RHMQ_Socket(instanceName);
}	

inline void RHMQ::AddSocket(class RHMQ_Socket* socket)
{
	Sockets.push_back(socket);
}

inline void RHMQ::RemoveSocket(class RHMQ_Socket* s) {
	auto it = std::find(Sockets.begin(), Sockets.end(), s);
	if (it != Sockets.end())
	{
		Sockets.erase(it);
	}
}
#endif // !RHMQ_H
