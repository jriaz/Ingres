# include	<compat.h>
# include	<gl.h>
# include	<systypes.h>
# include	<lo.h>
# include	<si.h>
# include	<st.h>
# include	<pc.h>
# include	<me.h>
# include	<ds.h>
# include	<ut.h>
# include	<nm.h>
# include       <clconfig.h>

# ifndef DESKTOP
# include	<sys/param.h>
# include	<sys/stat.h>
# endif /* DESKTOP */
# include	<errno.h>

# ifdef DESKTOP
# define RESPONSENAME 	"ut_image.lnk"
# define DEFNAME 	"ut_image.def"
# define MSLINK	      	0
# endif /* DESKTOP */
#ifndef DESKTOP
#define LIBNAME "ut_image.a"
#endif

/*
**Copyright (c) 2004 Ingres Corporation
**
** UTlnk.c -- Link an executable.
**
**	Parameters:
**		objs -- (input only)  An array of pointers to the locations
**			of the files to be linked.
**
**		usrlist -- (input only)  An array of pointers to some strings
**			to be passed to the linker (often a list of libraries).
**
**		exe -- (input only)  The location of a file where the linked
**			object is to be placed.
**
**	History:
**	Log:	utlnk.c,v $
**		
**		Revision 1.2  86/04/25  10:35:37  daveb
**		Can't use -r on System V.
**		
**		Revision 30.2  85/12/03  15:03:52  cgd
**		commented out -r option to /bin/ld.
**		Caused executables to be non-executable on Burroughs.
**		
**		Revision 3.24  85/07/12  03:08:11  greg
**		pid must be PID not PID *
**		
**		Revision 3.23  85/06/19  20:50:37  wong
**		Remove local symbols to save some space.
**		
**		12/84 (jhw) -- 3.0 Integration; changed `libs', a list of
**			locations, to `usrlist', a list of strings.
**		24-aug-87 (boba) -- Changed memory allocation to use MEreqmem.
**
**		13-oct-87 (bestwick) -- Altered to cd to object code directory
**			and then to use relative path names for object files.
**			This results in much shorter command lines, avoiding
**			problems that long lines cause on Uniplus+ machines
**			(British Telecom and ICL Clan4/Datamedia).  Also,
**			defined II_UT_TRACE logical that dumps ld command and
**			output to file: usage is 'setenv II_UT_TRACE /dev/tty'.
**			Finally, tidied ifdef's and added comments.
**
**		23-nov-87 (russ) -- Altered to read the ld(1) command line
**			from the file $ING_HOME/files/utld.def.  If a two
**			letter word beginning with the character '%' is read
**			from the file, it is interpreted as follows:
**
**				%O - the name of the output file is substituted
**				%D - the name of the directory containing the
**				     object files is substituted
**				%i - the input files, the list of object files
**				     from the objs[] argument, is substituted
**				     (with relative path names)
**				%L - the libraries, the list of libraries from
**				     the usrlist[] argument, is substituted
**				%% - the character '%' is substituted
**
**		6/88 (bobm) - integrate into 6.0 - change ST[r]index calls to
**			reflect KANJI changes (char * for character).
**
**		1/89 (bobm) - made provisions for list of objects consisting
**			of 1 file (prevent LOtos(0, ..) in this case).
**		27-Feb-1989 (fredv)
**			Use systypes.h instead of sys/types.h.
**		08-jan-90 (sylviap) 
**			Added PCcmdline parameters.
**      	08-mar-90 (sylviap)
**              	Added a parameter to PCcmdline call to pass a 
**			CL_ERR_DESC.
**		19-mar-90 (Mike S)
**			Add errfile, pristine, and clerror parameters.
**		31-may-90 (blaise)
**		    Integrated changes from ingresug:
**			Add include for <clconfig.h>;
**			If xCL_070_LIMITS_H_EXISTS is defined, include 
**			<limits.h> and define CMDLINSIZ as ARG_MAX.
**		07-mar-91 (mikem)
**		    This file would not compile (because of undefined ARG_MAX) 
**		    on sun4's with new clsecret.h generated by mksecret.sh.  
**		    On some systems the definition of ARG_MAX can be found
**		    in limits.h, this is not true on a sun4.
**		    Fixed this by changing the definition of CMDLINSIZ to 
**		    depend on whether or not ARG_MAX is defined.
**		4-feb-92 (mgw)
**			added trace file handling. Updated II_UT_TRACE
**			flag with this new functionality.
**		16-feb-92 (smc)
**		        Added forward declarations.
**	15-jul-93 (ed)
**	    adding <gl.h> after <compat.h>
**	19-apr-95 (canor01)
**	    added <errno.h>
**	20-apr-95 (emmag)
**	    Desktop porting changes.
**      25-Jul-95 (fanra01)
**          Rearranged the link order and options to match MS LINK for NT.
**          Also removed the lines which generate the .DEF file.  These
**          options can be added to the link response file via the .opt files.
**      25-Jul-95 (fanra01)
**          Removed entry of .DEF file name into linker response file.
**      16-Aug-95 (fanra01)
**          Added a compare for .RES files when outputing the library list into
**          the linker response file.  This is so resource files for windows
**          can be included.
**	13-dec-95 (morayf)
**          Added NCARGS def for SNI RMx00 (rmx_us5). ARG_MAX is not set in
**          <limits.h> for mips processors. It is a tunable, and should
**          really be determined using sysconf(_SC_ARG_MAX). This is no use
**          for allocating array bounds though, so I hard-wired the maximum
**          mtune value (51200) into NCARGS (also undefined for SNI) on the
**          basis that its better to over-allocate an array and not use some
**          of it than to allow the program to crash/run incorrectly due to
**          the array bounds being exceeded. 
**	23-feb-96 (morayf)
**	    Same NCARGS thing for Pyramid port.
**	10-oct-1996 (canor01)
**	    Allow generic override for SystemLocationVariable (II_SYSTEM).
**      02-jun-97 (mcgem01)
**          The maximum command line length on NT is MAX_CMDLINE.
**	26-Nov-1997 (allmi01)
**	    Added sgi_us5 to list of platforms which set NCARGS.
**	17-nov-1997 (canor01)
**	    Quote parameters in response file to support embedded spaces.
**      21-jan-1999 (hanch04)
**          replace nat and longnat with i4
**      21-apr-1999 (hanch04)
**        Replace STrindex with STrchr
**	12-May-1999 (rigka01)
**	    Bug 96873 - During the building of an ABF image, linking of
**	    the application fails when the link command exceeds the size of 
**	    the maximum command line length for the operating system.
**	    Fix is to put objects in a temporary library and link with the
**	    library instead of listing all objects in the link command.
**      03-jul-1999 (podni01)
**          Added support for ReliantUNIX V 5.44 - rux_us5 (same as rmx_us5)
**	28-Sep-1999 (rigka01)
**	    Bug 98629 - Fix performance problem with link introduced by
**	    fix to bug 96873. Archive object files in large groups rather 
**	    than one by one. 
**	14-Oct-1999 (kitch01)
**	    Minor changes to the above fix in rigka01's absence. Client 
**	    reported that if the link fails the errors are no longer shown.
**	    this is because the last command executed, the removal of the
**	    ut_image.a is succesful and thus there are no errors.
**	03-Nov-1999 (kitch01)
**	    Further work on bugs 96873 and 98629. Now we check the command line
**	    length and if it has been exceeded we use the method described by
**	    the changes of 12-may-99, 28-sep-99 and 14-oct-99. If the command
**	    line length has not been exceeded we issue the link command as is.
**	21-jan-1999 (hanch04)
**	    replace nat and longnat with i4
**	31-aug-2000 (hanch04)
**	    cross change to main
**        replace nat and longnat with i4
**	11-Nov-1999 (jenjo02)
**	    Use CL_CLEAR_ERR instead of SETCLERR to clear CL_ERR_DESC.
**	    The latter causes a wasted ___errno function call.
**      29-Nov-1999 (hanch04)
**          First stage of updating ME calls to use OS memory management.
**          Make sure me.h is included.  Use SIZE_TYPE for lengths.
**      17-Nov-2000 (hweho01)
**          Changed to call stat() more than one time if the linking is    
**          successful, because the large image files need more time to    
**          be written to the disk.
**      18-Feb-2000 (hweho01)
**          Specified "-X64" when issue the ar command to build the 64-bit
**          application for ris_u64. On AIX 4.3.x, the default object mode
**          is "-X32" for the ar command.
**      23-Sep-2002 (hweho01)
**          Added change for hybrid build on AIX platform.
**      25-Mar-2004 (hanal04) Bug 112021
**          Mark 2252 build error caused by previous change. pid5 is
**          undeclared and should have been current_process instead.
**      31-aug-2004 (sheco02)
**          X-integrate change 467422 to main.
**      01-sep-2004 (sheco02)
**          X-integration mistake. Back out the previous change.
**	20-Jun-2009 (kschendel) SIR 122138
**	    Hybrid add-on symbol changed, fix here.
*/

/* forward declarations */

/*
** Maximum size of generated command line.  This should be set
** to the maximum length of arguments accepted by exec.  On
** most machines this will be NCARGS, from <sys/param.h>.
*/

# ifdef xCL_070_LIMITS_H_EXISTS
#     include <limits.h>
# endif /* xCL_070_LIMITS_H_EXISTS */

# ifdef ARG_MAX
#     define CMDLINSIZ    (ARG_MAX)
# else
# if defined(rmx_us5) || defined(pym_us5) || defined(sgi_us5)
#     define NCARGS           51200 /* NB - this is maximum value of tunable */
# elif defined(NT_GENERIC)
#     define NCARGS           MAX_CMDLINE
# endif
#     define CMDLINSIZ    (NCARGS > 40000 ? 40000 : NCARGS)
# endif /* ARG_MAX */

/* maximum size of one line in the utld.def file */
#define BUFSIZE 	512

STATUS
UTlink(objs, usrlist, exe, errfile, pristine, clerror)
LOCATION	*objs[];
char		*usrlist[];
LOCATION	*exe;
LOCATION	*errfile;
bool		*pristine;
CL_ERR_DESC	*clerror;
{
# ifndef DESKTOP
	register char		**lp;
	register LOCATION	**lt;
	STATUS			ret_val;
	LOCATION		work_dir;
	LOCATION		utld_file;
	char			*cp, *p1, *s;
	char			obj_dirname[200];
	char			image_name[200];

	char			cmdline[CMDLINSIZ];
	i4			size_cmd;
	char			*str_i, *str_L;
	u_i4			size_i, size_L;
	char			buf[BUFSIZE];
	FILE			*fp;
	i4			stri_mark, strL_mark;

	/*
 	** Tracing is always available, turned on by setting II_UT_TRACE
	** to the name of the trace file.
 	*/
	FILE			*Trace = NULL;

	struct stat		istat;
	i4			hdrcmdlinelen;
	i4                      count, ret_stat;

	CL_CLEAR_ERR( clerror );
	*pristine = TRUE;	/* We can redirect all output on Unix */

	/*
	** Read data file.  If II_UTLD_FILE is set, use this
	** file, otherwise use $ING_HOME/files/utld.def.
	*/

	NMgtAt("II_UTLD_FILE",&s);
	if (s == NULL || *s == EOS) {
		ret_val = NMf_open("r","utld.def",&fp);
	} else {
		LOfroms( PATH & FILENAME, s, &utld_file);
		ret_val = SIopen( &utld_file, "r", &fp );
	}
	if (ret_val != OK)
		return(UT_LD_DEF);

	/* Skip comment lines and blank lines. */
	while ((ret_val = SIgetrec(buf,BUFSIZE,fp)) == OK) {
		if (*buf != '\n' && *buf != '#')
			break;
	}

	/* Close file */
	_VOID_ SIclose(fp);

	/* check for errors */
	if (ret_val != OK)
		return(UT_LD_DEF);

	/* strip off newline */
	if ((s = STchr(buf,'\n')) != NULL)
		*s = EOS;

	/* 
	** If the name of image file is not a full path name,
	** construct the absolute path name of the image file
	** to generate, because the command to do the linking
	** may not be executed in the current directory.
	*/

	LOtos(exe, &cp);
	if (*cp == '/') {
		STcopy(cp,image_name);
	} else {
		/* add current dir to image name */
		LOgt(image_name, &work_dir);
		STcat(image_name, "/");
		STcat(image_name, cp);
	}

	/*
	** Generate a string with the name of the directory
	** containing the objects.  We will want to cd there
	** if using relative path names for the objects.
	*/
	if (objs[1] != NULL)
		LOtos(objs[1],&s);
	else
		LOtos(objs[0],&s);
	STcopy(s,obj_dirname);
	if ((s = STrchr(obj_dirname,'/')) != NULL)
		*s = EOS;

	/*
	** Build the list of objects to pass as subst strings to
	** II_subst for the code %i.  The first string
	** will be the path to $ING_HOME/lib/abfmain.obj, which
	** is the first element in the objs list.  For %i,
	** the relative path names are needed, so STrindex is
	** used to search for the last '/'.  The next character
	** is used as the beginning of the string containing
	** the file name of the object.
	*/

	/* get string representation of the path to abfmain.obj */
	LOtos(*objs,&cp); 

	/* calculate size of subst string for %i */
	size_i = STlength(cp) + 1;
	for (lt = objs+1; *lt != (LOCATION *)NULL; ++lt)
	{
		LOtos(*lt, &p1);
		if ((s = STrchr(p1, '/')) == NULL)
			size_i += STlength(p1) + 1;
		else
			size_i += STlength(s + 1) + 1;
	}

	/* allocate memory for subst string for %i */
	if ((str_i = (PTR)MEreqmem(0, size_i, FALSE, (STATUS *)NULL)) == 
		NULL )
	    return(UT_LD_OBJS);

	/* build string for %i */
	STprintf(str_i,"%s ",cp);
	lt = objs + 1;
	while (*lt != (LOCATION *)NULL)
	{
		LOtos(*lt, &p1);

		if ((s = STrchr(p1, '/')) == NULL)
			STcat(str_i,p1);
		else
			STcat(str_i,s + 1);

		lt++;

		/* add a space if another location is expected */
		if (*lt != (LOCATION *)NULL) {
			STcat(str_i," ");
		}
	}

	/* calculate size of subst string for %L */
	size_L = 0;
	for (lp = usrlist; *lp != (char *)NULL; ++lp)
	{
		size_L += STlength(*lp) + 1;
	}

	/* allocate memory for subst string for %L */
	if ((str_L = (PTR)MEreqmem(0, size_L, FALSE, (STATUS *)NULL)) == 
		NULL )
		return(UT_LD_OBJS);

	/* build string for %L */
	*str_L = EOS;
	lp = usrlist;
	while (*lp != (char *)NULL)
	{
		STcat(str_L,*lp);

		lp++;

		/* add a space if another string is expected */
		if (*lp != (char *) NULL)
			STcat(str_L," ");
	}

	/*
	** Build command line.
	*/

	size_cmd = CMDLINSIZ;
	II_subst('%',buf,&size_cmd,cmdline,'i',str_i,'L',str_L,
		'D',obj_dirname,'O',image_name,0);

	/* 
	**  If tracing is required, open the trace file.
	*/

	UTopentrace(&Trace);

	/* If the command line length has been exceeded in II_subst size_cmd
	** will be set to be one less than CMDLINSIZ. If it is not then
	** execute the command as is. If it is then use 'ar' to create an
	** archive of objectes required and then execute the link with this
	** archive. II_subst() can also return 0 to indicate a failure with
	** too many arguments in the arg array.
	*/
	if ((size_cmd != 0) && (size_cmd < CMDLINSIZ-1))
	{

		if (Trace)
		{
	  	   /*
		   **  Tracing on - dump the command 
		   */
		   SIfprintf(Trace, "Executing via cmdline. Size = %d\n"
								, size_cmd);
		   SIfprintf(Trace, "%s\n", cmdline);
		   _VOID_ SIclose(Trace);
		}

		/*
		**  Call PCdocmdline to execute the command.
		*/

		ret_val = PCdocmdline((LOCATION *)NULL, cmdline, PC_WAIT, FALSE, 
					errfile, clerror);

		/*
		** it seems that the loader is producing a functional image,
		** returning with no errors, but not making it executable.
		** We'll make sure of it here - set execute wherever we have
		** read.
		*/
                /*
                ** The first stat() would fail if the image file is large,  
                ** so try more.  
                */
                if( ret_val == OK && (( ret_stat = stat(image_name, &istat)) 
                                       != 0 ))
                    for ( count = 0; count <= 3 ; count++ ) 
                     {
                      if(( ret_stat = stat(image_name, &istat)) == 0 )
                         break;
                       PCsleep(500);
                     }
                if (ret_val == OK && ret_stat == 0)
		{
			istat.st_mode |= ((istat.st_mode & 0444) >> 2);
			chmod(image_name,istat.st_mode);
		}

		UTlogtrace(errfile, ret_val);	/* save error output to file */

		return(ret_val);
	}
	else
	{
		if (Trace)
		{
			SIfprintf(Trace, "Executing via archive command\n");
		}
		STprintf(str_i,"%s ",cp);
#if defined(su4_u42)
		STcopy("ar r ",cmdline);
#elif defined(any_aix) && defined(BUILD_ARCH64)
		STcopy("ar -X64 r ",cmdline);
#else
		STcopy("ar -r ",cmdline);
#endif
		STcat(cmdline,obj_dirname);
		STcat(cmdline,"/");
		STcat(cmdline, LIBNAME);
		hdrcmdlinelen=STlength(cmdline);
		lt = objs + 1;
		while (*lt != (LOCATION *)NULL)
		{
		   LOtos(*lt, &p1);

		   if ((STlength(cmdline) +
			 STlength(obj_dirname) +
			 STlength(p1) +
			 STlength(" / ")) < CMDLINSIZ)
		   {
		      STcat(cmdline, " ");
		      STcat(cmdline,obj_dirname);
		      STcat(cmdline,"/"); 
		      if ((s = STrchr(p1, '/')) == NULL)
		      {
		         /* add obj to temporary library */
		         STcat(cmdline,p1);
		      }
		      else
		      {
		         /* add obj to temporary library */
		         STcat(cmdline,s + 1);
		      }
		   }
		   else
		   {
		      /* cannot extend command so issue as is */
		      if (Trace)
		         SIfprintf(Trace, "Issue ar command,length=%d\n",
			        	STlength(cmdline));
	  	      ret_val = PCdocmdline((LOCATION *)NULL, cmdline, PC_WAIT,
			                   FALSE, errfile, clerror);
		      if (ret_val)
		      {
                 UTlogtrace(errfile, ret_val); /* write error to file */
                 if (Trace)
                    _VOID_ SIclose(Trace);
                 return(ret_val);
		      }
		      /* re-initialize the ar command */
		      *cmdline=0;
#if defined(su4_u42)
		      STcopy("ar r ",cmdline);
#elif defined(any_aix) && defined(BUILD_ARCH64)
		      STcopy("ar -X64 r ",cmdline);
#else
		      STcopy("ar -r ",cmdline);
#endif
		      STcat(cmdline,obj_dirname);
		      STcat(cmdline,"/");
		      STcat(cmdline, LIBNAME);
		      STcat(cmdline, " ");
		      STcat(cmdline,obj_dirname);
		      STcat(cmdline,"/"); 
		      if ((s = STrchr(p1, '/')) == NULL)
		      {
		         /* add obj to temporary library */
		         STcat(cmdline,p1);
		      }
		      else
		      {
		         /* add obj to temporary library */
		         STcat(cmdline,s + 1);
		      }
		   }
		   lt++;
		}
		if (STlength(cmdline) > hdrcmdlinelen)
		{
		   if (Trace)
		      SIfprintf(Trace, "Issue final ar command,length=%d\n",
				STlength(cmdline));
	  	   ret_val = PCdocmdline((LOCATION *)NULL, cmdline, PC_WAIT,
				         FALSE, errfile, clerror);
		   if (ret_val)
		   {
		      UTlogtrace(errfile, ret_val); /* write error to file */
		      if (Trace)
                 _VOID_ SIclose(Trace);
		      return(ret_val);
		   }
		}
		else
		   if (Trace)
		      SIfprintf(Trace, "Final ar command already issued\n");

		/* calculate size of subst string for %L */
		size_L = 0;
		size_L = STlength(obj_dirname) + STlength(LIBNAME) +
			STlength("/") +  STlength(" ");
		for (lp = usrlist; *lp != (char *)NULL; ++lp)
		{
			size_L += STlength(*lp) + 1;
		}

		/* allocate memory for subst string for %L */
		if ((str_L = (PTR)MEreqmem(0, size_L, FALSE, (STATUS *)NULL)) ==
		    NULL )
		{
		   SIfprintf(Trace, "Unable to allocate buffer of size %d",
				size_L);
		   _VOID_ SIclose(Trace);
		   return(UT_LD_OBJS);
		}

		/* build string for %L */
		*str_L = EOS;
		STcat(str_L,obj_dirname);
		STcat(str_L,"/"); 
		STcat(str_L,LIBNAME);
		STcat(str_L, " ");
		lp = usrlist;
		while (*lp != (char *)NULL)
		{
		   STcat(str_L,*lp);

		   lp++;

		   /* add a space if another string is expected */
		   if (*lp != (char *) NULL)
		      STcat(str_L," ");
		}

		/*
		** Build command line.
		*/

		size_cmd = CMDLINSIZ;
		II_subst('%',buf,&size_cmd,cmdline,'i',str_i,'L',str_L,
			'D',obj_dirname,'O',image_name,0);

		if (Trace)
		{
		   /*
		   **  Tracing on - dump the command 
		   */

		   SIfprintf(Trace, "%s\n", cmdline);
		}

                /* II_subst failed again. nothing we can do so abort */
                if ((size_cmd == 0) || (size_cmd == CMDLINSIZ-1))
                {
		   SIfprintf(Trace , 
			"Unable to link via archive as well. Line size = %d\n",
		        size_cmd);
                   _VOID_ SIclose(Trace);
                   return(UT_LD_OBJS);
                }

		/*
		**  Call PCdocmdline to execute the command.
		*/

		ret_val = PCdocmdline((LOCATION *)NULL, cmdline, PC_WAIT, FALSE, 
					errfile, clerror);
		UTlogtrace(errfile, ret_val);	/* save error output to file */

		/*
		** it seems that the loader is producing a functional image,
		** returning with no errors, but not making it executable.
		** We'll make sure of it here - set execute wherever we have
		** read.
		*/
		if (ret_val == OK && stat(image_name,&istat) == 0)
		{
			istat.st_mode |= ((istat.st_mode & 0444) >> 2);
			chmod(image_name,istat.st_mode);
		}

		/* remove ut_image.a if it exists */
 		STcopy("rm ",cmdline);
		STcat(cmdline,obj_dirname);
		STcat(cmdline,"/"); 
		STcat(cmdline, LIBNAME);
		if (Trace)
		{
			SIfprintf(Trace, "%s\n", cmdline);
			_VOID_ SIclose(Trace);
		}
		
		/* Lets not worry whether the remove was OK */
		_VOID_ PCdocmdline((LOCATION *)NULL, cmdline, PC_WAIT, FALSE, 
					(LOCATION *)NULL, clerror);

		/*  This ret_val is from the actual link step */
		return(ret_val);
	}

# else  /* DESKTOP */

	STATUS		ret_val;
	char		cmdline[128];
	char		*cp, *s;
	LOCATION	response_file;
	LOCATION	utlnk_file;
	CL_ERR_DESC	err_code[1];
	char		buf[128];
	FILE		*fpd;
	FILE		*fpl;
	i2		linker;
	char *		scanptr;
	i4		isLib = 0;
	char		filecopy[65];
	char **		usrlist2 = usrlist;

	/*
	** Tracing is always available, turned on by setting II_UT_TRACE
	** to the name of the trace file.
	*/

	char		*Tracename = NULL;
	FILE		*Trace = NULL;
	LOCATION	trace_file;

	*pristine = FALSE;	/* anything's possible, I guess */

	/*
	**  If tracing is required (ie: II_UT_TRACE is defined), open
	**  the trace file.
	*/
	NMgtAt("II_UT_TRACE", &Tracename );
	if(Tracename && *Tracename) 
	{
	    LOfroms( PATH & FILENAME, Tracename, &trace_file);
	    SIopen( &trace_file, "w", &Trace );
	}

	/*
	** Read data file.  If II_UTLD_FILE is set, use this
	** file, otherwise use $ING_HOME/files/utlnk.def.
	*/

	NMgtAt("II_UTLD_FILE",&s);
	if (s == NULL || *s == EOS) 
	{
	    ret_val = NMf_open("r","utlnk.def",&fpd);
	} 
	else 
	{
	    LOfroms( PATH & FILENAME, s, &utlnk_file);
	    ret_val = SIopen( &utlnk_file, "r", &fpd );
	}

	if (ret_val != OK)
	    return(UT_LD_DEF);

	/* Skip comment lines and blank lines. */
	while ((ret_val = SIgetrec(buf,(i4)sizeof(buf),fpd)) == OK) 
	{
	    if (*buf != '\n' && *buf != ';')
		break;
	}

	/* check for errors */
	if (ret_val != OK)
	{
	    ret_val = UT_LD_DEF;
	    goto ret;
	}

	/* strip off newline */
	if ((s = STchr(buf,'\n')) != NULL)
		*s = EOS;

	/* determine which linker is being used */
	for (s = buf-1; *++s; )
	{
		if (strnicmp(s, "LINK", 4) == 0)
		{
			linker = MSLINK;
			break;
		}
	}

	/*
	**  Open the linker response file.
	*/
	LOfroms( PATH & FILENAME, RESPONSENAME, &response_file);
	SIopen( &response_file, "w", &fpl );

	/*
	** Write out the executable name and the map name.
	*/
	LOtos(exe, &cp);
	STcopy(cp, cmdline);
	cp = cmdline + STlength(cmdline);
	while	(--cp > cmdline)
	{
		if	(*cp == '.')
		{
			*cp = '\0';
			break;
		}
		if	((*cp == '\\') || (*cp == '/'))
			break;
	}
	NMgtAt(SystemLocationVariable, &s);

	/*
	** write the name of the EXE file 
	*/
	SIfprintf(fpl, "/OUT:\"%s.exe\"\n", cmdline);

	/*
	** write the name of the MAP file 
	*/
	SIfprintf(fpl, "/MAP:\"%s.map\"\n", cmdline);


	/*
	**  Write out the user .OBJ strings.
	*/
	do
	{
	      	/* scan for ".OBJ" and if present, add this to the list */
	      	STcopy(*usrlist, filecopy);

		/* check for comment or blank line */
		for (scanptr = filecopy;
			*scanptr == '\t' && *scanptr == ' ';++scanptr)
			;
		if (*scanptr == ';' || *scanptr == 0)
		{
			continue;
		}

		/* check for link options */
		if (strnicmp(filecopy, "OPTIONS:", 8) == 0)
		{
			SIfprintf(fpl, "\"%s\"\n",&filecopy[8]);
			continue;
		}

		/* scan to the end of the line */
	      	for (scanptr = filecopy;*scanptr;++scanptr)
		 	;
	      	for ( ;scanptr >= filecopy && *scanptr != '\\';--scanptr)
	         	;
	      	++scanptr;
	      	for ( ;*scanptr && *scanptr != '.';++scanptr)
		 	;

		/* look for ".OBJ" */
		if (*scanptr != 0 && strnicmp(scanptr, ".OBJ", 4) == 0)
		{
	      		SIfprintf(fpl,"\"%s\"\n",filecopy);
		}

	} while	(*++usrlist);

	/*
	**  Write out the obj file names.
	*/
	while	(*objs)
	{
		LOtos(*objs++, &cp);
		SIfprintf(fpl, "\"%s\"", cp);

		SIfprintf(fpl, "%s", *objs ? "\n" : "\n");
	}

	/*
	**  Write out the user .LIB strings.
	*/
	do
	{
	      	/* look for ".LIB" and if present, add to the list */
	      	STcopy(*usrlist2, filecopy);

		/* check for comment or blank line */
		for (scanptr = filecopy;
			*scanptr == '\t' && *scanptr == ' ';++scanptr)
			;
		if (*scanptr == ';' || *scanptr == 0)
		{
			continue;
		}

		/* scan to the end of the line */
	      	for (scanptr = filecopy;*scanptr;++scanptr)
		 	;
	      	for ( ;scanptr >= filecopy && *scanptr != '\\';--scanptr)
	         	;
	      	++scanptr;
	      	for ( ;*scanptr && *scanptr != '.';++scanptr)
		 	;

		/* look for ".LIB" */
		if (*scanptr != 0 && ((strnicmp(scanptr, ".LIB", 4) == 0) ||
                                      (strnicmp(scanptr, ".RES", 4) == 0) ))
		{
                        SIfprintf(fpl,"\"%s\"\n",filecopy);
			isLib = 1;
		}

	} while	(*++usrlist2);

	/* if no libs are present then output an empty 
	   line so as to satify the Microsoft linker.  */
	if (isLib == 0)
	{
	}

	/*
	**  Write out the .DEF file name.
	*/

	_VOID_ SIclose(fpl);

	/*
	**  Open the .DEF response file and write it
	*/

	/* Add linker response filename to end of command */
	STcat(buf, RESPONSENAME);

	/*
	**  Execute the link command, then read in & execute any remaining cmds
	*/
	if(errfile)
	    LOdelete(errfile);
	while	(1)
	{

		if	(Trace)		SIfprintf(Trace, "%s\n", buf);

		/*  Call PCcmdline to execute the command  */
		if (ret_val = PCdowindowcmdline((LOCATION *)NULL, buf, PC_WAIT, FALSE,
					 HIDE, errfile, err_code))
			break;

		/* execute any remaining commands in the data file */

		/* Skip comment lines and blank lines. */
		while ((ret_val = SIgetrec(buf,(i4)sizeof(buf),fpd))==OK)
		{
			if (*buf != '\n' && *buf != ';')
				break;
		}
	
		/* check for end-of-file */
		if (ret_val != OK)
		{
			ret_val = OK;
			break;
		}
	
		/* strip off newline */
		if ((s = STchr(buf,'\n')) != NULL)
			*s = EOS;

		/* add executable name (without extension) to end of command */
		STcat(buf, cmdline);
	}
ret:
	/* Close files */
	_VOID_ SIclose(fpd);
	if	(Trace)
	{
		SIfprintf(Trace, "ret_val=0x%x\n", ret_val);
		_VOID_ SIclose(Trace);
	}
	return(ret_val);

# endif /* DESKTOP */
}
