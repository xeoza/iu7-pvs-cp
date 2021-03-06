/*   -*- buffer-read-only: t -*- vi: set ro:
 *
 *  DO NOT EDIT THIS FILE   (client-fsm.c)
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
#define DEFINE_FSM
#include "../include/client-fsm.h"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */
#include "../include/client_fsm_handlers.h"
#include <stdlib.h>
/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

/**
 *  Enumeration of the valid transition types
 *  Some transition types may be common to several transitions.
 */
typedef enum {
    CLIENT_FSM_TR_CONNECT,
    CLIENT_FSM_TR_DATA,
    CLIENT_FSM_TR_EHLO,
    CLIENT_FSM_TR_ERROR,
    CLIENT_FSM_TR_FINISH,
    CLIENT_FSM_TR_INVALID,
    CLIENT_FSM_TR_MF,
    CLIENT_FSM_TR_QUIT,
    CLIENT_FSM_TR_RT
} te_client_fsm_trans;
#define CLIENT_FSM_TRANSITION_CT  9

/**
 *  State transition handling map.  Map the state enumeration and the event
 *  enumeration to the new state and the transition enumeration code (in that
 *  order).  It is indexed by first the current state and then the event code.
 */
typedef struct client_fsm_transition t_client_fsm_transition;
struct client_fsm_transition {
    te_client_fsm_state  next_state;
    te_client_fsm_trans  transition;
};
static const t_client_fsm_transition
client_fsm_trans_table[ CLIENT_FSM_STATE_CT ][ CLIENT_FSM_EVENT_CT ] = {

  /* STATE 0:  CLIENT_FSM_ST_INIT */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_INITIALIZED, CLIENT_FSM_TR_CONNECT }, /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_ERROR },  /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  },


  /* STATE 1:  CLIENT_FSM_ST_INITIALIZED */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_S_EHLO, CLIENT_FSM_TR_EHLO },   /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_ERROR },  /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  },


  /* STATE 2:  CLIENT_FSM_ST_S_EHLO */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_S_MF, CLIENT_FSM_TR_MF },       /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_ERROR },  /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  },


  /* STATE 3:  CLIENT_FSM_ST_S_MF */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_S_RT, CLIENT_FSM_TR_RT },       /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_ERROR },  /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  },


  /* STATE 4:  CLIENT_FSM_ST_S_RT */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_S_DATA, CLIENT_FSM_TR_DATA },   /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_ERROR },  /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  },


  /* STATE 5:  CLIENT_FSM_ST_S_DATA */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_S_QUIT, CLIENT_FSM_TR_QUIT },   /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_ERROR },  /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  },


  /* STATE 6:  CLIENT_FSM_ST_S_MAIL */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  OK */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  BAD */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  ERROR */
  },


  /* STATE 7:  CLIENT_FSM_ST_S_QUIT */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_FINISH }, /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_ERROR },  /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  },


  /* STATE 8:  CLIENT_FSM_ST_FINISH */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  OK */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  BAD */
    { CLIENT_FSM_ST_FINISH, CLIENT_FSM_TR_FINISH }, /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  ERROR */
  },


  /* STATE 9:  CLIENT_FSM_ST_S_ERROR */
  { { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  CONNECTED */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  OK */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  BAD */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }, /* EVT:  CONNECTION_LOST */
    { CLIENT_FSM_ST_S_ERROR, CLIENT_FSM_TR_ERROR }  /* EVT:  ERROR */
  }
};


#define Client_FsmFsmErr_off     19
#define Client_FsmEvInvalid_off  75
#define Client_FsmStInit_off     83


static char const zClient_FsmStrings[192] =
/*     0 */ "** OUT-OF-RANGE **\0"
/*    19 */ "FSM Error:  in state %d (%s), event %d (%s) is invalid\n\0"
/*    75 */ "invalid\0"
/*    83 */ "init\0"
/*    88 */ "initialized\0"
/*   100 */ "s_ehlo\0"
/*   107 */ "s_mf\0"
/*   112 */ "s_rt\0"
/*   117 */ "s_data\0"
/*   124 */ "s_mail\0"
/*   131 */ "s_quit\0"
/*   138 */ "finish\0"
/*   145 */ "s_error\0"
/*   153 */ "connected\0"
/*   163 */ "ok\0"
/*   166 */ "bad\0"
/*   170 */ "connection_lost\0"
/*   186 */ "error";

static const size_t aszClient_FsmStates[10] = {
    83,  88,  100, 107, 112, 117, 124, 131, 138, 145 };

static const size_t aszClient_FsmEvents[6] = {
    153, 163, 166, 170, 186, 75 };


#define CLIENT_FSM_EVT_NAME(t)   ( (((unsigned)(t)) >= 6) \
    ? zClient_FsmStrings : zClient_FsmStrings + aszClient_FsmEvents[t])

#define CLIENT_FSM_STATE_NAME(s) ( (((unsigned)(s)) >= 10) \
    ? zClient_FsmStrings : zClient_FsmStrings + aszClient_FsmStates[s])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

static int client_fsm_invalid_transition( te_client_fsm_state st, te_client_fsm_event evt );

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * */
/**
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
static int
client_fsm_invalid_transition( te_client_fsm_state st, te_client_fsm_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    char const * fmt = zClient_FsmStrings + Client_FsmFsmErr_off;
    fprintf( stderr, fmt, st, CLIENT_FSM_STATE_NAME(st), evt, CLIENT_FSM_EVT_NAME(evt));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}

/**
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  CLIENT_FSM_ST_DONE
 *  or CLIENT_FSM_ST_INVALID, it resets to
 *  CLIENT_FSM_ST_INIT and returns CLIENT_FSM_ST_INIT.
 */
te_client_fsm_state
client_fsm_step(
    te_client_fsm_state client_fsm_state,
    te_client_fsm_event trans_evt,
    void* connection,
    void* writeFS )
{
    te_client_fsm_state nxtSt;
    te_client_fsm_trans trans;

    if ((unsigned)client_fsm_state >= CLIENT_FSM_ST_INVALID) {
        return CLIENT_FSM_ST_INIT;
    }

#ifndef __COVERITY__
    if (trans_evt >= CLIENT_FSM_EV_INVALID) {
        nxtSt = CLIENT_FSM_ST_INVALID;
        trans = CLIENT_FSM_TR_INVALID;
    } else
#endif /* __COVERITY__ */
    {
        const t_client_fsm_transition * ttbl =
            client_fsm_trans_table[ client_fsm_state ] + trans_evt;
        nxtSt = ttbl->next_state;
        trans = ttbl->transition;
    }


    switch (trans) {
    case CLIENT_FSM_TR_CONNECT:
        /* START == CONNECT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_CONNECT(nxtSt, connection, writeFS);
        /* END   == CONNECT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_DATA:
        /* START == DATA == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_DATA(nxtSt, connection, writeFS);
        /* END   == DATA == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_EHLO:
        /* START == EHLO == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_EHLO(nxtSt, connection, writeFS);
        /* END   == EHLO == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_ERROR:
        /* START == ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_ERROR(nxtSt, connection, writeFS);
        /* END   == ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_FINISH:
        /* START == FINISH == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_FINISH(nxtSt, connection, writeFS);
        /* END   == FINISH == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_INVALID:
        /* START == INVALID == DO NOT CHANGE THIS COMMENT */
        exit(client_fsm_invalid_transition(client_fsm_state, trans_evt));
        /* END   == INVALID == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_MF:
        /* START == MF == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_MF(nxtSt, connection, writeFS);
        /* END   == MF == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_QUIT:
        /* START == QUIT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_QUIT(nxtSt, connection, writeFS);
        /* END   == QUIT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_RT:
        /* START == RT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_RT(nxtSt, connection, writeFS);
        /* END   == RT == DO NOT CHANGE THIS COMMENT */
        break;


    default:
        /* START == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
        exit(client_fsm_invalid_transition(client_fsm_state, trans_evt));
        /* END   == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
    }


    /* START == FINISH STEP == DO NOT CHANGE THIS COMMENT */
    /* END   == FINISH STEP == DO NOT CHANGE THIS COMMENT */

    return nxtSt;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of client-fsm.c */
