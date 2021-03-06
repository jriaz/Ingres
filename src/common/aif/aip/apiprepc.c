/*
** Copyright (c) 2004 Ingres Corporation
*/

# include <compat.h>
# include <gl.h>
# include <iicommon.h>

# include <iiapi.h>
# include <api.h>
# include <apithndl.h>
# include <apierhnd.h>
# include <apidspth.h>
# include <apimisc.h>
# include <apitrace.h>

/*
** Name: apiprepc.c
**
** Description:
**	This file contains the IIapi_prepareCommit() function interface.
**
**      IIapi_prepareCommit  API prepare commit function
**
** History:
**      01-oct-93 (ctham)
**          creation
**      17-may-94 (ctham)
**          change all prefix from CLI to IIAPI.
**      17-may-94 (ctham)
**          use II_EXPORT for functions which will cross the interface
**          between API and the application.
**	 31-may-94 (ctham)
**	     clean up error handle interface.
**	23-Jan-95 (gordy)
**	    Clean up error handling.
**	10-Feb-95 (gordy)
**	    Make callback for all errors.
**	 8-Mar-95 (gordy)
**	    Use IIapi_initialized() to test for init.
**	19-May-95 (gordy)
**	    Fixed include statements.
**	28-Jul-95 (gordy)
**	    Fixed tracing of pointers.
**	17-Jan-96 (gordy)
**	    Added environment handles.
**	 2-Oct-96 (gordy)
**	    Replaced original SQL state machines which a generic
**	    interface to facilitate additional connection types.
**      31-May-05 (gordy)
**          Don't permit new requests on handles marked for deletion.
*/




/*
** Name: IIapi_prepareCommit
**
** Description:
**	This function provide an interface for the frontend application
**      to prepare a transaction for 2 phase commit.  Please refer to
**      the API user specification for function description.
**
**      prepCmtParm  input and output parameters for IIapi_prepareCommit()
**
** Return value:
**      none.
**
** Re-entrancy:
**	This function does not update shared memory.
**
** History:
**      01-oct-93 (ctham)
**          creation
**      17-may-94 (ctham)
**          change all prefix from CLI to IIAPI.
**      17-may-94 (ctham)
**          use II_EXPORT for functions which will cross the interface
**          between API and the application.
**	 31-may-94 (ctham)
**	     clean up error handle interface.
**	23-Jan-95 (gordy)
**	    Clean up error handling.
**	10-Feb-95 (gordy)
**	    Make callback for all errors.
**	 8-Mar-95 (gordy)
**	    Use IIapi_initialized() to test for init.
**	28-Jul-95 (gordy)
**	    Fixed tracing of pointers.
**      31-May-05 (gordy)
**          Don't permit new requests on handles marked for deletion.
*/

II_EXTERN II_VOID II_FAR II_EXPORT
IIapi_prepareCommit( IIAPI_PREPCMTPARM II_FAR *prepCmtParm )
{
    IIAPI_TRANHNDL	*tranHndl;
    
    IIAPI_TRACE( IIAPI_TR_TRACE )
	( "IIapi_prepareCommit: preparing a transaction to commit\n" );
    
    /*
    ** Validate Input parameters
    */
    if ( ! prepCmtParm )
    {
	IIAPI_TRACE( IIAPI_TR_ERROR )
	    ( "IIapi_prepareCommit: null commit parameters\n" );
	return;
    }
    
    prepCmtParm->pr_genParm.gp_completed = FALSE;
    prepCmtParm->pr_genParm.gp_status = IIAPI_ST_SUCCESS;
    prepCmtParm->pr_genParm.gp_errorHandle = NULL;
    tranHndl = (IIAPI_TRANHNDL *)prepCmtParm->pr_tranHandle;
    
    /*
    ** Make sure API is initialized
    */
    if ( ! IIapi_initialized() )
    {
	IIAPI_TRACE( IIAPI_TR_ERROR )
	    ( "IIapi_prepareCommit: API is not initialized\n" );
	IIapi_appCallback( &prepCmtParm->pr_genParm, NULL, 
			   IIAPI_ST_NOT_INITIALIZED );
	return;
    }
    
    if ( ! IIapi_isTranHndl( tranHndl )  ||  IIAPI_STALE_HANDLE( tranHndl ) )
    {
	IIAPI_TRACE( IIAPI_TR_ERROR )
	    ( "IIapi_prepareCommit: invalid transaction handle\n" );
	IIapi_appCallback( &prepCmtParm->pr_genParm, NULL, 
			   IIAPI_ST_INVALID_HANDLE );
	return;
    }
    
    IIAPI_TRACE( IIAPI_TR_INFO )
	( "IIapi_prepareCommit: tranHndl = %p\n", tranHndl );
    
    IIapi_clearAllErrors( (IIAPI_HNDL *)tranHndl );
    
    /*
    ** Transaction name handle produced by IIapi_registerXID() is
    ** required for 2PC.
    */
    if ( ! IIapi_isTranName( tranHndl->th_tranName ) )
    {
	IIAPI_TRACE( IIAPI_TR_ERROR )
	    ("IIapi_prepareCommit: can't have 2PC without transaction name\n");
	IIapi_appCallback( &prepCmtParm->pr_genParm, NULL, 
			   IIAPI_ST_INVALID_HANDLE );
	return;
    }
    
    IIapi_uiDispatch( IIAPI_EV_PRECOMMIT_FUNC, 
		      (IIAPI_HNDL *) tranHndl, (II_PTR)prepCmtParm );

    return;
}
