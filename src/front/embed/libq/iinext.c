/*
** Copyright (c) 2004 Ingres Corporation
*/

# include	<compat.h>
# include	<cv.h>		/* 6-x_PC_80x86 */
# include	<gl.h>
# include	<sl.h>
# include	<iicommon.h>
# include	<generr.h>
# include	<gca.h>
# include	<iicgca.h>
# include	<iirowdsc.h>
# include	<iisqlca.h>
# include	<iilibq.h>
# include	<erlq.h>

/**
+*  Name: iinextget.c - Defines routine to see if there are any more rows in
**		        a RETRIEVE.
**
**  Defines:
**	IInextget	- Routine to see if any more data is following to
**			  allow assignments into host variable.
**	IIerrtest	- Was there an error during RETRIEVE.
**
**  Notes:
**	For a full example see file iiretinit.c.  In general IInextget will
**	be generated in the following context:
**
**  IIrtB1:
**	while (IInextget() != 0) {
**	    IIretdom(1,DB_INT_TYPE,sizeof(i4),&i);
**	    IIretdom(1,DB_FLT_TYPE,sizeof(f4),&f);
**	    IIretdom(1,DB_CHR_TYPE,0,c);
**	    if (IIerrtest() != 0) goto IIrtB1;
**	    {
**		Code;
**	    }
**	} 
**	IIflush((char *)0,0);
**
**  Version Notes:
**
**   INGRES 3.0 tuple description:
**
**	For tuples 1 thru N the format is:
**
**	{<typ-len, data1>, <typ-len, data2>, ...., EOTUP-len}
**	   ^					^         ^
**	2 * i2				    # domains	  unused
**
**	after the very last tuple 4 bytes return with <EXIT-unused length>.
**	If there are no tuples then only the above 4 bytes return.
**
**   INGRES 4.0 tuple description:
**
**	The zero'th tuple coming back from a retrieve is the descriptor of the
** 	domains. Its format is:
**
**	{dom_count, <typ-len>, <typ-len>, .... , marker }
**	  ^		^		  ^
**	  i4	    2 * i2	  dom_count pairs
**
**	where marker is either EOTUP or EXIT. If no tuples follow then marker
**	is EXIT otherwise it is EOTUP. This marker is only sizeof(i2).
**
**	Tuples 1 thru N-1 have the format:
**
**	{<data1>, <data2>, ...., EOTUP}
**			   ^	  ^
**		    # domains	  i2
**
**	Tuple N has the format:
**
**	{<data1>, <data2>, ...., EXIT}
**
** 	The tuple description is stored in the global IItp_dsc (see IIretinit).
**
**	Comment: INGRES 3.0 wins out in the case where te number of tuples, N,
**		 and N <= 1.  In this case 4.0 sends an extra descriptor.  In
**	all other cases (significant ones where N > 1) 4.0 wins.
**
**   INGRES 6.0 tuple description:
**	
**	Tuples now have NO delimiter.  GCA figures out if there is more
**	data, and, if so, it better be the case that we are aligned exactly
**	at the right data boundary.  See IIrdDescribe() for details on how they
**	are put into ii_rd_desc.
-*
**  History:
**	09-oct-1986	- Modified to use 6.0. (ncg)
**	26-aug-1987 	- Modified to use GCA. (ncg)
**	12-dec-1988	- Generic error processing. (ncg)
**	08-dec-1989	- Updated for Phoenix/Alerters. (bjb)
**	15-oct-1990 (barbara)
**	    Bug 9398. Warnings used to cause IInextget to terminate
**	    a select loop.  This made no sense when the warning was something
**	    like "non-printing chars converted to blanks" -- and what followed
**	    was no conversion at all into the user variable.  Warnings are
**	    now detected by IIdberr which sets the II_Q_WARN flag.  This flag
**	    is cleared in IIqry_end.  Numeric overflow on conversion of
**	    columns to user variables also sets this flag.
**	03-dec-1990 (barbara)
**	    Moved state (for INQUIRE_INGRES) from GLB_CB to LBQ_CB (bug 9160)
**	01-apr-1991 (kathryn)
**	    Integrate change from 3/90 which did not make it into 6.4.
**          26-mar-1990 - Verify that session id matches the session id of the
**          session that started SELECT loop.  Bug #9159. (bjb)
**	18-Dec-97 (gordy)
**	    Added support for multi-threaded applications.
**	21-jan-1999 (hanch04)
**	    replace nat and longnat with i4
**	31-aug-2000 (hanch04)
**	    cross change to main
**	    replace nat and longnat with i4
**/


/*{
+*  Name: IInextget - Routine to check if there is any more data coming
**		      from the DBMS.
**
**  Description:
**	This routine is called each time through the loop that is generated by
**	a RETRIEVE statement.  If there is more data (IIcgc_more_data) then 
**	return TRUE which will allow the caller to retrieve DB values into
**	host variables. Otherwise return FALSE, and the caller's loop will
**	be broken.
**
**  Inputs:
**	None
**
**  Outputs:
**	Returns:
**	    i4  - 1 - There is more data; assign it into host variables.
**		  0 - No more data, break the RETRIEVE loop.
**	Errors:
**	    Those returning from the DBMS.
-*
**  Side Effects:
**	
**  History:
**	08-aug-1983 	- Added processing of error blocks (lichtman)
**	23-aug-1985	- Modified for 4.0 support (ncg)
**	03-sep-1985	- Modified to pick up response message after
**			  the EXIT signal. (lin)
**	09-oct-1986	- Modified to use 6.0. (ncg)
**	12-dec-1988	- Generic error processing. (ncg)
**	08-dec-1989	- Restore dml in case of nested queries. (bjb)
**	15-oct-1990 (barbara)
**	    Fixed bug 9398 (see header history).
**      01-apr-1991 (kathryn)
**          Integrate change from 3/90 which did not make it into 6.4.
**          26-mar-1990 - Verify that session id matches the session id of the
**          session that started SELECT loop.  Bug #9159. (bjb)
**	04-apr-1991 (kathryn) Bug 36849
**	    Don`t verify session id match on singleton select.
*/

i4
IInextget()
{
    II_THR_CB	*thr_cb = IILQthThread();
    II_LBQ_CB	*IIlbqcb = thr_cb->ii_th_session;
    char	ebuf1[20], ebuf2[20];
    II_RET_DESC	*rd = &IIlbqcb->ii_lq_retdesc;

    /* Reset DML/SQLCA in case query was nested inside select loop */
    IIlbqcb->ii_lq_dml = IIlbqcb->ii_lq_svdml;
    /*
    ** If an error occurred during the RETRIEVE loop that is related to the
    ** RETRIEVE (such as a nested query, a bad variable type, but not an FRS
    ** error) then make sure the RETRIEVE stops.  Otherwise clear the error
    ** flag.  If we drop out in an error, IIflush will clean up.
    **
    ** Warnings do not stop the loop.  Overflow errors are classed as a
    ** warning in IIgetdat.  Other warnings are those from the dbms with
    ** generic code set to warning.
    **
    ** Future TM Update:  If using TM then do not stop on error.
    */
    if (   II_DBERR_MACRO(IIlbqcb)
	&& (IIlbqcb->ii_lq_curqry & II_Q_WARN) == 0
       )
    {
	return 0;
    }
    /*
    ** If session id saved by IIretinit doesn't match id of current session,
    ** exit loop.  IIflush will pop session id.
    */
    if (  !(IIlbqcb->ii_lq_flags & II_L_SINGLE) 
        && (IILQscSessionCompare( thr_cb ) == FALSE)
       )
	return 0;


    /* Drop out if someone tried a nested query */
    if (rd->ii_rd_flags & II_R_NESTERR)
    {
	return 0;
    }

    /*
    ** If the descriptor is empty then we failed in IIretinit or
    ** IIrdDescribe (both with an error).  In any case we should stop here.
    */
    if (rd->ii_rd_desc.rd_numcols == 0 || rd->ii_rd_flags == II_R_INITERR)
	return 0;

    /*
    ** Check to see if all the columns were retrieved.  Before even the first
    ** column IIretinit sets "colnum" to be NOCOLS inorder not to make this
    ** test.
    */
    if (rd->ii_rd_colnum != II_R_NOCOLS &&
        rd->ii_rd_colnum != rd->ii_rd_desc.rd_numcols)
    {
	CVna(rd->ii_rd_desc.rd_numcols, ebuf1);		/* i4  */
	CVna(rd->ii_rd_colnum, ebuf2);			/* i4  */
	IIlocerr(GE_CARDINALITY, E_LQ003C_RETCOLS, II_ERR, 2, ebuf1, ebuf2);
	IIlbqcb->ii_lq_curqry |= II_Q_QERR;
	if (IISQ_SQLCA_MACRO(IIlbqcb))
	    IIsqWarn( thr_cb, 3 );	/* Status information for SQLCA */
	return 0;
    }

    /* Reset index into descriptor */
    rd->ii_rd_colnum = 0;


    /* 
    ** If IIcgc_more_data returns 0, then we're at the end;
    **			   IICGC_GET_ERR then we've processed a DB error;
    **			   > 0 then those bytes remain to be read.
    */
    if (IIcgc_more_data(IIlbqcb->ii_lq_gca, GCA_TUPLES) > 0)
    {
	/* Up the rowcount */
	IIlbqcb->ii_lq_rowcount++;
	return 1;
    }
    return 0;
}
	    

/*{
+*  Name: IIerrtest - Test if there was an error.
**
**  Description:
**	Returns if there was an error.  Because this call is not isolated
**	to just within RETRIEVE loops, we can not just return 0 if the
**	ii_rd_flags field is set to mark a RETRIEVE error. (That would 
**	probably be cleaner).
**
**	Even overflow errors return as an error, and do not allow
**	host code execution (this may be relaxed later).
**
**  Inputs:
**	None
**
**  Outputs:
**	Returns:
**	    i4 - Error number (0 if no error).
**	Errors:
**	    None
-*
**  Side Effects:
**	
**  History:
**	09-oct-1986	- Commented. (ncg)
**	15-oct-1990 (barbara)
**	    Returns 0 if the error is actually a warning.  Test II_Q_WARN
**	    flag set by IIdberr/IIgetdata.
**	7-Jan-2008 (kibro01) b119483
**	    If we have an error and we're going to jump out of a DBPROC
**	    loop (which we do using this call), mark that we're out of it.
**      11-Aug-2009 (horda03) B122442
**          Removed kibro01's fix for Bug 119483. Clearing the DBPROC flag
**          caused VMS applications to hang. The real problem was the
**          generated code, the END label was incorrectly placed outside
**          the DBproc loop.
*/

i4
IIerrtest()
{
    II_THR_CB	*thr_cb = IILQthThread();
    II_LBQ_CB	*IIlbqcb = thr_cb->ii_th_session;

    return IIlbqcb->ii_lq_curqry & II_Q_WARN ?
	0 : IIlbqcb->ii_lq_error.ii_er_num;
}
