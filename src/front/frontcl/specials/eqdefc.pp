/%
** Copyright (c) 2007 Ingres Corporation
%/
/*
**  eqdefc.pp
**
**	This file contains function prototypes for all C function calls which 
**	might be generated by the ESQL/C preprocessor.  ESQL/C generated 
**	output source files will always #include eqdefc.h and eqpname.h
**	header files if they are preprocessed with the '-prototypes' flag.
**
**	This files uses the same format as the eqdef.pp file.
**
**	Note: to get the spacing correct, always put a tab before the
**	function name - use two tabs for continuing arguments on the 
**	next line.
**
**  History:
**	18-apr-94 (teresal)
**		Created. used eqdef.pp as a template.
**	04-may-94 (teresal)
**		Fixed prototype for IILQldh_LoDataHandler - the data
**		handler routine passes a generic pointer argument and
**		returns an int not a long. Also fixed IIseterr so
**		the handler returns an int (as documented) not a long.
**		Bugs 63205 & 63202.
**	8-may-1996 (angusm)
**		Fix for prototype of IILQlgd_LoGetData() 
**		(Bug 75185)
**	17-may-96 (thoda04)
**		Rearranged the #ifdef's to make Windows 3.1 and other ports easier.
**		Added the changes needed for Windows 3.1.
**	4-apr-1997(angusm)
**		Fix for prototype of IIG4fdFillDscr() (bug 77838).
**	2-may-97 (inkdo01)
**		Added entry for IILQprvProcGTTParm (for global temptab proc
**		parm).
**	24-mar-1999 (somsa01)
**	    Changed default type of II_LQTYPE_PTR to be a char pointer.
**      30-Mar-2000 (fanra01)
**          Bug 101103
**          Update prototype for IILQssSetSqlio to use II_LQTYPE_GPTR.
**          Previous change to II_LQTYPE_PTR to causes compile errors in
**          C++ where the session id is an int* and function is prototyped
**          as char*.
**	21-jan-1999 (hanch04)
**	    replace nat and longnat with i4
**	31-aug-2000 (hanch04)
**	    cross change to main
**	    replace nat and longnat with i4
**	02-Jul-2001 (hanje04)
**	    Update prototype of IILQprsProcStatus(void) to (int) following
**	    change to funtion by inkdo01 (change 450571)
**      13-Jul-2001 (ashco01)
**          Bug 104671
**          Updated prototype IILQisInqSqlio() defining type II_LQTYPE_GPTR 
**          for parameter #5. Previous change of II_LQTYPE_PTR from (void *) 
**          to (char *) caused compilation errors in less tolerant compilers
**          when building esqlc programs.
**      06-May-2002 (shust01)
**          Bug 107715
**          Updated prototype IILQprvProcValio() defining type II_LQTYPE_GPTR 
**          for parameter #7. Previous change of II_LQTYPE_PTR from (void *) 
**          to (char *) caused compilation errors in less tolerant compilers
**          when building esqlc programs, and passing integers to a procedure.
**	04-mar-2003 (somsa01)
**	    Reversed change from void * to char * of type of II_LQTYPE_PTR
**	    due to many client problems with various functions.
**      27-oct-2004 (huazh01)
**          Added IIhelp(). 
**          b64679, INGCBT545.
**       1-Nov-2005 (hanal04) Bug 115476 INGEMB148
**          Corrected prototype for IIhelp().
**	6-Jun-2006 (kschendel)
**	    Added IIsqDescInput.
**	29-Jan-2007 (kiria01) b117277
**	    Added IIset_random.
**	08-Mar-2007 (kiria01) b117277
**	    Added missing ;
**	09-apr-2007 (drivi01)
**	    BUG 117501
**	    Adding references to function IIcsRetScroll in order to fix
**	    build on windows.
**      24-Apr-2007 (kiria01) b118184
**          Renamed IIhelp to IIhelptoggle to avoid naming conflict
**          runsys routines.
**	11-Oct-2007 (kiria01) b118421
**	    Include guard against repeated inclusion and allow to be included
**	    from C++
*/

%#ifndef EQDEFC_H_INCLUDED
%#define EQDEFC_H_INCLUDED
%#ifdef __cplusplus
extern "C" {
%#endif

#ifdef SOME_UNUSUAL_SYSTEM
/*
** This SOME_UNUSUAL_SYSTEM section is just a dummy example of the intended
** porting mechanism.  It is expected that it will be replaced by a real
** config string when ported to a platform that requires special type mappings.
**
** To preserve sanity during future maintenance, it is hoped that ifdefs will
** be used only on the type mappings, and that each function declaration will
** appear once and only once in this file.
*/

# define II_LQTYPE_BOOL         unusual_bool
# define II_LQTYPE_I2           unusual_i2
# define II_LQTYPE_I4           unusual_i4
# define II_LQTYPE_LONGNAT      unusual_i4
# define II_LQTYPE_NAT          unusual_nat
# define II_LQTYPE_PTR          unusual_ptr
# define II_LQTYPE_VOID         unusual_void
# define II_LQTYPE_GPTR		unusual_generic_pointer

/*
** Additional Porting Note: If your CL has prototypes, you need to set
**	CP prototyping on by adding the following define to this file:
*/

# define CL_HAS_PROTOS

#endif /* SOME_UNUSUAL_SYSTEM */

# ifdef WIN16
# define II_LQTYPE_BOOL         int
# define II_LQTYPE_I4           long
# define II_LQTYPE_NAT          long
# endif /* WIN16 */


/*   	 Ingres type		Mapped to */

# ifndef II_LQTYPE_BOOL
# define II_LQTYPE_BOOL         char
# endif

# ifndef II_LQTYPE_I2
# define II_LQTYPE_I2           short
# endif

# ifndef II_LQTYPE_I4
# define II_LQTYPE_I4           int
# endif

# ifndef II_LQTYPE_LONGNAT
# define II_LQTYPE_LONGNAT      long
# endif

# ifndef II_LQTYPE_NAT
# define II_LQTYPE_NAT          int
# endif

# ifndef II_LQTYPE_PTR
# define II_LQTYPE_PTR          void *
# endif

# ifndef II_LQTYPE_VOID
# define II_LQTYPE_VOID         void
# endif

/*
** Special symbol indicates places where we have replaced 'char *' with a
** generic pointer like in the io calls or replaced a pointer to a 
** structure like the in IIsqInit().
*/

# ifndef II_LQTYPE_GPTR
# define II_LQTYPE_GPTR		void *
# endif


/*********************************************/
/* LIBQ calls - listed in alphabetical order */
/*********************************************/

void	IIbreak(void);
void	IIcsClose(char *cursor_name,II_LQTYPE_NAT num1,II_LQTYPE_NAT num2);
void	IIcsDaGet(II_LQTYPE_NAT lang,II_LQTYPE_GPTR sqd);
void	IIcsDelete(char *table_name,char *cursor_name,II_LQTYPE_NAT num1,
		II_LQTYPE_NAT num2);
void	IIcsERetrieve(void);
void	IIcsERplace(char *cursor_name,II_LQTYPE_NAT num1,II_LQTYPE_NAT num2);
void	IIcsGetio(II_LQTYPE_I2 *indaddr,II_LQTYPE_NAT isvar,II_LQTYPE_NAT type,
		 II_LQTYPE_NAT length,II_LQTYPE_GPTR address);
void	IIcsOpen(char *cursor_name,II_LQTYPE_NAT num1,II_LQTYPE_NAT num2);
void	IIcsParGet(char *target_list,char **argv,
#ifdef  CL_HAS_PROTOS
		II_LQTYPE_NAT (*transfunc)(II_LQTYPE_PTR,II_LQTYPE_PTR,
		II_LQTYPE_PTR, II_LQTYPE_PTR));
#else
		II_LQTYPE_NAT (*transfunc)());
#endif
void	IIcsQuery(char *cursor_name,II_LQTYPE_NAT num1,II_LQTYPE_NAT num2);
void	IIcsRdO(II_LQTYPE_NAT what,char *rdo);
void	IIcsReplace(char *cursor_name,II_LQTYPE_NAT num1,II_LQTYPE_NAT num2);
II_LQTYPE_NAT	IIcsRetrieve(char *cursor_name,II_LQTYPE_NAT num1,
		II_LQTYPE_NAT num2);
II_LQTYPE_NAT	IIcsRetScroll(char *cursor_name, II_LQTYPE_NAT num1,
		II_LQTYPE_NAT num2, II_LQTYPE_NAT fetcho, II_LQTYPE_NAT fetchn);
void	IIeqiqio(II_LQTYPE_I2 *indaddr,II_LQTYPE_NAT isvar,II_LQTYPE_NAT type,
		 II_LQTYPE_NAT len,II_LQTYPE_PTR addr,char *name);
void	IIeqstio(char *name, II_LQTYPE_I2 *indaddr,II_LQTYPE_NAT isvar,
		 II_LQTYPE_NAT type,II_LQTYPE_NAT len,II_LQTYPE_PTR data);
II_LQTYPE_I4	IIerrtest(void);
void	IIexDefine(II_LQTYPE_NAT what,char *name,II_LQTYPE_NAT num1,
		II_LQTYPE_NAT num2);
II_LQTYPE_NAT	IIexExec(II_LQTYPE_NAT what,char *name,II_LQTYPE_NAT num1,
		II_LQTYPE_NAT num2);
void	IIexit(void);
void	IIflush(char *file_nm,II_LQTYPE_NAT line_no);
II_LQTYPE_NAT	IIgetdomio(II_LQTYPE_I2 *indaddr,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT hosttype,II_LQTYPE_LONGNAT hostlen,
		II_LQTYPE_GPTR addr);
II_LQTYPE_VOID	IIingopen(II_LQTYPE_NAT lang,char *p1,char *p2,char *p3,
		char *p4,char *p5,char *p6,char *p7,char *p8,char *p9,
		char *p10,char *p11,char *p12,char *p13,char *p14,char *p15);
II_LQTYPE_VOID	IILQcnConName(char *con_name);
II_LQTYPE_PTR	IILQdbl(double dblval);
II_LQTYPE_VOID	IILQesEvStat(II_LQTYPE_NAT escall,II_LQTYPE_NAT eswait);
II_LQTYPE_PTR	IILQint(II_LQTYPE_NAT intval);
void	IILQisInqSqlio(II_LQTYPE_I2 *indaddr,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT type,II_LQTYPE_NAT len,II_LQTYPE_GPTR addr,
		II_LQTYPE_NAT attrib);
void	IILQldh_LoDataHandler(II_LQTYPE_NAT type,II_LQTYPE_I2 *indvar, 
		II_LQTYPE_NAT ((*datahdlr)(II_LQTYPE_GPTR)),
		II_LQTYPE_PTR hdlr_arg);
II_LQTYPE_LONGNAT	IILQled_LoEndData(void);
void	IILQlgd_LoGetData(II_LQTYPE_NAT flags,II_LQTYPE_NAT hosttype, 
		II_LQTYPE_LONGNAT hostlen,char *addr,
		II_LQTYPE_LONGNAT maxlen,II_LQTYPE_LONGNAT *seglen, 
		II_LQTYPE_NAT *dataend);
void	IILQlpd_LoPutData(II_LQTYPE_NAT flags,II_LQTYPE_NAT hosttype,
		II_LQTYPE_LONGNAT hostlen,char *addr,II_LQTYPE_LONGNAT seglen,
		II_LQTYPE_NAT dataend);
II_LQTYPE_VOID	IILQpriProcInit(II_LQTYPE_NAT proc_type,char *proc_name);
II_LQTYPE_NAT	IILQprsProcStatus(int);
II_LQTYPE_VOID	IILQprvProcValio(char *param_name,II_LQTYPE_NAT param_byref,
		II_LQTYPE_I2 *indaddr,II_LQTYPE_NAT isref,II_LQTYPE_NAT type,
		II_LQTYPE_NAT length,II_LQTYPE_GPTR addr1, ...);
II_LQTYPE_VOID	IILQprvProcGTTParm(char *param_name, char *temptab_name);
void	IILQshSetHandler(II_LQTYPE_NAT hdlr,II_LQTYPE_NAT (*funcptr)());
II_LQTYPE_VOID	IILQsidSessID(II_LQTYPE_NAT session_id);
void	IILQssSetSqlio(II_LQTYPE_NAT attrib,II_LQTYPE_I2 *indaddr,
		II_LQTYPE_NAT isvar,II_LQTYPE_NAT type,II_LQTYPE_NAT len,
		II_LQTYPE_GPTR data);
II_LQTYPE_NAT	IInexec(void);
II_LQTYPE_NAT	IInextget(void);
II_LQTYPE_NAT	IIparret(char *tlist,char **argv,II_LQTYPE_NAT (*xfunc)());
II_LQTYPE_NAT	IIparset(char *string,char **argv,
#ifdef  CL_HAS_PROTOS
		char *(*transfunc)(II_LQTYPE_PTR,II_LQTYPE_PTR,
		II_LQTYPE_PTR));
#else
		char *(*transfunc)());
#endif
II_LQTYPE_VOID	IIputctrl(II_LQTYPE_NAT ctrl);
void	IIputdomio(II_LQTYPE_I2 *indaddr,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT type,II_LQTYPE_LONGNAT length,
		II_LQTYPE_GPTR addr);
void	IIretinit(char *file_nm,II_LQTYPE_NAT line_no);
II_LQTYPE_LONGNAT	(* IIseterr(II_LQTYPE_NAT (*proc)(II_LQTYPE_NAT *errno)))();
void	IIsexec(void);
void	IIsqConnect(II_LQTYPE_NAT lang,char *db,char *a1,char *a2,char *a3,
		char *a4,char *a5,char *a6,char *a7,char *a8,char *a9,
		char *a10,char *a11, char *a12,char *a13);
void	IIsqDaIn(II_LQTYPE_NAT lang, II_LQTYPE_GPTR sqd);
void 	IIsqDescInput(II_LQTYPE_NAT lang,char *stmt_name,II_LQTYPE_GPTR sqd);
void 	IIsqDescribe(II_LQTYPE_NAT lang,char *stmt_name,II_LQTYPE_GPTR sqd,
		II_LQTYPE_NAT using_flag);
void	IIsqDisconnect(void);
void	IIsqExImmed(char *query);
void	IIsqExStmt(char *stmt_name,II_LQTYPE_NAT using_vars);
void	IIsqFlush(char *filename,II_LQTYPE_NAT linenum);
II_LQTYPE_VOID	IIsqGdInit(II_LQTYPE_I2 sqlsttype,char *sqlstdata);
void	IIsqInit(II_LQTYPE_GPTR sqc);
void	IIsqMods(II_LQTYPE_NAT mod_flag);
II_LQTYPE_VOID	IIsqParms(II_LQTYPE_NAT flag,char * name_spec,
		II_LQTYPE_NAT val_type,char *str_val, II_LQTYPE_NAT int_val);
void	IIsqPrepare(II_LQTYPE_NAT lang,char *stmt_name,II_LQTYPE_GPTR sqd,
		II_LQTYPE_NAT using_flag,char *query);
II_LQTYPE_VOID	IIsqPrint(II_LQTYPE_GPTR sqlca);
void	IIsqStop(II_LQTYPE_GPTR sqc);
void	IIsqTPC(II_LQTYPE_NAT highdxid,II_LQTYPE_NAT lowdxid);
void	IIsqUser(char * usename);
II_LQTYPE_VOID	IIsqlcdInit(II_LQTYPE_GPTR sqc,II_LQTYPE_NAT *sqlcd);
void	IIsyncup(char *file_nm,II_LQTYPE_NAT line_no);
II_LQTYPE_NAT	IItupget(void);
void	IIutsys(II_LQTYPE_NAT uflag,char *uname,char *uval);
void	IIvarstrio(II_LQTYPE_I2 *indnull,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT type,II_LQTYPE_NAT length,II_LQTYPE_GPTR addr);
void	IIwritio(II_LQTYPE_NAT trim,II_LQTYPE_I2 *ind_unused,
		II_LQTYPE_NAT isref_unused,II_LQTYPE_NAT type,
		II_LQTYPE_NAT length,char *qry);
void	IIxact(II_LQTYPE_NAT what);

/********************************************/
/* FRS calls - listed in alphabetical order */
/********************************************/

II_LQTYPE_NAT	IIactcomm(II_LQTYPE_NAT commval,II_LQTYPE_NAT val);
II_LQTYPE_NAT	IIactscrl(char *tabname,II_LQTYPE_NAT mode,II_LQTYPE_NAT val);
II_LQTYPE_NAT	IIaddform(II_LQTYPE_GPTR frame);
II_LQTYPE_NAT	IIchkfrm(void);
II_LQTYPE_NAT	IIclrflds(void);
II_LQTYPE_NAT	IIclrscr(void);
II_LQTYPE_NAT	IIdispfrm(char *nm,char *md);
II_LQTYPE_VOID	IIendforms(void);
II_LQTYPE_NAT	IIendfrm(void);
II_LQTYPE_NAT	IIendmu(void);
II_LQTYPE_VOID	IIendnest(void);
II_LQTYPE_NAT	IIfldclear(char *strvar);
II_LQTYPE_NAT	IIfnames(void);
II_LQTYPE_NAT	IIforminit(char *nm);
II_LQTYPE_NAT	IIforms(II_LQTYPE_NAT language);
II_LQTYPE_NAT	IIFRaeAlerterEvent(II_LQTYPE_NAT intr_val);
II_LQTYPE_NAT	IIFRafActFld(char *strvar,II_LQTYPE_NAT entry_act,
		II_LQTYPE_NAT val);
II_LQTYPE_VOID	IIFRgotofld(II_LQTYPE_NAT dir);
II_LQTYPE_VOID	IIFRgpcontrol(II_LQTYPE_NAT state,II_LQTYPE_NAT alt);
II_LQTYPE_VOID	IIFRgpsetio(II_LQTYPE_NAT pid,II_LQTYPE_I2 *nullind,
		II_LQTYPE_BOOL isvar,II_LQTYPE_NAT type,II_LQTYPE_NAT len,
		II_LQTYPE_PTR val);
II_LQTYPE_VOID	IIFRInternal(II_LQTYPE_NAT dummy);
II_LQTYPE_NAT	IIFRitIsTimeout(void);
II_LQTYPE_NAT	IIFRreResEntry(void);
II_LQTYPE_VOID	IIFRsaSetAttrio(II_LQTYPE_NAT fsitype,char *cname, 
		II_LQTYPE_I2 *nullind,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT datatype,II_LQTYPE_NAT datalen,
		II_LQTYPE_PTR value);
II_LQTYPE_BOOL	IIfrshelp(II_LQTYPE_NAT type,char *subj,char *flnm);
II_LQTYPE_VOID	IIFRsqDescribe(II_LQTYPE_NAT lang,II_LQTYPE_NAT is_form,
		char *form_name,char *table_name,char *mode,
		II_LQTYPE_GPTR sqd);
II_LQTYPE_VOID	IIFRsqExecute(II_LQTYPE_NAT lang,II_LQTYPE_NAT is_form,
		II_LQTYPE_NAT is_input,II_LQTYPE_GPTR sqd);
II_LQTYPE_NAT	IIFRtoact(II_LQTYPE_NAT timeout,II_LQTYPE_NAT intr_val);
II_LQTYPE_NAT	IIfsetio(char *nm);
II_LQTYPE_NAT	IIfsinqio(II_LQTYPE_I2 *nullind,II_LQTYPE_NAT variable,
		II_LQTYPE_NAT type,II_LQTYPE_NAT len,II_LQTYPE_GPTR data,
		char *name);
II_LQTYPE_NAT	IIfssetio(char *name,II_LQTYPE_I2 *nullind,
		II_LQTYPE_NAT isvar,II_LQTYPE_NAT type,II_LQTYPE_NAT len,
		II_LQTYPE_PTR data);
II_LQTYPE_NAT	IIgetfldio(II_LQTYPE_I2 *ind,II_LQTYPE_NAT variable,
		II_LQTYPE_NAT type,II_LQTYPE_NAT len,II_LQTYPE_PTR data,
		char *name);
II_LQTYPE_NAT	IIgetoper(II_LQTYPE_NAT set);
II_LQTYPE_NAT	IIgtqryio(II_LQTYPE_I2 *ind,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT type,II_LQTYPE_NAT len,II_LQTYPE_NAT *variable,
		char *name);
II_LQTYPE_BOOL	IIhelpfile(char *subj,char *flnm);
II_LQTYPE_BOOL  IIhelptoggle(II_LQTYPE_BOOL flag);
II_LQTYPE_NAT	IIinitmu(void);
II_LQTYPE_NAT	IIinqset(char *object,char *p0,char *p1,char *p2,char *p3);
II_LQTYPE_NAT	IIiqfsio(II_LQTYPE_I2 *nullind,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT type,II_LQTYPE_NAT length,II_LQTYPE_PTR data,
		II_LQTYPE_NAT attr,char *name,II_LQTYPE_NAT row);
II_LQTYPE_NAT	IIiqset(II_LQTYPE_NAT objtype,II_LQTYPE_NAT row,
		char *formname,char *fieldname,char *term);
II_LQTYPE_NAT	IImessage(char *buf);
II_LQTYPE_NAT	IImuonly(void);
II_LQTYPE_NAT	IInestmu(void);
II_LQTYPE_NAT	IInfrskact(II_LQTYPE_NAT frsknum,char *exp,II_LQTYPE_NAT val,
		II_LQTYPE_NAT act,II_LQTYPE_NAT intrval);
II_LQTYPE_NAT	IInmuact(char *strvar,char *exp,II_LQTYPE_NAT val,
		II_LQTYPE_NAT act,II_LQTYPE_NAT intrp);
II_LQTYPE_VOID	IIprmptio(II_LQTYPE_NAT noecho,char *prstring,
		II_LQTYPE_I2 *nullind,II_LQTYPE_NAT isvar,II_LQTYPE_NAT type,
		II_LQTYPE_NAT length,II_LQTYPE_PTR data);
II_LQTYPE_VOID	IIprnscr(char *filename);
II_LQTYPE_NAT	IIputfldio(char *s1,II_LQTYPE_I2 *ind,II_LQTYPE_NAT isvar,
		II_LQTYPE_NAT type,II_LQTYPE_NAT len,II_LQTYPE_PTR data);
II_LQTYPE_NAT	IIputoper(II_LQTYPE_NAT set);
II_LQTYPE_NAT	IIredisp(void);
II_LQTYPE_NAT	IIrescol(char *tabname,char *colname);
II_LQTYPE_NAT	IIresfld(char *strvar);
II_LQTYPE_NAT	IIresmu(void);
II_LQTYPE_NAT	IIresnext(void);
II_LQTYPE_NAT	IIretval(void);
II_LQTYPE_NAT	IIrf_param(char *infmt,char **inargv,
		II_LQTYPE_NAT (*transfunc)());
II_LQTYPE_NAT	IIrunform(void);
II_LQTYPE_NAT	IIrunmu(II_LQTYPE_NAT dispflg);
II_LQTYPE_NAT	IIrunnest(void);
II_LQTYPE_VOID	IIset_random(II_LQTYPE_NAT seed);
II_LQTYPE_NAT	IIsf_param(char *infmt,char **inargv,
		II_LQTYPE_NAT (*transfunc)());
II_LQTYPE_NAT	IIsleep(II_LQTYPE_NAT sec);
II_LQTYPE_NAT	IIstfsio(II_LQTYPE_NAT attr,char *name,II_LQTYPE_NAT row,
		II_LQTYPE_I2 *nullind,II_LQTYPE_NAT isvar,II_LQTYPE_NAT type,
		II_LQTYPE_NAT length,II_LQTYPE_PTR tdata);
II_LQTYPE_NAT	IItbact (char *frmnm,char *tabnm,II_LQTYPE_NAT loading);
II_LQTYPE_NAT	IITBcaClmAct(char *tabname,char *colname,
		II_LQTYPE_NAT entry_act,II_LQTYPE_NAT val);
II_LQTYPE_VOID	IITBceColEnd(void);
II_LQTYPE_NAT	IItbinit (char *formname,char *tabname,char *tabmode);
II_LQTYPE_NAT	IItbsetio(II_LQTYPE_NAT cmnd,char *formname,char *tabname,
		II_LQTYPE_NAT rownum);
II_LQTYPE_NAT	IItbsmode(char *modestr);
II_LQTYPE_NAT	IItclrcol(char *colname);
II_LQTYPE_NAT	IItclrrow(void);
II_LQTYPE_NAT	IItcogetio(II_LQTYPE_I2 *ind,II_LQTYPE_NAT variable,
		II_LQTYPE_NAT type,II_LQTYPE_NAT len,II_LQTYPE_PTR data,
		char *colname);
II_LQTYPE_NAT	IItcolval(char *colname);
II_LQTYPE_NAT	IItcoputio(char *colname,II_LQTYPE_I2 *ind,
		II_LQTYPE_NAT variable,II_LQTYPE_NAT type,
		II_LQTYPE_NAT len,II_LQTYPE_PTR data);
II_LQTYPE_NAT	IItdata(void);
II_LQTYPE_NAT	IItdatend(void);
II_LQTYPE_NAT	IItdelrow(II_LQTYPE_NAT tofill);
II_LQTYPE_NAT	IItfill(void);
II_LQTYPE_NAT	IIthidecol(char *colname,char *coltype);
II_LQTYPE_NAT	IItinsert(void);
II_LQTYPE_NAT	IItrc_param(char *infmt,char **inargv,
		II_LQTYPE_NAT (*transfunc)());
II_LQTYPE_NAT	IItscroll(II_LQTYPE_NAT tofill,II_LQTYPE_NAT recnum);
II_LQTYPE_NAT	IItsc_param(char *infmt,char **inargv,
		II_LQTYPE_NAT (*transfunc)());
II_LQTYPE_NAT	IItunend(void);
II_LQTYPE_BOOL	IItunload(void);
II_LQTYPE_NAT	IItvalrow(void);
II_LQTYPE_NAT	IItvalval(II_LQTYPE_NAT set);
II_LQTYPE_NAT	IIvalfld(char *strvar);

/**************************************/
/* EXEC 4GL calls - to be added later */
/**************************************/
II_LQTYPE_NAT   IIG4acArrayClear(II_LQTYPE_I4 array);
II_LQTYPE_NAT   IIG4bpByrefParam(II_LQTYPE_I2 *ind, II_LQTYPE_NAT isval, 
                II_LQTYPE_NAT type, II_LQTYPE_NAT length, 
                II_LQTYPE_PTR data, char *name);
II_LQTYPE_NAT   IIG4ccCallComp();
II_LQTYPE_NAT   IIG4chkobj(II_LQTYPE_I4 object, II_LQTYPE_NAT access,
                II_LQTYPE_NAT index,  II_LQTYPE_NAT  caller);
II_LQTYPE_NAT   IIG4drDelRow(II_LQTYPE_I4 array, II_LQTYPE_NAT  index);
II_LQTYPE_NAT   IIG4fdFillDscr(II_LQTYPE_I4 object, II_LQTYPE_NAT  language, 
                II_LQTYPE_GPTR sqd);
II_LQTYPE_NAT   IIG4gaGetAttr(II_LQTYPE_I2 *ind, II_LQTYPE_NAT  isvar, 
                II_LQTYPE_NAT  type, II_LQTYPE_NAT  length, 
                II_LQTYPE_PTR data, char *name);
II_LQTYPE_NAT   IIG4ggGetGlobal(II_LQTYPE_I2 *ind, II_LQTYPE_NAT  isvar, 
                II_LQTYPE_NAT  type, II_LQTYPE_NAT  length, 
                II_LQTYPE_PTR data, char *name, II_LQTYPE_NAT  gtype);
II_LQTYPE_NAT   IIG4grGetRow(II_LQTYPE_I2 *rowind, II_LQTYPE_NAT  isvar, 
                II_LQTYPE_NAT  rowtype, II_LQTYPE_NAT  rowlen, 
                II_LQTYPE_PTR rowptr, II_LQTYPE_I4 array, II_LQTYPE_NAT  index);
II_LQTYPE_NAT   IIG4i4Inq4GL(II_LQTYPE_I2 *ind, II_LQTYPE_NAT  isvar, 
                II_LQTYPE_NAT  type, II_LQTYPE_NAT  length, 
                II_LQTYPE_PTR data, II_LQTYPE_I4 object, II_LQTYPE_NAT  code);
II_LQTYPE_NAT   IIG4icInitCall(char *name, II_LQTYPE_NAT  type);
II_LQTYPE_NAT   IIG4irInsRow(II_LQTYPE_I4 array, II_LQTYPE_NAT  index, 
                II_LQTYPE_I4 row, II_LQTYPE_NAT  state, II_LQTYPE_NAT  which);
II_LQTYPE_NAT   IIG4rrRemRow(II_LQTYPE_I4 array, II_LQTYPE_NAT  index);
II_LQTYPE_NAT   IIG4rvRetVal(II_LQTYPE_I2 *ind, II_LQTYPE_NAT  isval, 
                II_LQTYPE_NAT  type, II_LQTYPE_NAT  length, II_LQTYPE_PTR data);
II_LQTYPE_NAT   IIG4s4Set4GL(II_LQTYPE_I4 object, II_LQTYPE_NAT  code, 
                II_LQTYPE_I2 *ind, II_LQTYPE_NAT  isvar, II_LQTYPE_NAT  type, 
                II_LQTYPE_NAT  length, II_LQTYPE_PTR data);
II_LQTYPE_NAT   IIG4saSetAttr(char *name, II_LQTYPE_I2 *ind, 
                II_LQTYPE_NAT  isvar, II_LQTYPE_NAT  type, 
                II_LQTYPE_NAT  length, II_LQTYPE_PTR data);
II_LQTYPE_NAT   IIG4seSendEvent(II_LQTYPE_I4 frame);
II_LQTYPE_NAT   IIG4sgSetGlobal(char *name, II_LQTYPE_I2 *ind, 
                II_LQTYPE_NAT  isvar, II_LQTYPE_NAT type, II_LQTYPE_NAT length,
                II_LQTYPE_PTR data);
II_LQTYPE_NAT   IIG4srSetRow(II_LQTYPE_I4 array, II_LQTYPE_NAT index, 
                II_LQTYPE_I4 row);
II_LQTYPE_NAT   IIG4udUseDscr(II_LQTYPE_NAT language, II_LQTYPE_NAT direction, 
                II_LQTYPE_GPTR *sqd);
II_LQTYPE_NAT   IIG4vpValParam(char *name, II_LQTYPE_I2 *ind, 
                II_LQTYPE_NAT  isval, II_LQTYPE_NAT type, II_LQTYPE_NAT length,
                II_LQTYPE_PTR data);

%#ifdef __cplusplus
}
%#endif

/* Ensure next line is the last in the file */
%#endif /% EQDEFC_H_INCLUDED %/
