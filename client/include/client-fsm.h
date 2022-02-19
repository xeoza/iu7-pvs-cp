/*   -*- buffer-read-only: t -*- vi: set ro:
 *
 *  DO NOT EDIT THIS FILE   (client-fsm.h)
 *
 *  It has been AutoGen-ed
 *  From the definitions    client.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (C) 1992-2015 Bruce Korb - all rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Bruce Korb'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  This file enumerates the states and transition events for a FSM.
 *
 *  te_client_fsm_state
 *      The available states.  FSS_INIT is always defined to be zero
 *      and FSS_INVALID and FSS_DONE are always made the last entries.
 *
 *  te_client_fsm_event
 *      The transition events.  These enumerate the event values used
 *      to select the next state from the current state.
 *      CLIENT_FSM_EV_INVALID is always defined at the end.
 */
#ifndef AUTOFSM_CLIENT_FSM_H_GUARD
#define AUTOFSM_CLIENT_FSM_H_GUARD 1
/**
 *  Finite State machine States
 *
 *  Count of non-terminal states.  The generated states INVALID and DONE
 *  are terminal, but INIT is not  :-).
 */
#define CLIENT_FSM_STATE_CT  10
typedef enum {
    CLIENT_FSM_ST_INIT,        CLIENT_FSM_ST_INITIALIZED,
    CLIENT_FSM_ST_S_EHLO,      CLIENT_FSM_ST_S_MF,
    CLIENT_FSM_ST_S_RT,        CLIENT_FSM_ST_S_DATA,
    CLIENT_FSM_ST_S_MAIL,      CLIENT_FSM_ST_S_QUIT,
    CLIENT_FSM_ST_FINISH,      CLIENT_FSM_ST_S_ERROR,
    CLIENT_FSM_ST_INVALID,     CLIENT_FSM_ST_DONE
} te_client_fsm_state;

/**
 *  Finite State machine transition Events.
 *
 *  Count of the valid transition events
 */
#define CLIENT_FSM_EVENT_CT 5
typedef enum {
    CLIENT_FSM_EV_CONNECTED,       CLIENT_FSM_EV_OK,
    CLIENT_FSM_EV_BAD,             CLIENT_FSM_EV_CONNECTION_LOST,
    CLIENT_FSM_EV_ERROR,           CLIENT_FSM_EV_INVALID
} te_client_fsm_event;

/**
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  CLIENT_FSM_ST_DONE or CLIENT_FSM_ST_INVALID, it resets to
 *  CLIENT_FSM_ST_INIT and returns CLIENT_FSM_ST_INIT.
 */
extern te_client_fsm_state
client_fsm_step(
    te_client_fsm_state client_fsm_state,
    te_client_fsm_event trans_evt,
    void* connection,
    void* writeFS );

#endif /* AUTOFSM_CLIENT_FSM_H_GUARD */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of client-fsm.h */
