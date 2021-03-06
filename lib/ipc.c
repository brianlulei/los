// User-level IPC library routines

#include <include/lib.h>

/* Receive a vlaue via IPC and return it.
 * If 'pg' is nonnull, then any page sent by the sender will be mapped at
 *		that address.
 * If 'from_env_store' is nonnull, then store the IPC sender's envid in
 *		*from_env_store.
 * If 'perm_store' is nonnull, then store the IPC sender's page permission
 *		in *perm_store (this is nonzero if a page was successfully
 *		transfered to 'pg').
 * If the system call fails, then store 0 in *fromenv and *perm (if
 *		they're nonnull) and return the error.
 * Otherwise, return the value sent by the sender
 *
 * Hint:
 *	Use 'thisenv' to discover the value and who sent it.
 *	If 'pg' is null, pass sys_ipc_recv a value that it will understand
 *	as meaning "no page". (Zero is not the right value, since that's
 *	a perfectly valid place to map a page.)
 */
int32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
	int r;
	// Send recv system call
	if (pg)
		r = sys_ipc_recv(pg);
	else
		r = sys_ipc_recv((void*) UTOP);

	if (r < 0) {
		if (from_env_store)
			*from_env_store = 0;

		if (perm_store)
			*perm_store = 0;

		return r;
	}

	// At this point, thisenv is blocked, waiting for an env to send msg to
	// it and change ENV_STATUS. So it can be scheduled.
	if (from_env_store)
		*from_env_store = thisenv->env_ipc_from;

	if (perm_store)
		*perm_store = thisenv->env_ipc_perm;

	return thisenv->env_ipc_value;
}

/* Send 'val' (and 'pg' with 'perm', if 'pg' is nonnull) to 'toenv'.
 * This function keeps trying until it succeeds.
 * It should panic on any error that other than -E_IPC_NOT_RECV.
 *
 * Hint:
 *	Use sys_yield() to be CPU-Friendly.
 *	If 'pg' is null, pass sys_ipc_try_send a value that it will understand
 *	as meaning "no page". (Zero is not the right value) 
 */
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
	int r = -E_IPC_NOT_RECV;
	while (r < 0) {
		if (r != -E_IPC_NOT_RECV)
			panic("ipc_send : sys_ipc_try_send error : %e.\n", r);

		if (pg)
			r = sys_ipc_try_send(to_env, val, pg, perm);
		else
			r = sys_ipc_try_send(to_env, val, (void*)UTOP, perm);
	}
}

/* Find the first environment of the given type. We'll use this to
 * find special environment.
 * Returns 0 if no such envirinment exists.
 */
envid_t
ipc_find_env(enum EnvType type)
{
	int i;
	for (i = 0; i < NENV; i++)
		if (envs[i].env_type == type)
			return envs[i].env_id;
	return 0;
}
