/*
**Copyright (c) 2004 Ingres Corporation
**
*/

/*
NO_OPTIM = rs4_us5 r64_us5
*/

#include    <compat.h>
#include    <gl.h>
#include    <cs.h>
#include    <di.h>
#include    <er.h>
#include    <me.h>
#include    <pc.h>
#include    <iicommon.h>
#include    <dbdbms.h>
#include    <dmf.h>
#include    <ulf.h>
#include    <dm.h>
#include    <lg.h>
#include    <lgdef.h>
#include    <lgdstat.h>
#include    <lgkdef.h>

/**
**
**  Name: LGBEGIN.C - Implements the LGbegin function of the logging system
**
**  Description:
**	This module contains the code which implements LGbegin.
**	
**	    LGbegin -- begin a transaction in the logging system
**	    LGconnect -- Connect to an existing log context.
**
**  History:
**	Summer, 1992 (bryanp)
**	    Working on the new portable logging and locking system.
**	29-sep-1992 (nandak)
**	    Use one common transaction ID type.
**	21-oct-1992 (bryanp)
**	    Improve error messages when resources are exhausted.
**	05-jan-1993 (jnash)
**	    Reduced logging project.  Init lxb_reserved_space.
**	15-mar-1993 (rogerk)
**	    Reduced Logging - Phase IV:
**	    Added LG_NORESERVE option - used by recovery transactions which
**		will write log records without reserving space for them.
**	17-mar-1993 (rogerk)
**	    Moved lgd status values to lgdstat.h so that they are visible
**	    by callers of LGshow(LG_S_LGSTS).
**	26-apr-1993 (andys)
**	    Cluster 6.5 Project I
**	    Renamed stucture members of LG_LA to new names. This means
**	    replacing lga_high with la_sequence, and lga_low with la_offset.
**	14-jul-93 (ed)
**	    replacing <dbms.h> by <gl.h> <sl.h> <iicommon.h> <dbdbms.h>
**	23-aug-1993 (bryanp)
**	    Initialize lxb_wait_reason.
**	18-oct-1993 (rogerk)
**	    Removed unused lgd_dmu_cnt, lxb_dmu_cnt fields.
**	04-Jan-1994 (chiku)
**	    Initialize sys_err in LGbegin():
**	    Added CL_ERR_CLEAR() in LGbegin() [BUG#: 57803].
**	05-Jan-1995 (jenjo02)
**	    Mutex granularity project. Semaphores must now be
**	    explicitly named in calls to LGMUTEX.C
**	    Added many semaphores to reduce blocking effects
**	    of single lgd_mutex.
**	18-Jan-1996 (jenjo02)
**          lbk_table replaced with lfbb_table (LFBs) and ldbb_table
**          (LDBs) to ensure that semaphores now embedded in those
**          structures are not corrupted. Similarly, sbk_table 
**          replaced with lpbb_table (LPBs), lxbb_table (LXBs), and
**          lpdb_table (LPDs).
**      22-nov-96 (stial01,dilma04)
**          Row Locking Project:
**          LG_begin() Reserve tran id 0,0
**	13-Dec-1996 (jenjo02)
**	    To avoid deadlocking with LK, don't allow new protected update 
**	    transactions to begin while stalling a DB for checkpoint.
**	    Bug 67888, previously addressed by defining a new lock type
**	    of LK_CKP_TXN and taking that lock each time a transaction
**	    or CKPDB starts. The LK_CKP_TXN lock has proven to be very
**	    contentious, so this solution is offered in its place.
**
**	    When a checkpoint is beginning on a database, delay the 
**	    checkpoint until all protected update transactions (active or
**	    inactive) have completed. At the same time, don't allow
**	    new update transactions to start until the stall condition
**	    has cleared. The fledging LXBs are allocated, left
**	    incomplete, and suspended. When the stall condition clears,
**	    the waiting LXBs are resumed, completed, and placed on
**	    the inactive queue.
**      27-feb-1997 (stial01)
**          Reserve tranid (0,anything) so that unique ids generated by LK
**          do not conflict with LG generated transaction ids
**	13-Nov-1997 (jenjo02)
**	    Modified 13-Dec-1996 change. Newly starting read-write txns
**	    will be stalled if online checkpoint has indicated that they
**	    should via the LDB_STALL_RW flag.
**    08-Dec-1997 (hanch04)
**        Initialize new block field in LG_LA for support of logs > 2 gig
**	17-Dec-1997 (jenjo02)
**	    Made queue of logwriter LXBs static instead of a "wait" queue.
**	24-Aug-1998 (jenjo02)
**	    lxb_tran_id now embedded in (new) LXH structure.
**	16-Nov-1998 (jenjo02)
**	    Cross-process thread identity changed to CS_CPID structure
**	    from PID and SID.
**	15-Mar-1998 (jenjo02)
**	    Stalls for CKPDB no longer happen in here, but are now 
**	    detected in LGwrite(), LGreserve().
**	24-Aug-1999 (jenjo02)
**	    New function, LG_allocate_lxb(), now computes transaction id,
**	    obviating the need to take lfb_mutex to do it inline.
**	    Removed LXB inactive queue and its mutex. Inactive LXBs now
**	    don't appear on any queue until they become active.
**	30-Sep-1999 (jenjo02)
**	    Added lxb_first_lsn for LG_S_OLD_LSN function
**	15-Dec-1999 (jenjo02)
**	    Added support for SHARED transactions.
**	14-Mar-2000 (jenjo02)
**	    Removed static LG_begin() function and the unnecessary
**	    level of indirection it induced.
**	21-jan-1999 (hanch04)
**	    replace nat and longnat with i4
**	31-aug-2000 (hanch04)
**	    cross change to main
**	    replace nat and longnat with i4
**      23-Nov-2002  (hweho01)
**          Turned off optimization for AIX build.
**          Got "E_DMA413_LGBEGIN_BAD_DB" error during formatting the
**          primary transaction log file.
**          The compiler is VisualAge 5.023.
**      23-Jul-2003 (hweho01)
**          Turned off optimization for AIX 64-bit build.
**          Got "E_DMA413_LGBEGIN_BAD_DB" error during formatting the
**          log file. 
**          Compiler : VisualAge 5.023.
**	17-Dec-2003 (jenjo02)
**	    Added support for explicit LKconnect() functionality.
**      7-oct-2004 (thaju02)
**          Use SIZE_TYPE to allow memory pools > 2Gig.
**	6-Apr-2007 (kschendel) SIR 122890
**	    Use an intermediate for aliasing external i4 id's and LG_ID.
**	    Originally this was to fix gcc -O2's strict aliasing, but
**	    Ingres has so many other strict-aliasing violations that
**	    -fno-strict-aliasing is needed.  The id/LG_ID thing was so
**	    egregious, though, that I've kept this change.
**	09-Dec-2008 (jonj)
**	    SIR 120874: use new form uleFormat, CL_CLEAR_ERR.
*/

/*
** Forward declarations of static functions:
*/

/*{
** Name: LGbegin	- Begin transaction.
**
** Description:
**      Begin a transaction and return an external transaction identifier
**	and an internal transaction id.  The internal transaction identifier
**	is used in LGwrite calls to associate records from the same transaction.
**
**      This routine informs the logging system to begin a transaction
**	against a particular database.  The logging system initializes
**	information about the transaction and returns a transaction 
**	identifier which is used in calls to LGwrite.
**
**      The transaction can be marked as being journaled.  This implies
**	that the log record for this transaction should be copied to the
**	journal file.
**
**	The transaction can also be marked as not-protected.  A normal
**	transaction is marked as protected.  Protected means that on abnormal
**	termination the transaction is considered to be outstanding and must
**	be terminated by the recovery mechanism before the logging system
**	will delete information about the transaction.  A non-protected
**	transaction has all of its logging system information deleted
**	when the transaction is terminated abnormally.  As an example:
**	read-only transactions do not need to be protected.
**
**	If the LG_NORESERVE flag is passed, the transaction is allowed to
**	write log records without pre-reserving logspace for them.  The
**	transaction is expected to either guarantee through other methods
**	methods that space will exist or that hitting logfull is non-disastrous
**	(for example if the transaction is NOPROTECT and requires no space
**	to be reserved for abort processing).
**
** Shared Log Transaction Notes:
**
**      Shared Log Transactions are LXBs that can be referenced
**	concurrently by multiple processes.  They are identified by a
**	lxb_status of LXB_SHARED.
**
**	Currently, shared LXBs are implicitly created for XA transactions
**	or can be created explicitly via LGconnect().
**
**	When a Shared LXB is indicated, two LXBs are actually created:
**
**	    - The actual shared LXB is created - this is the LXB that
**	      all log records will be held on.  All of the information regarding
**	      the inactive/active state of the transaction, associated
**	      log records, LSNs, LGAs, etc, are kept on this LXB.
**
**	    - A second LXB is created that is used as the creating
**	      thread's handle to the shared LXB.  Its status type is
**	      LXB_SHARED_HANDLE and its purpose is mainly to point to the
**	      shared LXB.  No log records are associated with this LXB, and
**	      its status is always unPROTECT. The lxb_cpid is set to the value
**	      of the creating thread. The lxb_shared_lxb fields holds a pointer
**	      to the shared LXB.
**
**	The LXB id of the LXB_SHARED lxb is returned to the
**	caller as the log id.  All log requests should be made on
**	this log id (the actual id of the shared LXB is unknown to the
**	calling thread).
**
**	When a thread connects to an already existing shared LXB, an
**	LXB_SHARED_HANDLE lxb is created and pointed at the shared LXB.
**	As in the create shared call, the id returned to the caller is the
**	id of the handle LXB.
**
**	All routines which receive requests on a log id of the type
**	LXB_SHARED_HANDLE will handle the request as though
**	it were made on the shared log id.  If the thread must be suspended then
**	the cpid parm is used from the SHARED_HANDLE lxb.
**
**	The lxb_handle_count of the shared LXB indicates the number of
**	handles that are connected to the shared LXB. A queue of handles is
**	maintained from the shared LXB. Commit/rollback/abort of any of the
**	handles must be atomic with respect to the others; abort of one must abort all.
**	Each time an LXB_SHARED_HANDLE lxb is deallocated, the reference count 
**	of the shared LXB mustbe decremented.  When the last handle is released, 
**	the shared LXB is released.
**
** Inputs:
**	flag				Either zero or any combination of
**					    LG_NOPROTECT: Not recovered 
**					    LG_READONLY: Readonly transactions.
**					    LG_JOURNAL: Journaled transaction.
**					    LG_NOABORT: Don't pick this 
**						transaction for force abort.
**					    LG_NORESERVE: Transaction will log
**						updates without reserving space
**	db_id				Database identifier.
**	l_user_name			The length of the user name.
**	user_name			The user name of the transaction.
**	DisTranId			Optional pointer to DB_DIS_TRAN_ID.
**
** Outputs:
**	tran_id				External transaction identifier.
**      lx_id                           Transaction identifier.
**      sys_err                         Reason for error return status.
**	Returns:
**	    OK
**	    LG_BADPARAM
**	    LG_EXCEED_LIMIT
**	    LG_DB_INCONSISTENT
**	Exceptions:
**	    none
**
** Side Effects:
**	    none
**
** History:
**	Summer, 1992 (bryanp)
**	    Working on the new portable logging and locking system.
**	29-sep-1992 (nandak)
**	    Use one common transaction ID type.
**	21-oct-1992 (bryanp)
**	    Improve error messages when resources are exhausted.
**	    Also fixed confusion between "err_code" and "sys_err".
**	05-jan-1993 (jnash)
**	    Reduced logging project.  Init lxb_reserved_space.
**	15-mar-1993 (rogerk)
**	    Reduced Logging - Phase IV:
**	    Added LG_NORESERVE option - used by recovery transactions which
**		will write log records without reserving space for them.
**	23-aug-1993 (bryanp)
**	    Initialize lxb_wait_reason.
**	18-oct-1993 (rogerk)
**	    Removed unused lgd_dmu_cnt, lxb_dmu_cnt fields.
**	23-Jan-1996 (jenjo02)
**	    lgd_mutex no longer held on entry to function. Many
**	    other mutexes replace it.
**	26-Jun-1996 (jenjo02)
**	    Count only active txns in lgd_protect_count.
**      22-nov-96 (stial01,dilma04)
**          Row Locking Project:
**          Reserve tran id 0,0
**	13-Dec-1996 (jenjo02)
**	    To avoid deadlocking with LK, don't allow new protected update 
**	    transactions to begin while stalling a DB for checkpoint.
**	    Bug 67888, previously addressed by defining a new lock type
**	    of LK_CKP_TXN and taking that lock each time a transaction
**	    or CKPDB starts. The LK_CKP_TXN lock has proven to be very
**	    contentious, so this solution is offered in its place.
**
**	    When a checkpoint is beginning on a database, delay the 
**	    checkpoint until all protected update transactions (active or
**	    inactive) have completed. At the same time, don't allow
**	    new update transactions to start until the stall condition
**	    has cleared. The fledging LXBs are allocated, left
**	    incomplete, and suspended. When the stall condition clears,
**	    the waiting LXBs are resumed, completed, and placed on
**	    the inactive queue.
**      27-feb-1997 (stial01)
**          Reserve tranid (0,anything) so that unique ids generated by LK
**          do not conflict with LG generated transaction ids
**	13-Nov-1997 (jenjo02)
**	    Modified 13-Dec-1996 change. Newly starting read-write txns
**	    will be stalled if online checkpoint has indicated that they
**	    should via the LDB_STALL_RW flag.
**	24-Aug-1998 (jenjo02)
**	    lxb_tran_id now embedded in (new) LXH structure.
**	15-Mar-1998 (jenjo02)
**	    Stalls for CKPDB no longer happen in here, but are now 
**	    detected in LGwrite(), LGreserve().
**	24-Aug-1999 (jenjo02)
**	    New function, LG_allocate_lxb(), now computes transaction id,
**	    obviating the need to take lfb_mutex to do it inline.
**	    Removed LXB inactive queue and its mutex. Inactive LXBs now
**	    don't appear on any queue until they become active.
**	15-Dec-1999 (jenjo02)
**	    Added support for SHARED XA transactions.
**	22-Oct-2002 (jenjo02)
**	    LG_allocate_lxb() may reject XA connection to SHARED
**	    transaction if it's being aborted.
**	08-Apr-2004 (jenjo02)
**	    Init new lxb_in_mini variable.
**	15-Mar-2006 (jenjo02)
**	    Init lxb_last_lsn as it now shows in logstat.
**	    lxb_wait_reason defines moved to lg.h
**	26-Jun-2006 (jonj)
**	    Init new lxb elements lxb_lock_id, lxb_txn_state.
**	    Add support for xa_start([JOIN]).
**	01-Aug-2006 (jonj)
**	    LG_allocate_lxb() may return an LXB with a status
**	    of LXB_RESUME, indicating the reuse of an orphaned
**	    but intact LXB to use as an XA handle.
**	11-Sep-2006 (jonj)
**	    Init (new) lxb_handle_wts, lxb_handle_ets.
**	15-Jan-2010 (jonj)
**	    SIR 121619 MVCC: init lxb_active_lxbq, set transaction
**	    open in xid array, maintain ldb_lgid_low, high.
*/
STATUS
LGbegin(
i4		    flag,
LG_DBID             external_db_id,
DB_TRAN_ID	    *tran_id,
LG_LXID		    *external_lx_id,
i4		    l_user_name,
char		    *user_name,
DB_DIS_TRAN_ID	    *DisTranId,
CL_ERR_DESC	    *sys_err)
{
    register LGD        *lgd = (LGD *)LGK_base.lgk_lgd_ptr;
    register LXB	*lxb;
    LXB			*prev_lxb;
    LXBQ		*next_lxbq;
    register LPD	*lpd;
    register LDB	*ldb;
    register LPB	*lpb;
    register LFB	*lfb;
    LWQ			*lwq;
    SIZE_TYPE		*lpdb_table;
    SIZE_TYPE		*lxbb_table;
    i4			err_code;
    STATUS		status, return_status;
    LXB			*slxb;
    LG_I4ID_TO_ID	db_id;
    LG_ID		*lx_id = (LG_ID*)external_lx_id;

    LG_WHERE("LGbegin")

    CL_CLEAR_ERR(sys_err);

    /*	Check the db_id. */

    db_id.id_i4id = external_db_id;
    if (db_id.id_lgid.id_id == 0 || (i4)db_id.id_lgid.id_id > lgd->lgd_lpdb_count)
    {
	uleFormat(NULL, E_DMA412_LGBEGIN_BAD_ID, (CL_ERR_DESC *)NULL,
		    ULE_LOG, NULL, NULL, 0, NULL, &err_code, 2,
		    0, db_id.id_lgid.id_id, 0, lgd->lgd_lpdb_count);
	return(LG_BADPARAM);
    }

    lpdb_table = (SIZE_TYPE *)LGK_PTR_FROM_OFFSET(lgd->lgd_lpdb_table);
    lpd = (LPD *)LGK_PTR_FROM_OFFSET(lpdb_table[db_id.id_lgid.id_id]);

    if (lpd->lpd_type != LPD_TYPE ||
	lpd->lpd_id.id_instance != db_id.id_lgid.id_instance)
    {
	uleFormat(NULL, E_DMA413_LGBEGIN_BAD_DB, (CL_ERR_DESC *)NULL,
		    ULE_LOG, NULL, NULL, 0, NULL, &err_code, 3,
		    0, lpd->lpd_type, 0, lpd->lpd_id.id_instance,
		    0, db_id.id_lgid.id_instance);
	return(LG_BADPARAM);
    }
    if (/* lpd->lpd_lpb->lpb_pid != Lg_pid || */
	l_user_name == 0)
    {
	uleFormat(NULL, E_DMA414_LGBEGIN_BAD_LEN, (CL_ERR_DESC *)NULL,
		    ULE_LOG, NULL, NULL, 0, NULL, &err_code, 1, 0, l_user_name);
	return(LG_BADPARAM);
    }

    ldb = (LDB *)LGK_PTR_FROM_OFFSET(lpd->lpd_ldb);
    lpb = (LPB *)LGK_PTR_FROM_OFFSET(lpd->lpd_lpb);
    lfb = (LFB *)LGK_PTR_FROM_OFFSET(lpb->lpb_lfb_offset);

    /*
    ** You can't start any new protected transactions against an
    ** inconsistent database:
    */
    if ((flag & LG_NOPROTECT) == 0 && (ldb->ldb_status & LDB_INVALID))
    {
	return(LG_DB_INCONSISTENT);
    }

    /* Explicit connect to existing context? */
    if ( flag & LG_CONNECT )
    {
	/* "lx_id" must be that of a SHARED LXB */
	if ( lx_id->id_id == 0 || (i4)lx_id->id_id > lgd->lgd_lxbb_count)
	{
	    uleFormat(NULL, E_DMA412_LGBEGIN_BAD_ID, (CL_ERR_DESC *)NULL,
			ULE_LOG, NULL, NULL, 0, NULL, &err_code, 2,
			0, lx_id->id_id, 0, lgd->lgd_lxbb_count);
	    return(LG_BADPARAM);
	}

	lxbb_table = (SIZE_TYPE *)LGK_PTR_FROM_OFFSET(lgd->lgd_lxbb_table);
	slxb = (LXB *)LGK_PTR_FROM_OFFSET(lxbb_table[lx_id->id_id]);

	if ( slxb->lxb_type != LXB_TYPE ||
	     slxb->lxb_id.id_instance != lx_id->id_instance )
	{
	    uleFormat(NULL, E_DMA413_LGBEGIN_BAD_DB, (CL_ERR_DESC *)NULL,
			ULE_LOG, NULL, NULL, 0, NULL, &err_code, 3,
			0, slxb->lxb_type, 
			0, slxb->lxb_id.id_instance,
			0, lx_id->id_instance);
	    return(LG_BADPARAM);
	}
	if ( status = LG_mutex(SEM_EXCL, &slxb->lxb_mutex) )
	    return(status);
	if ( (slxb->lxb_status & LXB_SHARED) == 0 )
	{
	    LG_I4ID_TO_ID xid;
	    xid.id_lgid = slxb->lxb_id;
	    uleFormat(NULL, E_DMA494_LG_INVALID_SHR_TXN, (CL_ERR_DESC *)NULL,
			ULE_LOG, NULL, NULL, 0, NULL, &err_code, 6,
			0, slxb->lxb_type,
			0, LXB_TYPE,
			0, xid.id_i4id,
			0, slxb->lxb_status,
			0, 0,
			0, "LGbegin");
	    (VOID)LG_unmutex(&slxb->lxb_mutex);
	    return(LG_BADPARAM);
	}
	if ( slxb->lxb_status & LXB_ABORT )
	{
	    LG_I4ID_TO_ID xid;
	    /* Reject connect to aborting SHR txn */
	    xid.id_lgid = slxb->lxb_id;
	    uleFormat(NULL, E_DMA495_LG_BAD_SHR_TXN_STATE, (CL_ERR_DESC *)NULL,
		ULE_LOG, NULL, NULL, 0, NULL, &err_code, 4,
		0, xid.id_i4id,
		0, slxb->lxb_status,
		0, "LGbegin",
		0, 0);
	    (VOID)LG_unmutex(&slxb->lxb_mutex);
	    return(LG_BADPARAM);
	}
    }

    /*
    **  Allocate an LXB, returning with lxb_type initialized and
    **  its lxb_mutex held and LXB's transaction id assigned.
    **
    **  Pass DisTranId to LG_allocate_lxb(). If present and
    **  it's an XA transaction, one or two LXBs may be allocated,
    **  an LXB_SHARED and an LXB_SHARED_HANDLE, or just an
    **  LXB_SHARED_HANDLE if a GTRID-equivalent SHARED LXB
    **  has previously been allocated (we're enlisting in a
    **	shared transaction).
    **
    **  That describes an implicit xa_start/xa_end-free
    **  transaction.
    **
    **  When an explicit xa_start is used, we arrive here with
    **  flag & LG_XA_START_XA. By itself, the dist_tran_id
    **  must not otherwise exist in the logging system;
    **  when combined with LG_XA_START_JOIN, the dist_tran_id
    **  must exist, and we connect to that context.
    **  LG_allocate_lxb() figures this all out.
    */
    if ((lxb = (LXB *)LG_allocate_lxb(flag,
		lfb, lpd, DisTranId, &slxb,
		&return_status)) == 0)
    {
	if ( return_status == LG_EXCEED_LIMIT )
	    uleFormat(NULL, E_DMA440_LGBEGIN_NO_LXBS, (CL_ERR_DESC *)NULL,
			ULE_LOG, NULL, NULL, 0, NULL, &err_code, 0);
	return (return_status);
    }

    /* 
    ** Inform the LGbegin() caller of SHARED txn status.
    **
    ** Set return status if a SHARED txn was created or if
    ** we just connected to a SHARED txn:
    */
    if ( flag & LG_CONNECT )
    {
	(VOID)LG_unmutex(&slxb->lxb_mutex);
	slxb = (LXB*)NULL;
	return_status = OK;
    }
    else if ( lxb->lxb_status & LXB_RESUME )
	/* 
	** This is an already allocated XA LXB replete with
	** lock_id that now belongs to us - just pretty
	** much accept it as-is.
	*/
	return_status = LG_CONTINUE;
    else if ( slxb )
	return_status = LG_SHARED_TXN;
    else if ( lxb->lxb_status & LXB_SHARED_HANDLE )
	return_status = LG_CONNECT_TXN;
    else 
	return_status = OK;

    /*	Change various counters. */

    lpb->lpb_stat.begin++;
    lgd->lgd_stat.begin++;
    ldb->ldb_stat.begin++;

    /* Return assigned transaction id and log_id to caller */
    *tran_id = lxb->lxb_lxh.lxh_tran_id;
    *lx_id = lxb->lxb_id;

    /* If this is a LogWriter thread, add it to the queue of such */
    if (flag & LG_LOGWRITER_THREAD)
    {
	lxb->lxb_status |= (LXB_WAIT | LXB_LOGWRITER_THREAD);
	lxb->lxb_wait_reason = LG_WAIT_WRITEIO;
	lwq = &lgd->lgd_lwlxb;
	if (status = LG_mutex(SEM_EXCL, &lwq->lwq_mutex))
	    return(status);
	lxb->lxb_wait.lxbq_next = lwq->lwq_lxbq.lxbq_next;
	lxb->lxb_wait.lxbq_prev = LGK_OFFSET_FROM_PTR(&lwq->lwq_lxbq.lxbq_next);
	next_lxbq = (LXBQ *)LGK_PTR_FROM_OFFSET(lwq->lwq_lxbq.lxbq_next);
	next_lxbq->lxbq_prev =
	    lwq->lwq_lxbq.lxbq_next = LGK_OFFSET_FROM_PTR(&lxb->lxb_wait);
	lwq->lwq_count++;
	lxb->lxb_wait_lwq = LGK_OFFSET_FROM_PTR(lwq);
	(VOID)LG_unmutex(&lwq->lwq_mutex);
    }


    /*	Initialize the LXB(s). */
    if ( lxb->lxb_status & LXB_RESUME )
    {
	/*
	** Reusing previously disassociated LXB.
	** All about it should be ok, but we'll
	** resupply user name, if it matters.
	*/
	MEcopy((PTR)user_name, l_user_name, (PTR)lxb->lxb_user_name);
	lxb->lxb_l_user_name = l_user_name;
	(VOID)LG_unmutex(&lxb->lxb_mutex);
    }
    else do
    {
	lxb->lxb_cp_lga = lfb->lfb_header.lgh_cp;
	lxb->lxb_lpd = LGK_OFFSET_FROM_PTR(lpd);
	lxb->lxb_sequence = 0;
	lxb->lxb_first_lga.la_sequence = 0;
	lxb->lxb_first_lga.la_block    = 0;
	lxb->lxb_first_lga.la_offset   = 0;
	lxb->lxb_last_lga.la_sequence  = 0;
	lxb->lxb_last_lga.la_block     = 0;
	lxb->lxb_last_lga.la_offset    = 0;
	lxb->lxb_first_lsn.lsn_high = 
	    lxb->lxb_first_lsn.lsn_low = 0;
	lxb->lxb_last_lsn.lsn_high = 
	    lxb->lxb_last_lsn.lsn_low = 0;
	lxb->lxb_wait_lsn.lsn_high = 
	    lxb->lxb_wait_lsn.lsn_low  = 0;
	lxb->lxb_reserved_space = 0;
	lxb->lxb_lfb_offset = lpb->lpb_lfb_offset;
	lxb->lxb_stat.split = 0;
	lxb->lxb_stat.write = 0;
	lxb->lxb_stat.force = 0;
	lxb->lxb_stat.wait = 0;
	lxb->lxb_in_mini = 0;
	lxb->lxb_lock_id = 0;
	lxb->lxb_txn_state = 0;

	lxb->lxb_handle_preps = 0;
	lxb->lxb_handle_wts = 0;
	lxb->lxb_handle_ets = 0;

	MEcopy((PTR)user_name, l_user_name, (PTR)lxb->lxb_user_name);
	lxb->lxb_l_user_name = l_user_name;
	lxb->lxb_wait_reason = 0;

	/* Initialize transaction hash pointers */
	lxb->lxb_lxh.lxh_lxbq.lxbq_next = 
	    lxb->lxb_lxh.lxh_lxbq.lxbq_next = 0;

	/* Store the session id of the transaction. */
	if ( flag & LG_XA_START_XA && lxb->lxb_status & LXB_SHARED )
	{
	    /* XA SHARED txns are linked to the RCP, owned by no one */
	    lxb->lxb_cpid.sid = 0;
	    lxb->lxb_cpid.pid = 0;
	    lxb->lxb_status |= LXB_ORPHAN;
	}
	else
	    CSget_cpid(&lxb->lxb_cpid);

	if ( (flag & LG_CONNECT) == 0 )
	{
	    lxb->lxb_status |= (LXB_INACTIVE | LXB_PROTECT);
	    
	    /* HANDLEs to SHARED txns are un-PROTECTed, never become ACTIVE */
	    if ( flag & LG_NOPROTECT || lxb->lxb_status & LXB_SHARED_HANDLE )
		lxb->lxb_status &= ~LXB_PROTECT;
	    if (flag & LG_NOABORT)
		lxb->lxb_status |= LXB_NOABORT;
	    if (flag & LG_NORESERVE)
		lxb->lxb_status |= LXB_NORESERVE;
	    if (flag & LG_READONLY)
		lxb->lxb_status |= LXB_READONLY;
	    else if (flag & LG_JOURNAL)
		lxb->lxb_status |= LXB_JOURNAL;
	}

	if (status = LG_mutex(SEM_EXCL, &ldb->ldb_mutex))
	    return(status);

	ldb->ldb_lxb_count++;
	lpd->lpd_lxb_count++;

	/*	Queue LXB to LPD queue of LXB's. */
	/*  ldb_mutex protects this queue */
	lxb->lxb_owner.lxbq_next = lpd->lpd_lxbq.lxbq_next;
	lxb->lxb_owner.lxbq_prev = LGK_OFFSET_FROM_PTR(&lpd->lpd_lxbq.lxbq_next);
	next_lxbq = (LXBQ *)LGK_PTR_FROM_OFFSET(lpd->lpd_lxbq.lxbq_next);
	next_lxbq->lxbq_prev =
	    lpd->lpd_lxbq.lxbq_next = LGK_OFFSET_FROM_PTR(&lxb->lxb_owner);

	/*
	** If transaction's lxb_id.id_id
	** represents a new low or high transaction boundary,
	** then update the low/high boundary of open transactions.
	*/
	if ( ldb->ldb_lgid_low == 0 ||
	     lxb->lxb_id.id_id < ldb->ldb_lgid_low )
	{
	    ldb->ldb_lgid_low = lxb->lxb_id.id_id;
	}
	if ( ldb->ldb_lgid_high == 0 ||
	     lxb->lxb_id.id_id > ldb->ldb_lgid_high )
	{
	        ldb->ldb_lgid_high = lxb->lxb_id.id_id;
	    }

	/* Not (yet) on the DB's list of active transactions */
	lxb->lxb_active_lxbq.lxbq_next = 
	    lxb->lxb_active_lxbq.lxbq_prev = 0;

	/* Set open tranid "active" in xid array */
	SET_XID_ACTIVE(lxb);

	(VOID)LG_unmutex(&lxb->lxb_mutex);

	/* Loop back to init the SHARED LXB if one was allocated */
	if ( (lxb = slxb) )
	{
	    slxb = (LXB*)NULL;

	    /*
	    ** SHARED XA transactions are owned by the RCP as
	    ** ORPHANed transactions with one of those funny
	    ** extra LPDs used to link the LDB to the RCP.
	    */
	    if ( flag & LG_XA_START_XA && lxb->lxb_status & LXB_SHARED )
	    {
		LPD	*next_lpd;
		LPB	*master_lpb = (LPB*)LGK_PTR_FROM_OFFSET(lgd->lgd_master_lpb);

		if ( (lpd = (LPD*)LG_allocate_cb(LPD_TYPE)) == 0 )
		    return(LG_EXCEED_LIMIT);
		
		lpd->lpd_stat.write = 0;
		lpd->lpd_stat.force = 0;
		lpd->lpd_stat.wait = 0;
		lpd->lpd_stat.begin = 0;
		lpd->lpd_stat.end = 0;
		lpd->lpd_lxb_count = 0;
		lpd->lpd_ldb = LGK_OFFSET_FROM_PTR(ldb);
		lpd->lpd_lpb = lgd->lgd_master_lpb;
		lpd->lpd_lxbq.lxbq_next = lpd->lpd_lxbq.lxbq_prev =
			LGK_OFFSET_FROM_PTR(&lpd->lpd_lxbq.lxbq_next);
		ldb->ldb_lpd_count++;
		(VOID)LG_unmutex(&ldb->ldb_mutex);
		/*
		** Link the new LPD to the recovery process LPB.
		*/
		if (status = LG_mutex(SEM_EXCL, &master_lpb->lpb_mutex))
		    return(status);
		lpd->lpd_lpb = lgd->lgd_master_lpb;
		lpd->lpd_next = master_lpb->lpb_lpd_next;
		lpd->lpd_prev = LGK_OFFSET_FROM_PTR(&master_lpb->lpb_lpd_next);

		next_lpd = (LPD *)LGK_PTR_FROM_OFFSET(master_lpb->lpb_lpd_next);
		next_lpd->lpd_prev = LGK_OFFSET_FROM_PTR(lpd);
		master_lpb->lpb_lpd_next = LGK_OFFSET_FROM_PTR(lpd);
		master_lpb->lpb_lpd_count++;

		(VOID)LG_unmutex(&master_lpb->lpb_mutex);
	    }
	    else
		(VOID)LG_unmutex(&ldb->ldb_mutex);
	}
	else
	    (VOID)LG_unmutex(&ldb->ldb_mutex);
    } while ( lxb );

    return(return_status);
}

/*{
** Name: LGconnect	- Connect to an existing log context.
**
** Description:
**	Connects a thread to an existing log context so that
**	multiple threads may share the same transaction atomicity
**	yet independently wait on log events.
**
**	On the first connection to the log context, a new LXB
**	is allocated to represent the SHARED context and
**	the existing LXB is converted to an associated
**	SHARED HANDLE. The SHARED LXB inherits all of the
**	characteristics and attributes of the existing
**	context.
**
**	Subsequent connects simply create and connect a 
**	HANDLE LXB to the SHARED context.
**
**
** Inputs:
**	connect_lx_id		The context to connect to.
**
** Outputs:
**      handle_lx_id            The new SHARED log identifier.
**      sys_err                 Reason for error return status.
**	Returns:
**	    OK
**	    LG_BADPARAM
**	Exceptions:
**	    none
**
** Side Effects:
**	    none
**
** History:
**	17-Dec-2003 (jenjo02)
**	    Invented for Partitioned Table and Parallel Query
**	    project.
**	31-Mar-2004 (jenjo02)
**	    Oops, forgot to copy lxb_last_lsn from hlxb
**	    to slxb.
**	08-Apr-2004 (jenjo02)
**	    Init new lxb_in_mini variable.
**	15-Jan-2010 (jonj)
**	    SIR 121619 MVCC: Relocate active LXB to
**	    ldb_active_lxbq.
*/
STATUS
LGconnect(
LG_LXID		    connect_lx_id,
LG_LXID		    *handle_lx_id,
CL_ERR_DESC	    *sys_err)
{
    LGD        *lgd = (LGD *)LGK_base.lgk_lgd_ptr;
    LXB		*next_lxb, *prev_lxb;
    LXBQ	*next_lxbq, *prev_lxbq, *lxbq;
    LPD		*lpd;
    LDB		*ldb;
    LPB		*lpb;
    LFB		*lfb;
    SIZE_TYPE	*lxbb_table;
    i4		err_code;
    STATUS	status, return_status;
    LXH		*next_lxh, *prev_lxh;
    LG_I4ID_TO_ID clx_id, lpd_xid;

    LG_ID	*hlx_id = (LG_ID*)handle_lx_id;
    LXB		*clxb, *hlxb, *slxb, *dummy_lxb;

    LG_WHERE("LGconnect")

    CL_CLEAR_ERR(sys_err);

    /*	Check the "connect to" log id */

    clx_id.id_i4id = connect_lx_id;
    if ( clx_id.id_lgid.id_id == 0 || (i4)clx_id.id_lgid.id_id > lgd->lgd_lxbb_count)
    {
	uleFormat(NULL, E_DMA496_LGCONNECT_BAD_ID, (CL_ERR_DESC *)NULL,
		    ULE_LOG, NULL, NULL, 0, NULL, &err_code, 2,
		    0, clx_id.id_lgid.id_id, 0, lgd->lgd_lxbb_count);
	return(LG_BADPARAM);
    }

    lxbb_table = (SIZE_TYPE *)LGK_PTR_FROM_OFFSET(lgd->lgd_lxbb_table);
    clxb = (LXB *)LGK_PTR_FROM_OFFSET(lxbb_table[clx_id.id_lgid.id_id]);

    if ( clxb->lxb_type != LXB_TYPE ||
	 clxb->lxb_id.id_instance != clx_id.id_lgid.id_instance )
    {
	uleFormat(NULL, E_DMA497_LGCONNECT_BAD_ID, (CL_ERR_DESC *)NULL,
		    ULE_LOG, NULL, NULL, 0, NULL, &err_code, 3,
		    0, clxb->lxb_type, 
		    0, clxb->lxb_id.id_instance,
		    0, clx_id.id_lgid.id_instance);
	return(LG_BADPARAM);
    }

    if ( status = LG_mutex(SEM_EXCL, &clxb->lxb_mutex) )
	return(status);

    lfb = (LFB *)LGK_PTR_FROM_OFFSET(clxb->lxb_lfb_offset);
    lpd = (LPD *)LGK_PTR_FROM_OFFSET(clxb->lxb_lpd);
    ldb = (LDB *)LGK_PTR_FROM_OFFSET(lpd->lpd_ldb);
    lpb = (LPB *)LGK_PTR_FROM_OFFSET(lpd->lpd_lpb);

    /* If not converted to a handle yet, do it now */
    if ( (clxb->lxb_status & LXB_SHARED_HANDLE) == 0 )
    {
	if ( (slxb = LG_allocate_lxb(0, 
				lfb, lpd,
				(DB_DIS_TRAN_ID*)NULL,
				&dummy_lxb,
				&return_status)) == (LXB*)NULL )
	{
	    uleFormat(NULL, E_DMA498_LGCONNECT_NO_LXBS, (CL_ERR_DESC*)NULL,
			ULE_LOG, NULL, NULL, 0, NULL, &err_code, 0);
	    LG_unmutex(&clxb->lxb_mutex);
	    return(return_status);
	}

	/* Both the "connect to" and SHARED LXBs are mutexed */

	/* This is the new context we'll connect to */
	*hlx_id = slxb->lxb_id;

	/* Link SHARED LXB to LGD queue of such */
	(VOID)LG_mutex(SEM_EXCL, &lgd->lgd_lxbb_mutex);
	slxb->lxb_slxb.lxbq_next = lgd->lgd_slxb.lxbq_next;
	slxb->lxb_slxb.lxbq_prev = 
	    LGK_OFFSET_FROM_PTR(&lgd->lgd_slxb.lxbq_next);
	lxbq = (LXBQ *)
	    LGK_PTR_FROM_OFFSET(lgd->lgd_slxb.lxbq_next);
	lxbq->lxbq_prev = lgd->lgd_slxb.lxbq_next = 
	    LGK_OFFSET_FROM_PTR(&slxb->lxb_slxb);
	(VOID)LG_unmutex(&lgd->lgd_lxbb_mutex);

	/*
	** If the "connect to" lxb is active, replace it
	** on the active hash and txn and LDB queues with the
	** SHARED LXB.
	*/
	if ( clxb->lxb_status & LXB_ACTIVE )
	{
	    /* Replace cLXB with sLXB on active txn queue */
	    if (LG_mutex(SEM_EXCL, &lgd->lgd_lxb_q_mutex))
		return;
	    next_lxh = (LXH *)LGK_PTR_FROM_OFFSET(clxb->lxb_lxh.lxh_lxbq.lxbq_next);
	    prev_lxh = (LXH *)LGK_PTR_FROM_OFFSET(clxb->lxb_lxh.lxh_lxbq.lxbq_prev);
	    next_lxh->lxh_lxbq.lxbq_prev = LGK_OFFSET_FROM_PTR(&slxb->lxb_lxh.lxh_lxbq);
	    prev_lxh->lxh_lxbq.lxbq_next = LGK_OFFSET_FROM_PTR(&slxb->lxb_lxh.lxh_lxbq);
	    slxb->lxb_lxh.lxh_lxbq.lxbq_next = LGK_OFFSET_FROM_PTR(next_lxh);
	    slxb->lxb_lxh.lxh_lxbq.lxbq_prev = LGK_OFFSET_FROM_PTR(prev_lxh);

	    /* Replace cLXB with sLXB on active queue */
	    next_lxb = (LXB *)LGK_PTR_FROM_OFFSET(clxb->lxb_next);
	    prev_lxb = (LXB *)LGK_PTR_FROM_OFFSET(clxb->lxb_prev);

	    next_lxb->lxb_prev = LGK_OFFSET_FROM_PTR(slxb);
	    prev_lxb->lxb_next = LGK_OFFSET_FROM_PTR(slxb);
	    
	    slxb->lxb_next = LGK_OFFSET_FROM_PTR(next_lxb);
	    slxb->lxb_prev = LGK_OFFSET_FROM_PTR(prev_lxb);

	    (VOID)LG_unmutex(&lgd->lgd_lxb_q_mutex);
	}

	/* The new SHARED LXB inherits all from the "connect to " */

	slxb->lxb_status = clxb->lxb_status | LXB_SHARED;
	slxb->lxb_status &= ~(LXB_WAIT | LXB_WAIT_LBB);

	/* The "connect to" LXB is transformed into a handle */
	clxb->lxb_status |=  (LXB_SHARED_HANDLE | LXB_INACTIVE);
	clxb->lxb_status &= ~(LXB_PROTECT | LXB_ACTIVE);
	clxb->lxb_shared_lxb = LGK_OFFSET_FROM_PTR(slxb);
	
	/* Copy Tran_id */
	slxb->lxb_lxh.lxh_tran_id = clxb->lxb_lxh.lxh_tran_id;
	
	slxb->lxb_cp_lga = clxb->lxb_cp_lga;
	slxb->lxb_lpd = clxb->lxb_lpd;
	slxb->lxb_sequence = clxb->lxb_sequence;
	slxb->lxb_first_lga = clxb->lxb_first_lga;
	slxb->lxb_last_lga = clxb->lxb_last_lga;
	slxb->lxb_first_lsn = clxb->lxb_first_lsn;
	slxb->lxb_last_lsn = clxb->lxb_last_lsn;
	slxb->lxb_wait_lsn = clxb->lxb_wait_lsn;
	slxb->lxb_lfb_offset = clxb->lxb_lfb_offset;
	/* Only the sLXB carries reserved space */
	slxb->lxb_reserved_space = clxb->lxb_reserved_space;
	clxb->lxb_reserved_space = 0;
	/* Move stats to sLXB */
	slxb->lxb_stat.split = clxb->lxb_stat.split;
	slxb->lxb_stat.write = clxb->lxb_stat.write;
	slxb->lxb_stat.force = clxb->lxb_stat.force;
	clxb->lxb_stat.split = 0;
	clxb->lxb_stat.write = 0;
	clxb->lxb_stat.force = 0;
	/* Shared LXBs never wait, handles do */
	slxb->lxb_stat.wait = 0;
	slxb->lxb_handle_preps = 0;
	slxb->lxb_handle_wts = 0;
	slxb->lxb_handle_ets = 0;
	slxb->lxb_wait_reason = 0;
	slxb->lxb_cpid = clxb->lxb_cpid;
	slxb->lxb_shared_lxb = 0;
	slxb->lxb_in_mini = 0;

	MEcopy((PTR)clxb->lxb_user_name, 
			 clxb->lxb_l_user_name,
			 (PTR)slxb->lxb_user_name);
	slxb->lxb_l_user_name = clxb->lxb_l_user_name;

	/* Add "connect to" LXB to list of handles */
	slxb->lxb_handle_count = 1;
	slxb->lxb_handles.lxbq_next =
		slxb->lxb_handles.lxbq_prev =
	    LGK_OFFSET_FROM_PTR(&clxb->lxb_handles.lxbq_next);
	clxb->lxb_handles.lxbq_next =
		clxb->lxb_handles.lxbq_prev =
	    LGK_OFFSET_FROM_PTR(&slxb->lxb_handles.lxbq_next);

	/*	Queue new sLXB to LPD queue of LXB's. */
	/*  ldb_mutex protects this queue */
	(VOID)LG_mutex(SEM_EXCL, &ldb->ldb_mutex);

	ldb->ldb_lxb_count++;
	lpd->lpd_lxb_count++;

	slxb->lxb_owner.lxbq_next = lpd->lpd_lxbq.lxbq_next;
	slxb->lxb_owner.lxbq_prev = LGK_OFFSET_FROM_PTR(&lpd->lpd_lxbq.lxbq_next);
	next_lxbq = (LXBQ *)LGK_PTR_FROM_OFFSET(lpd->lpd_lxbq.lxbq_next);
	next_lxbq->lxbq_prev =
	    lpd->lpd_lxbq.lxbq_next = LGK_OFFSET_FROM_PTR(&slxb->lxb_owner);

	if ( clxb->lxb_active_lxbq.lxbq_next )
	{
	    /* Replace cLXB with sLXB on LDB active queue */
	    next_lxbq = (LXBQ *)LGK_PTR_FROM_OFFSET(clxb->lxb_active_lxbq.lxbq_next);
	    prev_lxbq = (LXBQ *)LGK_PTR_FROM_OFFSET(clxb->lxb_active_lxbq.lxbq_prev);
	    next_lxbq->lxbq_prev = LGK_OFFSET_FROM_PTR(&slxb->lxb_active_lxbq);
	    prev_lxbq->lxbq_next = LGK_OFFSET_FROM_PTR(&slxb->lxb_active_lxbq);
	    slxb->lxb_active_lxbq.lxbq_next = LGK_OFFSET_FROM_PTR(next_lxbq);
	    slxb->lxb_active_lxbq.lxbq_prev = LGK_OFFSET_FROM_PTR(prev_lxbq);

	    clxb->lxb_active_lxbq.lxbq_next =
	        clxb->lxb_active_lxbq.lxbq_prev = 0;
	}
	else
	    slxb->lxb_active_lxbq.lxbq_next =
	        slxb->lxb_active_lxbq.lxbq_prev = 0;

	/* Activate sLXB */
	SET_XID_ACTIVE(slxb);


	(VOID)LG_unmutex(&ldb->ldb_mutex);
	(VOID)LG_unmutex(&slxb->lxb_mutex);
	(VOID)LG_unmutex(&clxb->lxb_mutex);
	/* Conversion is complete */
    }
    else
    {
	if ( clxb->lxb_shared_lxb == 0 )
	{
	    LG_I4ID_TO_ID xid;
	    xid.id_lgid = clxb->lxb_id;
	    uleFormat(NULL, E_DMA493_LG_NO_SHR_TXN, (CL_ERR_DESC *)NULL,
			ULE_LOG, NULL, NULL, 0, NULL, &err_code, 3,
			0, xid.id_i4id,
			0, clxb->lxb_status,
			0, "LGconnect");
	    (VOID)LG_unmutex(&clxb->lxb_mutex);
	    return (LG_BADPARAM);
	}

	slxb = (LXB *)LGK_PTR_FROM_OFFSET(clxb->lxb_shared_lxb);

	/* Pass sLXB id to LGbegin for the connect */
	*hlx_id = slxb->lxb_id;

	/* Release the "connect to" LXB */
	(VOID)LG_unmutex(&clxb->lxb_mutex);
    }

    /* Now call LGbegin to create the new handle LXB */
    lpd_xid.id_lgid = lpd->lpd_id;
    return(LGbegin((slxb->lxb_status & LXB_PROTECT)
			? LG_CONNECT
			: LG_CONNECT | LG_NOPROTECT,
			(LG_DBID) lpd_xid.id_i4id,
			&slxb->lxb_lxh.lxh_tran_id,
			handle_lx_id,
			slxb->lxb_l_user_name,
			slxb->lxb_user_name,
			(DB_DIS_TRAN_ID*)NULL,
			sys_err));
}
