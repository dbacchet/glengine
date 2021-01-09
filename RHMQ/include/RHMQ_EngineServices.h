#pragma once

#include "RHMQ.h"
#include "generated/engine_services_generated.h"

#include <list>

//#define LOG_STDOUT

class RHMQ_EngineServices
{
public:
	RHMQ_EngineServices(const char* endpoint_address = nullptr)
	{
		if (endpoint_address) {
			Init(endpoint_address);
		}
#ifdef LOG_STDOUT
		SimStateCB = [](RHMsg::SimState state, double time) {
			fprintf(stdout, "SimState: %s Time: %0.3lf\n", RHMsg::EnumNameSimState(state), time);
		};
#endif
	}

	virtual ~RHMQ_EngineServices()
	{
		if (socket) { delete socket; }
	}

	bool Init(const char* endpoint_address)
	{
		if (socket) { delete socket; }
		socket = RHMQ::CreateSocket();
		if (!socket->Init(ZMQ_REQ, endpoint_address)) {
			socket = nullptr;
		}
		return socket != nullptr;
	}

	bool IsValid() { return socket != nullptr; }

	bool IsConnected(int ms = 0)
	{
		const bool last_connected = connected;
		connected = socket && socket->IsConnected(ms);
		if (connected ^ last_connected) {
			pending_reply = false;
			if (connected && pending_requests.empty()) {
				SendSimStateRequest(RHMsg::SimStateCommand_Query);
				NextState = RHMsg::SimState_Idle;
			}
			if (!connected) {
				socket->ReInit();
				SimState = RHMsg::SimState_Busy;
				if(pending_requests.empty())
					NextState = RHMsg::SimState_Busy;
			}
		}
		return connected;
	}

	// Handles sending requests and receiving replies
	bool Process(int timeout_ms = 0)
	{
		const bool last_connected = connected;
		if (!IsValid() || !IsConnected(timeout_ms)) { return last_connected; }
		bool changed_state = !last_connected;

		if (!pending_reply && !pending_requests.empty()) {
			pending_reply = true;
			auto& msg = pending_requests.front();
			socket->Send((unsigned char*)msg.data(), (int)msg.size());
			pending_requests.pop_front();
		}

		unsigned char* reply = nullptr;
		if (pending_reply && socket->Receive(reply, timeout_ms) > 0) {
			pending_reply = false;
			changed_state = true;
			if (auto msg = RHMsg::GetEngineService(reply)) {
				if (auto m = msg->service_as_SimStateReply()) {
					SimState = m->state();
					if (NextState == SimState) {
						NextState = RHMsg::SimState_Busy;
					}
					else if (NextState == RHMsg::SimState_Idle && SimState == RHMsg::SimState_Active) {
						// Connect to an already active engine
						NextState = RHMsg::SimState_Busy;
					}
					else if (NextState != RHMsg::SimState_Busy) {
						SendSimStateRequest(RHMsg::SimStateCommand_Query);
					}
					if (SimStateCB)
						SimStateCB(m->state(), m->time());
				}
			}
		}
		return changed_state;
	}

public:

	// Possible states:
	//  RHMsg::SimState_Busy = 0,
	//  RHMsg::SimState_Idle = 1,
	//  RHMsg::SimState_Active = 2,
	//  RHMsg::SimState_Paused = 3,

	RHMsg::SimState GetSimState() const { return SimState; }

	bool ResetSim() {
		if (SimState == RHMsg::SimState_Active && !pending_reply) {
			SendSimStateRequest(RHMsg::SimStateCommand_Reset);
			NextState = RHMsg::SimState_Idle;
			//Process();
			return true;
		}
		return false;
	}

	bool StartSim() {
		if (SimState == RHMsg::SimState_Idle && !pending_reply) {
			SendSimStateRequest(RHMsg::SimStateCommand_Start);
			NextState = RHMsg::SimState_Active;
			//Process();
			return true;
		}
		return false;
	}

protected:

	// Possible commands:
	//  RHMsg::SimStateCommand_Query
	//  RHMsg::SimStateCommand_Start
	//  RHMsg::SimStateCommand_Pause
	//  RHMsg::SimStateCommand_Reset

	void SendSimStateRequest(RHMsg::SimStateCommand command)
	{
#ifdef LOG_STDOUT
		fprintf(stdout, "SetSimState: %s\n", RHMsg::EnumNameSimStateCommand(command));
#endif
		flatbuffers::FlatBufferBuilder fbb;
		auto service = RHMsg::CreateSimStateRequest(fbb, command);
		auto request = RHMsg::CreateEngineService(fbb, RHMsg::EngineServiceType_SimStateRequest, service.Union());
		fbb.Finish(request);
		pending_requests.push_back(std::string((char*)fbb.GetBufferPointer(), fbb.GetSize()));
	}

	void SetSimStateCB(std::function<void(RHMsg::SimState, double)> callback) {
		SimStateCB = callback;
	}

private:
	RHMQ_Socket* socket = nullptr;
	bool connected = false;
	bool pending_reply = false;

	// Simple requests (without reply callbacks)
	std::list<std::string> pending_requests;

	std::function<void(RHMsg::SimState, double)> SimStateCB = nullptr;
	RHMsg::SimState SimState = RHMsg::SimState_Busy;
	RHMsg::SimState NextState = RHMsg::SimState_Busy;
};
