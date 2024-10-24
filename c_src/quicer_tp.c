#define TRACEPOINT_CREATE_PROBES
#define TRACEPOINT_DEFINE
#include "quicer_tp.h"

extern uint64_t CxPlatTimeUs64(void);

// help macro to copy atom to env for debug emulator assertions
#define ATOM_IN_ENV(X) enif_make_copy(env, ATOM_##X)

void
tp_snk(ErlNifEnv *env,
       const char *ctx,
       const char *fun,
       const char *tag,
       uint64_t rid,
       uint64_t mark)
{
  ErlNifPid pid;
  if (enif_whereis_pid(env, ATOM_SNABBKAFFE_COLLECTOR, &pid))
    {
      ERL_NIF_TERM snk_event;
      ERL_NIF_TERM snk_event_key_array[7]
          = { ATOM_IN_ENV(SNK_KIND),    ATOM_IN_ENV(CONTEXT),
              ATOM_IN_ENV(FUNCTION),    ATOM_IN_ENV(TAG),
              ATOM_IN_ENV(RESOURCE_ID), ATOM_IN_ENV(MARK),
              ATOM_IN_ENV(SNK_META) };

      ERL_NIF_TERM snk_evt_meta;
      ERL_NIF_TERM snk_evt_meta_key_array[1] = { ATOM_IN_ENV(TIME) };
      ERL_NIF_TERM snk_evt_meta_val_array[1]
          = { enif_make_uint64(env, CxPlatTimeUs64()) };

      // shall never fail
      enif_make_map_from_arrays(env,
                                snk_evt_meta_key_array,
                                snk_evt_meta_val_array,
                                1,
                                &snk_evt_meta);

      ERL_NIF_TERM snk_event_val_array[7] = {
        ATOM_IN_ENV(DEBUG),                         // snk_kind
        enif_make_string(env, ctx, ERL_NIF_LATIN1), // context
        enif_make_string(env, fun, ERL_NIF_LATIN1), // fun
        enif_make_string(env, tag, ERL_NIF_LATIN1), // tag
        enif_make_uint64(env, rid),                 // rid
        enif_make_uint64(env, mark),                // mark
        snk_evt_meta                                // snk_meta
      };

      enif_make_map_from_arrays(
          env, snk_event_key_array, snk_event_val_array, 7, &snk_event);

      ERL_NIF_TERM report = enif_make_tuple2(
          env,
          ATOM_IN_ENV(GEN_CAST),
          enif_make_tuple2(env, ATOM_IN_ENV(TRACE), snk_event));
      enif_send(NULL, &pid, NULL, report);
    }
}
