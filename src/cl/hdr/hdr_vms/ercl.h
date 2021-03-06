# ifndef ER_INCLUDED
# define ER_INCLUDED
/*
**    Copyright (c) 1986, 2000 Ingres Corporation
*/

# include    <ernames.h>
 
/**CL_SPEC
** Name:    er.h - Definitions for the ER compatibility library.
**
** Description:
**      This header defines all the structures and constants
**	needed in calls to the ER library routines.
**
** Specification:
**
**  Description:  
**	The ER routines perform all the processing for error
**      message lookup, formatting and logging.  
**      
**  Intended Uses:
**	The ER routines are intended to be used by all INGRES 
**      code, both frontends and backends.  To convert any
**      error code to text (including substitution of parameters)
**      these routines must be called.
**      
**  Assumptions:
**	The following assumptions apply to the ER routines:
**          1.  Two error binary file exists per language per 
**              installation of INGRES.  
**              This file should be in a format
**              One is for fast(cached) message text and 
**              the other is for slow(several I/Os to retrieve)
**              messages.  This file should be in a format 
**              that permits direct access by error code.  
**              (Note it should not be a sequential file
**              that is scanned to find matching text.
**          2.  There will be one set of error text files 
**              per language per installation
**              of INGRES.  This allows the user to customize
**              (for language transalation) the error text.
**          3.  One error text file compiler must exist
**              per installation to build the binary file.
**          4.  Individual facilities within INGRES will
**              have their own error text files.  These 
**              are named as follows:
**                    ERCLF.txt    for CL errors
**                    ERDMF.txt    for DMF errors
**                    ERQEF.txt    for QEF errors
**                    ERQBF.txt    for QBF errors
**                     etc.
**              These files are stored in the same 
**              directory as the ER source directory.
**              The text must be stored in edit type file
**              with a special format. (See CL section on ER.)
**          
**          5.  The logging process is optional.  The Er routines 
**              must be able to recognize the presence or absence
**              of this process and function normally, i.e. all
**              message formatting is done, message simply not
**              logged.
**          
**          6.  To avoid conflict with error numbers, a new error
**              numbering scheme has been adopted.  All major facilites 
**              are assigned an error mask such as E_CL_MASK for the
**              CL facility, E_DM_MASK for DMF, E_QE_MASK for QEF, etc.
**              The first four hex numbers are used to delineate which
**              facility the error code belongs to and is used to 
**              make up the mask.  For example the mask for DMF is
**              0x00030000, the mask for the CL is 0x00010000, and
**              the user visible error codes have values starting
**              from 0 to FFFF.  The facility number is assigned
**              in the header file dbms.h.  Since the CL is broken 
**              down into sub-facitilites they each have a mask.  
**              These are defined in compat.h as follows:
**
**              NOTE!!! current user visiable error codes stay
**                      the same and are in the range 0000-FFFF.
**
**		Major facility error mask for CL.
**
**		     E_CL_MASK          0x00010000
**
**		Error masks for CL sub-functions.
**
**                     E_BT_MASK          0x00000100
**                     E_CH_MASK          0x00000200
**                     E_CK_MASK          0x00000300
**                     E_CP_MASK          0x00000400
**                     E_CV_MASK          0x00000500
**                     E_DI_MASK          0x00000600
**                     E_DS_MASK          0x00000700
**                     E_DY_MASK          0x00000800
**                     E_ER_MASK          0x00000900
**                     E_EX_MASK          0x00000A00
**                     E_GV_MASK          0x00000B00
**                     E_ID_MASK          0x00000C00
**                     E_II_MASK          0x00000D00
**                     E_IN_MASK          0x00000E00
**                     E_LG_MASK          0x00000F00
**                     E_LK_MASK          0x00001000
**                     E_LO_MASK          0x00001100
**                     E_ME_MASK          0x00001200
**                     E_MH_MASK          0x00001300
**                     E_NM_MASK          0x00001400
**                     E_OL_MASK          0x00001500
**                     E_PC_MASK          0x00001600
**                     E_PE_MASK          0x00001700
**                     E_QU_MASK          0x00001800
**                     E_SI_MASK          0x00001900
**                     E_SM_MASK          0x00001A00
**                     E_SR_MASK          0x00001B00
**                     E_ST_MASK          0x00001C00
**                     E_SX_MASK          0x00001D00
**                     E_TE_MASK          0x00001E00
**                     E_TM_MASK          0x00001F00
**                     E_TO_MASK          0x00002000
**                     E_TR_MASK          0x00002100
**                     E_UT_MASK          0x00002200
**
**	    New CL sub facilities.
**
**                     E_NT_MASK          0x00002300
**		       E_SL_MASK          0x00002400
**
**	This means that  ER errors must have a value defined
**      as follows:
**
**      #define	  ER_NO_FILE	(E_CL_MASK + E_ER_MASK + 0x01)
**
**  Definitions/Concepts:
**	ER includes the following external routines:
**	    ERclose    -  Close all error message files.
**	    ERget      -  Look up text of a message associated with
**			  a message number and return to the user.
**			  (This is returned in a static buffer for
**			  use by the frontends only.)
**	    ERinit     -  Initialize the error system parameters.
**			  This is used by the server only, in
**			  most cases.  The frontends will implicitly
**			  initialize the system.
**	    ERlangcode -  Return the language code associated with
**			  a session (for use by FE only).
**	    ERlangstr  -  Return the language string associated with
**			  a language code.
**          ERlookup   -  ***NOTE*** this function is on its way out
**	    ERslookup  -  Look up text of message associated
**                        with error number and substitute format
**                        parameters.
**          ERreceive  -  Reads messages sent by ERsend.  This
**                        is only in error logger process.
**	    ERrelease  -  Release a class of fast messages from
**			  the memory cache.  (For use by FEs only).
**          ERreport   -  Lookup text for CL error message, no
**                        parameter substition allowed, same
**                        as 4.0 and older versions of INGRES.
**          ERsend     -  Sends message to error logger process.
**	    ERx	       -  Dummy macro, used in order to surround
**			  quoted text in source code that is not
**			  to be put in message files for possible
**			  translation.
**
**      ER includes the following support routines.
**          ERcompile  -  Creates the message output file.
**
**	The logging process is optional but the ER routines must be
**      able to function properly if one does not exist.
**
** History: 
**      05-nov-1985 (derek)
**	    Created new for Jupiter.
**	16-sep-1986 (kobayashi)
**	    Changed for Kanji.
**	09-apr-1987 (peter_
**		Moved to Unix and added ER_PTR_ARGUMENT and 
**		ER_DEFAULTLANGUAGE.
**	08-jul-1988 (roger)
**	    Added ER_TEXTONLY flag to ERlookup().
**	29-sep-1988 (thurston)
**	    Added the ER_SEMAPHORE flag to ERinit().
**      03-apr-1989 (jennifer)
**          Added new defintions and error codes for security audit 
**          type messages.
**	19-May-1989 (Mike S)
**	    Move the message file names here, and change them to "_v1"
**	23-may-1989 (jrb)
**	    Renamed MNX files to _v2 for generic errors project.
**	16-jan-1990 (sandyd)
**	    Added ER_MAXLANGUAGES, which is now referenced by generic FE code
** 	    in 6.4.  (This change is the VMS equivalent of the UNIX predcl 
**	    change #110043 submitted by Jon Wong.)
**      6/90 (bobm)
**          change to _v3 to reflect new format.
**      4/91 (Mike S)
**          Add message file name tempates for unbundling.
**      6/91 (stevet)
**          Changed ER_MAXLANGUAGES to 17.
**	26-oct-92 (andre)
**	    replaced references to ERlookup() with references to ERslookup()
**	29-oct-92 (andre)
**	    change to _v4 for SQLSTATEs
**	12-nov-92 (sandyd/dave)
**	    Had to add #ifndef ER_INCLUDED to protect against multiple includes.
**	    This was necessary because of prototyping added to front!hdr!hdr 
**	    ug.h.  The prototype references the ER_MSGID type, so ug.h must
**	    now include er.h.
**	7-jun-93 (ed)
**	    created glhdr version contain prototypes, renamed existing file
**	    to xxcl.h and removed any func_externs
**	16-jun-95 (albany)
**	    Removed extraneous ^M.
**	11-jun-1998 (kinte01)
**	    Cross integrate change 435890 from oping12
**          19-May-1998 (horda03) X-Integrated change 427896.
**            Add ER_NAMEONLY for Evidence Set production.
**      18-jun-1999 (canor01)
**          Add ER_NOPARAM for no parameter substitution.
**	01-dec-2000	(kinte01)
**	    Bug 103393 - removed nat, longnat, u_nat, & u_longnat
**	    from VMS CL as the use is no longer allowed
**	21-sep-2001 (somsa01)
**	    Added ER_MU_SEMFUNCS.

**/
 
/*
**  Macro Definition of ERx.
*/
 
#define	    ERx(c)	c
 
 
/*
**  Defined constants.
*/
 
/*  Flag values to ERslookup. */
 
#define	    ER_TIMESTAMP    0x01
#define	    ER_TEXTONLY	    0x02	/* Like ERget, but substitute params */
#define     ER_NAMEONLY     0x04
#define     ER_NOPARAM      0x08	/* No parameter substitution */

/*  Flag values to ERinit. */
 
#define	    ER_ASYNC	    0x01	/* On for async IO (for server) */
#define	    ER_MSGTEST	    0x02	/* On for running test msg system */
#define	    ER_SEMAPHORE    0x04	/* On for semaphoring around msg IO's */
#define	    ER_MU_SEMFUNCS  0x08	/* Use default MU semaphore routines*/

/*  Maximum length of a returned string and maximum number of messages. */
 
#define	    ER_MAX_LEN	    (i4)1000
#define     ER_MSG_MAX      (i4)8 
#define     ER_OPR_MAX     (i4)248
/*  Maximum length of a returned language string by ERlangstr() */
 
#define	    ER_MAX_LANGSTR	12
 
/*  Maximum number of languages currently supported by ER */

#define     ER_MAXLANGUAGES	17

/*	Default language to use   */
 
# define	ER_DEFAULTLANGUAGE	ERx("english")
 
/* delimitor for end of individual error message */
 
#define	    ER_DELIM	    '~'
#define	    ER_NUM_DELIM    ':'
 
 
/*  Flag values to ERsend and ERreceive. */
 
#define	    ER_ERROR_MSG    0x00
#define	    ER_AUDIT_MSG    0x01	
#define	    ER_OPER_MSG     0x02	

/*  Error return values from ER routines.   */
 
#define		ER_OK	    0
#define		ER_NO_FILE  (E_CL_MASK + E_ER_MASK + 0x01)
#define		ER_NOT_FOUND  (E_CL_MASK + E_ER_MASK + 0x02)
#define		ER_BADPARAM (E_CL_MASK + E_ER_MASK + 0x03)
#define		ER_BADREAD  (E_CL_MASK + E_ER_MASK + 0x04)
#define		ER_TOOSMALL (E_CL_MASK + E_ER_MASK + 0x05)
#define		ER_TRUNCATED	(E_CL_MASK + E_ER_MASK + 0x06)
#define		ER_BADOPEN  (E_CL_MASK + E_ER_MASK + 0x07)
#define		ER_BADRCV   (E_CL_MASK + E_ER_MASK + 0x08)
#define		ER_BADSEND  (E_CL_MASK + E_ER_MASK + 0x09)
#define		ER_DIRERR   (E_CL_MASK + E_ER_MASK + 0x0A)
#define		ER_NOALLOC  (E_CL_MASK + E_ER_MASK + 0x0B)
#define		ER_MSGOVER  (E_CL_MASK + E_ER_MASK + 0x0C)
#define		ER_NOFREE   (E_CL_MASK + E_ER_MASK + 0x0D)
#define		ER_BADCLASS (E_CL_MASK + E_ER_MASK + 0x0E)
#define		ER_BADLANGUAGE	(E_CL_MASK + E_ER_MASK + 0x0F)
#define		ER_NO_AUDIT	(E_CL_MASK + E_ER_MASK + 0x10)
#define		ER_FILE_FORMAT (E_CL_MASK + E_ER_MASK + 0x11)
#define		ER_FILE_VERSION (E_CL_MASK + E_ER_MASK + 0x12)
#define		ER_FILE_CORRUPTION (E_CL_MASK + E_ER_MASK + 0x13)


/* 
** Message file names 
**
** Before changing these, please read the discussion in 
** 	front!frontcl!fmsginst!iifstmsg.c
**
**    For unbundled programs (those with a non-NULL part name) the file
**    name templates are used.  "%s" is replaced with the first three
**    characters of the partname (lowercased if necessary).
*/
#define		ER_FASTFILE	ERx("fast_v4.mnx")
#define		ER_SLOWFILE	ERx("slow_v4.mnx")
 
#define         ER_FASTTMPLT    ERx("f%sv4.mnx")
#define         ER_SLOWTMPLT    ERx("s%sv4.mnx")

/*}
** Name: ER_ARGUMENT	- Format argument to ERslookup.
**
** Description:
**	This structure defines the type of parameter passed to ERslookup
**	for value substitution.
**
** History:
**	05-nov-1985 (derek)
**	    Created new for 5.0.
*/
typedef struct _ER_ARGUMENT
{
    i4		er_size;		/* Size of value in bytes. */

#	define	ER_PTR_ARGUMENT	-1
/*
**	    Used by the frontend code to pass variable argument lists
**	    lists instead of control blocks (see ERslookup). If er_size
**	    is ER_PTR_ARGUMENT, the corresponding value is a pointer.
**	    This applies to all of the formats. By definition, if
**	    the format is 'd' the pointer is to a i4, if it
**	    is 'f' it is a pointer to a double, if it is a 'c' it is 
**	    a pointer to a NULL terminated string. if it is an 'x'
**	    it is a pointer to an u_i4.
*/
    PTR		er_value;		/* Pointer to value. */
}   ER_ARGUMENT;
 
typedef	    u_i4    ER_MSGID;	/* data type of message id */
typedef	    i4	    ER_CLASS;	/* data type of class number */

# endif /* ER_INCLUDED */
