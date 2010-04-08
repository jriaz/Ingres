/*
**Copyright (c) 2004 Ingres Corporation
*/
/*	@(#)y1.c	1.6	*/

/*
PROGRAM = byacc
**
NEEDLIBS = BYACCLIB COMPATLIB 
**
** History:
**	22-jan-93 (swm)
**	    If PRIVATE_TOOLS is set look for PARSER in $ING_SRC/$ING_VERS/bin
**	    instead of $ING_SRC/bin.
**	 6-sep-93 (swm)
**	    Rearranged NMgtAt code that determines location of PARSER so
**	    that its static data does not get overwritten before it is used.
**	21-sep-93 (peterk)
**	    Correct previous change: use ING_SRC value for PARSER path name
**	24-jan-1994 (gmanning)
**	    Use ING_SRC value for PARSER path for NT.
**	14-oct-93 (swm)
**	    Bug #56448
**	    Altered ERROR calls to conform to new portable interface.
**	    It is no longer a "pseudo-varargs" function. It cannot become a
**	    varargs function as this practice is outlawed in main line code.
**	    Instead it takes a formatted buffer as its only parameter.
**	    Each call which previously had additional arguments has an
**	    STprintf embedded to format the buffer to pass to ERROR.
**	    This works because STprintf is a varargs function in the CL.
**	01-feb-1994 (wolf) 
**	    Add missing #else in the series of #ifdefs that builds up the
**	    name/path of the parser.
**      14-feb-1995 (tutto01)
**          Check to see if pointer is NULL before using it.
**          This was causing segmentation faults while compiling in psl.
**	23-sep-1996 (canor01)
**	    Move global data definitions to yaccdata.c
**	02-jul-1999 (schte01)
**       Changed int to i4  to accommodate 64-bit definition of i4  to long.
**       This is a cross-integration of the ingres30 change 441406, modified  
**       for int->nat.
**	21-jan-1999 (hanch04)
**	    replace nat and longnat with i4
**	31-aug-2000 (hanch04)
**	    cross change to main
**	    replace nat and longnat with i4
**	24-Jan-2004 (schka24)
**	    Fix readonly placement on function table to get rid of those
**	    really annoying compile messages on the ytab output.
**	30-Jan-2004 (hanje04)
**	    If ING_VERS isn't set a SEGV can ocur in others(). Set vers*=NULL
**	    when ING_VERS isn't set. 
**	09-jun-2004 (abbjo03)
**	    Modify 24-jan-2004 change to use 'const' instead of READONLY, so
**	    that it compiles without errors on VMS.
**	29-Sep-2004 (drivi01)
**	    Removed MALLOCLIB from NEEDLIBS
**  26-Jun-2006 (raddo01)
**		bug 116276: SEGV on double free() when $ING_SRC/lib/ doesn't exist.
**	10-Jul-2006 (bonro01)
**	    Fix compile error from last change.  Move prototype out of
**	    main function.
**	13-May-2009 (kschendel) b122041
**	    Compiler warning fixes.
**      2-Jul-2009 (kschendel) SIR 122138
**         9-apr removal of int -> long define on lp64 caused segv on 64 bit
**         arch with alignment restrictions.  When using the mem[] array for
**         struct items, make sure it's aligned.
*/

# include   <compat.h>
# include   <lo.h>
# include   <si.h>
# include   <st.h>
# include   <me.h>
# include   "dextern.h"

/* variables used locally */

/* lookahead computations */

GLOBALREF i4		tbitset;	/* size of lookahead sets */
GLOBALREF struct looksets lkst[LSETSIZE];
GLOBALREF i4		nlset;	/* next lookahead set index */
/* flag to suppress lookahead computations */
GLOBALREF i4		nolook;
/* temp storage for lookahead computations */
GLOBALREF struct looksets clset;

/* working set computations */

GLOBALREF struct wset	wsets[WSETSIZE];
GLOBALREF struct wset	*cwp;

/* state information */

GLOBALREF i4		nstate;	/* number of states */
/* ptrs to descriptions of the states */
GLOBALREF struct item	*pstate[NSTATES + 2];
/* contains type info about states */
GLOBALREF i4		tystate[NSTATES];
/* index to the stored goto table */
GLOBALREF i4		indgo[NSTATES];
/* states generated by terminal gotos */
GLOBALREF i4		tstates[NTERMS];
/* states generated by nonterm gotos */
GLOBALREF i4		ntstates[NNONTERM];
/* chain of ovflows of term/nonterm generation lists */
GLOBALREF i4		mstates[NSTATES];

/* storage for the actions in the parser */

GLOBALREF i4		amem[ACTSIZE];    /* action table storage */
/* next free action table position */
GLOBALREF i4		*memp;

/* other storage areas */

/* temp storage, indexed by terms + ntokens or states */
GLOBALREF i4		temp1[TEMPSIZE];
GLOBALREF i4		lineno;		/* current input line number */
GLOBALREF i4		fatfl;		/* if on, error is fatal */
GLOBALREF i4		nerrors;		/* number of errors */

/* storage for information about the nonterminals */

/* vector of pointers to productions yielding each nonterminal */
GLOBALREF i4		**pres[NNONTERM + 2];
/* vector of pointers to first sets for each nonterminal */
GLOBALREF struct looksets *pfirst[NNONTERM + 2];
/* vector of nonterminals nontrivially deriving e */
GLOBALREF i4		pempty[NNONTERM + 1];

extern VOID closeall();

main(argc, argv)
int argc;
char *argv[];
{
    char stbuf[STBUFSIZE];
    MEadvise(ME_INGRES_ALLOC);

    memp = amem;

    /* make sure that whoever changed the parameters got it right */

    if (sizeof(struct looksets) * BITSPERBYTE < NTERMS + 1)
	ERROR(STprintf(stbuf, "TBITSET too small for NTERMS.  Rebuild %s",
	    argv[0]));
    if (WSETSIZE < NNONTERM)
	ERROR(STprintf(stbuf, "WSETSIZE too small.  Rebuild %s", argv[0]));
    if (LSETSIZE < NNONTERM)
	ERROR(STprintf(stbuf, "LSETSIZE too small.  Rebuild %s", argv[0]));
    if (TEMPSIZE < NTERMS + NNONTERM + 1 || TEMPSIZE < NSTATES)
	ERROR(STprintf(stbuf, "TEMPSIZE too small.  Rebuild %s", argv[0]));

    setup(argc, argv);		/* initialize and read productions */
    tbitset = NWORDS(ntokens);

    /* make table of which productions yield a given nonterminal */
    cpres();

    /* make a table of which nonterminals can match the empty string */
    cempty();

    cpfir();		    /* make a table of firsts of nonterminals */
    stagen();		    /* generate the states */
    output();		    /* write the states and the tables */
    go2out();
    hideprod();
    summary();
    callopt();
    others();
	closeall();
    PCexit(OK);
}

VOID
closeall()
{
    if (finput)
        (VOID)SIclose(finput);
    if (faction)
    {
        (VOID)SIflush(faction);
        (VOID)SIclose(faction);
    }
    if (fdefine)
    {
        (VOID)SIflush(fdefine);
        (VOID)SIclose(fdefine);
    }
    if (ftable)
    {
        (VOID)SIflush(ftable);
        (VOID)SIclose(ftable);
    }
    if (ftemp)
    {
        (VOID)SIflush(ftemp);
        (VOID)SIclose(ftemp);
    }
    if (foutput)
    {
        (VOID)SIflush(foutput);
        (VOID)SIclose(foutput);
    }
    finput = faction = fdefine = ftable = ftemp = foutput = (FILE *)NULL;
}

/* put out other arrays, copy the parsers */
others()
{
    register i4     c,
		    lastc = -1,
		    i,
		    j;
    LOCATION	    parloc;
    FILE	    *fileptr = Filespecs[YYFUNC].fileptr;
    char	    *err_buf;
    char	    pname[MAX_LOC];
    char	    stbuf[STBUFSIZE];
# ifdef UNIX
    char	    *sym;
    char	    ing_src[256], vers[256];

    NMgtAt("ING_SRC",&sym);
    if (sym == NULL || *sym == EOS)
	ERROR("$ING_SRC not set");
    STcopy(sym, ing_src);
    NMgtAt("ING_VERS",&sym);
    if (sym)
       STcopy(sym, vers);
    else
	*vers=EOS;
    NMgtAt("PRIVATE_TOOLS",&sym);
    if (sym == NULL || *sym == EOS || *vers == EOS)
    {
	IISTprintf(pname,"%s/lib/%s",ing_src,PARSER);
    } else {
	IISTprintf(pname,"%s/%s/lib/%s",ing_src,vers,PARSER);
    }
# else
# ifdef OS2
    char	    *sym;

    NMgtAt("ING_SRC",&sym);
    if (sym == NULL || *sym == EOS)
	ERROR("$ING_SRC not set");
    STpolycat(3,sym,"\\lib\\",PARSER, pname);
# else
# ifdef WIN32
    char	    *sym;

    NMgtAt("ING_SRC",&sym);
    if (sym == NULL || *sym == EOS)
	ERROR("$ING_SRC not set");
    STpolycat(3,sym,"\\lib\\",PARSER, pname);
# else
    STcopy(PARSER,pname);
# endif
# endif
# endif
    if (LOfroms(PATH & FILENAME, pname, &parloc) != OK)
	ERROR(STprintf(stbuf, "bad parser filename %s", pname));

    if (SIopen(&parloc, "r", &finput) != OK)
	ERROR(STprintf(stbuf, "cannot find parser %s", pname));

    warray(YYR1, levprd, nprod);

    aryfil(temp1, nprod, 0);
    /* had_act[i] is either 1 or 0 */
    PLOOP(1,i)
    {
	temp1[i] = ((prdptr[i + 1] - prdptr[i] - 2) << 1) | had_act[i];
    }
    warray(YYR2, temp1, nprod );

    aryfil(temp1, nstate, -1000);
    TLOOP(i)
    {
	for(j = tstates[i]; j != 0; j = mstates[j])
	{
	    temp1[j] = tokset[i].value;
	}
    }
    NTLOOP(i)
    {
	for(j = ntstates[i]; j != 0; j = mstates[j])
	{
	    temp1[j] = -i;
	}
    }
    warray(YYCHK, temp1, nstate);

    warray(YYDEF, defact, nstate);

    if (SIopen(&debugloc, "r", &fdebug) != OK)
    {
	LOtos(&debugloc, &err_buf);
	ERROR(STprintf(stbuf, "cannot re-open %s", err_buf));
    }
    while ((c = SIgetc(fdebug)) != EOF)
	SIputc(c, ftable);
    SIclose(fdebug);
	fdebug = (FILE *)NULL;
    ZAPFILE(&debugloc);

    if (functions)
    {
	c = 0;
	PLOOP(0,i)
	{
	    if ((c = funcid(i)) && (c != lastc))
	    {
		lastc = c;
		SIfprintf(ftable, "FUNC_EXTERN\ti4\t%s%diftn();\n", prefix, c);
		SIfprintf(fileptr,"FUNC_EXTERN\ti4\t%s%diftn();\n", prefix, c);
	    }
	}
	/* A little c tutorial:  const foo *ptr -> pointer to const foo
	** foo * const ptr -> const pointer to foo
	** We want the latter.
	*/
	SIfprintf(fileptr, "\n\nGLOBALDEF \ti4\t(* const %sfunc[])()=\n{",
	    prefix);
	if (Filespecs[YYFUNC].filegiven)
	    SIfprintf(ftable, "\n\nextern\ti4\t(*%sfunc[])();\n", prefix);
	j = 0;
	PLOOP(0,i)
	{
	    if (j != 0)
	    {
		SIputc(',', fileptr);
	    }
	    j++;

	    if (c = funcid(i))
	    {
		SIfprintf(fileptr, "\n\t%s%diftn", prefix, c);
	    }
	    else
	    {
		SIfprintf(fileptr, "\n\t(i4 (*)()) NULL");
	    }
	}

	SIfprintf(fileptr, "\n};\n");
    }

    /* copy parser text */

    while((c = SIgetc(finput)) != EOF)
    {
	if (c == '$')
	{
	    c = SIgetc(finput);
	    switch (c)
	    {
		case 'A':
		case 'B':
		    if ((c == 'A' && !functions) || (c == 'B' && functions))
		    {
			/* copy actions */
			if (SIopen(&actloc, "r", &faction) != OK)
			{
			    LOtos(&actloc, &err_buf);
			    ERROR(STprintf(stbuf,
				"cannot reopen action tempfile %s", err_buf));
			}

			while ((c = SIgetc(faction)) != EOF)
			    SIputc(c, ftable);
			SIclose(faction);
			faction = (FILE *)NULL;
			ZAPFILE(&actloc);
		    }
		    c = SIgetc(finput);
		    break;
		case 'D':
		    {
		    i4	prevparm = FALSE;

		    SIfprintf(ftable, "%sparse(", prefix);
		    if (reentrant)
		    {
			prevparm = TRUE;
			SIfprintf(ftable, "cb");
		    }
		    for (i = 0; i < Numparams; i++)
		    {
			if (prevparm)
			    SIputc(',', ftable);
			prevparm = TRUE;
			SIfprintf(ftable, Params[i].parmname);
		    }
		    SIputc(')', ftable);
		    if (reentrant)
			SIfprintf(ftable, "\n%s\t*cb;", argtype);
		    for (i = 0; i < Numparams; i++)
		    {
			SIfprintf(ftable, "\n%s\t%s;", Params[i].parmtype,
			    Params[i].parmname);
		    }
		    c = SIgetc(finput);
		    break;
		    }
		case 'C':
		    if (reentrant)
			SIfprintf(ftable, "yacc_cb->");
		    c = SIgetc(finput);
		    break;
		case 'E':
		    if (reentrant)
		    {
			SIfprintf(ftable, "cb");
			if (Numparams)
			    SIputc(',', ftable);
		    }

		    for (i = 0; i < Numparams; i++)
		    {
			SIfprintf(ftable, "%s", Params[i].parmname);
			if (i < Numparams - 1)
			    SIputc(',', ftable);
		    }
		    c = SIgetc(finput);
		    break;
		case 'F':
		    SIfprintf(ftable, prefix);
		    c = SIgetc(finput);
		    break;
		case 'G':
		    if (functions)
		    {
			if (reentrant)
			{
			    SIfprintf(ftable, ", cb");
			    if (Numparams)
				SIputc(',', ftable);
			}

			for (i = 0; i < Numparams; i++)
			{
			    SIfprintf(ftable, "%s", Params[i].parmname);
			    if (i < Numparams - 1)
				SIputc(',', ftable);
			}
		    }
		    c = SIgetc(finput);
		    break;
		case 'H':
		    if (reentrant)
		    {
			SIfprintf(ftable, "(YACC_CB *) (cb->%s)", structname);
		    }
		    c = SIgetc(finput);
		    break;
		case 'I':
		    if (reentrant)
		    {
			SIfprintf(ftable, ", cb");
		    }

		    for (i = 0; i < Numparams; i++)
		    {
			SIfprintf(ftable, ", %s", Params[i].parmname);
		    }
		    c = SIgetc(finput);
		    break;
		default:
		    SIputc('$', ftable );
		    break;
            }
	}
	SIputc(c, ftable);
    }

    SIclose(ftable);
	ftable = (FILE *)NULL;
}

char *
chcopy(p, q)
char	*p;
char	*q;
{
    /* copies string q into p, returning next free char ptr */
    while(*p = *q++)
	++p;
    return( p );
}

#define             ISIZE               400
#define		    STACKSPOT		" ^ "

/* creates output string for item pointed to by pp */
char *
writem(pp)
i4  *pp;
{
    i4		i,*p;
    static char sarr[ISIZE];
    char	*q;

    for (p = pp; *p > 0 ; ++p)
	;
    p = prdptr[-*p];
    q = chcopy(sarr, nontrst[*p-NTBASE].name);
    q = chcopy(q, " : ");

    for(;;)
    {
	/* '_' used to be used as the character to show the top of stack */
	if (++p == pp)
	{
	    MEcopy(STACKSPOT, (u_i2) (sizeof(STACKSPOT) - 1), q);
	    q += sizeof(STACKSPOT) - 1;
	}
	else
	{
	    *q++ = ' ';
	}
	*q = '\0';
	if((i = *p) <= 0)
	    break;
	q = chcopy(q, symnam(i));
	if (q > &sarr[ISIZE - 30])
	    ERROR("item too big" );
    }

    if ((i = *pp) < 0)
    {
	/* an item calling for a reduction */
	q = chcopy(q, "    (");
	STprintf(q, "%d)", -i);
    }

    return (sarr);
}

/* return a pointer to the name of symbol i */
char *
symnam(i)
{
    char    *cp;

    cp = (i >= NTBASE) ? nontrst[i - NTBASE].name : tokset[i].name;
    if (*cp == ' ')
	++cp;
    return (cp);
}

GLOBALREF struct wset *zzcwp;
GLOBALREF i4  zzgoent;
GLOBALREF i4  zzgobest;
GLOBALREF i4  zzacent;
GLOBALREF i4  zzexcp;
GLOBALREF i4  zzclose;
GLOBALREF i4  zzsrconf;
GLOBALREF i4  * zzmemsz;
GLOBALREF i4  zzrrconf;

/* output the summary on the tty */
summary()
{
    if (foutput!=NULL)
    {
	SIfprintf(foutput, "\n%d/%d terminals, %d/%d nonterminals\n", ntokens,
		NTERMS, nnonter, NNONTERM);
	SIfprintf(foutput, "%d/%d grammar rules, %d/%d states\n", nprod, NPROD,
		nstate, NSTATES);
	SIfprintf(foutput,
		"%d shift/reduce, %d reduce/reduce conflicts reported\n",
		zzsrconf, zzrrconf);
	SIfprintf(foutput, "%d/%d working sets used\n", zzcwp-wsets,  WSETSIZE);
	SIfprintf(foutput, "memory: states,etc. %d/%d, parser %d/%d\n",
		zzmemsz-mem0, MEMSIZE, memp-amem, ACTSIZE);
	SIfprintf(foutput, "%d/%d distinct lookahead sets\n", nlset, LSETSIZE);
	SIfprintf(foutput, "%d extra closures\n", zzclose - 2*nstate);
	SIfprintf(foutput, "%d shift entries, %d exceptions\n", zzacent, zzexcp);
	SIfprintf(foutput, "%d goto entries\n", zzgoent);
	SIfprintf(foutput, "%d entries saved by goto default\n", zzgobest);
    }
    if (zzsrconf!=0 || zzrrconf!=0)
    {
	SIfprintf(stderr,"\nconflicts: ");
	if (zzsrconf)
	    SIfprintf(stderr, "%d shift/reduce" , zzsrconf);
	if (zzsrconf && zzrrconf)
	    SIfprintf(stderr, ", ");
	if (zzrrconf)
	    SIfprintf(stderr, "%d reduce/reduce" , zzrrconf);
	SIfprintf(stderr, "\n");
    }

    if (ftemp != NULL)
	SIclose(ftemp);
	ftemp = (FILE *)NULL;
    if (fdefine != NULL)
	SIclose(fdefine);
	fdefine = (FILE *)NULL;
}

/* write out error comment */
ERROR(s)
char	*s;
{	
    ++nerrors;
    SIfprintf(stderr, "\n fatal error: ");
    SIfprintf(stderr, s);
    SIfprintf(stderr, ", line %d\n", lineno);
    if (!fatfl)
	return;
    summary();
    PCexit(FAIL);
}

/* set elements 0 through n-1 to c */
aryfil(v, n, c)
i4	*v;
i4	n;
i4	c;
{
    i4	    i;

    for (i = 0; i < n; ++i)
	v[i] = c;
}

/*
** Set a to the union of a and b.
** Return 1 if b is not a subset of a, 0 otherwise.
*/

setunion(a, b)
register i4	*a;
register i4	*b;
{
    register i4     i, x, sub;

    sub = 0;
    SETLOOP(i)
    {
	x = *a;
	*a |= *b++;
	if (*a++ != x)
	    sub = 1;
    }
    return (sub);
}

prlook(p)
struct looksets	    *p;
{
    register i4     j, *pp;

    pp = p->lset;
    if (pp == 0)
    {
	SIfprintf(foutput, "\tNULL");
    }
    else
    {
	SIfprintf(foutput, " { ");
	TLOOP(j)
	{
	    if(BIT(pp,j))
		SIfprintf(foutput, "%s ", symnam(j));
	}
	SIfprintf(foutput,  "}");
    }
}

/*
** Compute an array with the beginnings of  productions yielding given
** nonterminals.  The array pres points to these lists
*/
cpres()
{
    /* the array pyield has the lists: the total size is only NPROD+1 */
    register i4     **pmem;
    register i4     c, j, i;
    static i4	    *pyield[NPROD];
    char stbuf[STBUFSIZE];

    pmem = pyield;

    NTLOOP(i)
    {
	c = i + NTBASE;
	pres[i] = pmem;
	fatfl = 0;  /* make undefined  symbols  nonfatal */
	PLOOP(0, j)
	{
	    if (*prdptr[j] == c)
		*pmem++ =  prdptr[j] + 1;
	}
	if(pres[i] == pmem)
	{
	    ERROR(STprintf(stbuf, "nonterminal %s not defined!",
		nontrst[i].name));
	}
    }
    pres[i] = pmem;
    fatfl = 1;
    if (nerrors)
    {
	summary();
	PCexit(FAIL);
    }
    if (pmem != &pyield[nprod])
	ERROR(STprintf(stbuf, "internal Yacc error: pyield %d",
	    pmem-&pyield[nprod]));
}

GLOBALREF int indebug;

cpfir()
{
    /* compute an array with the first of nonterminals */
    register i4     *p, **s, i, **t, ch, changes;

    zzcwp = &wsets[nnonter];
    NTLOOP(i)
    {
	aryfil(wsets[i].ws.lset, tbitset, 0);
    	t = pres[i + 1];
	for (s = pres[i]; s < t; ++s)
	{
	    /* initially fill the sets */
	    for (p = *s; (ch = *p) > 0; ++p)
	    {
		if (ch < NTBASE)
		{
		    SETBIT(wsets[i].ws.lset, ch);
		    break;
		}
		else if (!pempty[ch - NTBASE])
		{
		    break;
		}
	    }
	}
    }

    /* now, reflect transitivity */

    changes = 1;
    while (changes)
    {
	changes = 0;
	NTLOOP(i)
	{
	    t = pres[i+1];
	    for (s = pres[i]; s < t; ++s)
	    {
		for (p = *s; (ch = (*p - NTBASE)) >= 0; ++p)
		{
		    changes |= setunion(wsets[i].ws.lset, wsets[ch].ws.lset);
		    if (!pempty[ch])
			break;
		}
	    }
	}
    }

    NTLOOP(i)
    {
	pfirst[i] = flset(&wsets[i].ws);
    }
    if (!indebug)
	return;
    if ((foutput != NULL))
    {
	NTLOOP(i)
	{
	    SIfprintf(foutput,  "\n%s: ", nontrst[i].name);
	    prlook(pfirst[i]);
	    SIfprintf(foutput,  " %d\n", pempty[i]);
	}
    }
}

/* sorts last state,and sees if it equals earlier ones. returns state number */
state(c)
i4	c;
{
    i4		    size1, size2;
    register i4     i;
    struct item	    *p1, *p2, *k, *l, *q1, *q2;

    p1 = pstate[nstate];
    p2 = pstate[nstate + 1];
    if (p1 == p2)
	return(0); /* null state */
    /* sort the items */
    for (k = p2 - 1; k > p1; k--)   /* make k the biggest */
    {
	for (l = k - 1; l >= p1; --l)
	{
	    if (l->pitem > k->pitem)
	    {
		i4 *s;
		struct looksets *ss;

		s = k->pitem;
		k->pitem = l->pitem;
		l->pitem = s;
		ss = k->look;
		k->look = l->look;
		l->look = ss;
	    }
	}
    }
    size1 = p2 - p1; /* size of state */

    for (i = (c >= NTBASE) ? ntstates[c - NTBASE] : tstates[c]; i != 0;
	i = mstates[i])
    {
	/* get ith state */
	q1 = pstate[i];
	q2 = pstate[i + 1];
	size2 = q2 - q1;
	if (size1 != size2)
	    continue;
	k = p1;
	for (l = q1; l < q2; l++)
	{
	    if (l->pitem != k->pitem)
		break;
	    ++k;
	}
	if (l != q2)
	    continue;
	/* found it */
	pstate[nstate + 1] = pstate[nstate]; /* delete last state */
	/* fix up lookaheads */
	if (nolook)
	    return(i);
	for (l = q1, k = p1; l < q2; ++l, ++k)
	{
	    int s;

	    SETLOOP(s)
	    {
		clset.lset[s] = l->look->lset[s];
	    }
	    if (setunion(clset.lset, k->look->lset))
	    {
		tystate[i] = MUSTDO;
		/* register the new set */
		l->look = flset(&clset);
	    }
	}
	return (i);
    }
    /* state is new */
    if (nolook)
	ERROR("yacc state/nolook error");
    pstate[nstate+2] = p2;
    if (nstate + 1 >= NSTATES)
	ERROR("too many states");
    if (c >= NTBASE)
    {
	mstates[nstate] = ntstates[c - NTBASE];
	ntstates[c - NTBASE] = nstate;
    }
    else
    {
	mstates[nstate] = tstates[c];
	tstates[c] = nstate;
    }
    tystate[nstate]=MUSTDO;
    return (nstate++);
}

GLOBALREF i4	pidebug; /* debugging flag for putitem */

putitem(ptr, lptr)
i4		*ptr;
struct looksets *lptr;
{
    register struct item *j;

    if (pidebug && (foutput!=NULL))
    {
	SIfprintf(foutput, "putitem(%s), state %d\n", writem(ptr), nstate);
    }
    j = pstate[nstate + 1];
    j->pitem = ptr;
    if (!nolook)
	j->look = flset(lptr);
    pstate[nstate + 1] = ++j;
    if ((i4 *) j > zzmemsz)
    {
	zzmemsz = (i4 *) j;
	if (zzmemsz >=  &mem0[MEMSIZE])
	    ERROR("out of state space");
    }
}

/*
** Mark nonterminals which derive the empty string.
** Also, look for nonterminals which don't derive any token strings.
*/

cempty()
{
#define             EMPTY               1
#define		    WHOKNOWS		0

    register i4     i, *p;
    char stbuf[STBUFSIZE];

/*
** first, use the array pempty to detect productions
** that can never be reduced
*/

    /* set pempty to WHONOWS */
    aryfil(pempty, nnonter + 1, WHOKNOWS);

    /* now, look at productions, marking nonterminals which derive something */

    more:
    PLOOP(0,i)
    {
	if (pempty[*prdptr[i] - NTBASE])
	    continue;
	for (p = prdptr[i] + 1; *p >= 0; ++p)
	{
	    if (*p >= NTBASE && pempty[*p - NTBASE] == WHOKNOWS)
		break;
	}
	if (*p < 0)	/* production can be derived */
	{
	    pempty[*prdptr[i] - NTBASE] = EMPTY;
	    goto more;
	}
    }

    /* now, look at the nonterminals, to see if they are all EMPTY */

    NTLOOP(i)
    {
	/* the added production rises or falls as the start symbol ... */
	if (i == 0)
	    continue;
	if (pempty[i] != EMPTY)
	{
	    fatfl = 0;
	    ERROR(STprintf(stbuf,
		"nonterminal %s never derives any token string",
		nontrst[i].name));
	}
    }

    if (nerrors)
    {
	summary();
	PCexit(FAIL);
    }

    /*
    ** now, compute the pempty array,
    ** to see which nonterminals derive the empty string
    */

    /* set pempty to WHOKNOWS */

    aryfil(pempty, nnonter + 1, WHOKNOWS);

    /* loop as long as we keep finding empty nonterminals */

again:
    PLOOP(1,i)
    {
	if(pempty[*prdptr[i] - NTBASE] == WHOKNOWS) /* not known to be empty */
	{
	    for (p = prdptr[i] + 1;
		*p >= NTBASE && pempty[*p - NTBASE] == EMPTY; ++p)
		    ;
	    if (*p < 0)	    /* we have a nontrivially empty nonterminal */
	    {
		pempty[*prdptr[i] - NTBASE] = EMPTY;
		goto again; /* got one ... try for another */
	    }
	}
    }
}

GLOBALREF i4	gsdebug;

/* generate the states */
stagen()
{
    i4			    i, j;
    register i4	    c;
    register struct wset    *p, *q;
    struct item *base;
    i4			    c_bug;

    /* initialize */

    nstate = 0;
    /*
    ** THIS IS FUNNY from the standpoint of portability.
    ** It represents the magic moment when the mem0 array, which has
    ** been holding the productions, starts to hold item pointers, of a
    ** different type...
    ** Someday, alloc should be used to allocate all this stuff... for now, we
    ** accept that if pointers don't fit in integers, there is a problem...
    */

    base = (struct item *) ME_ALIGN_MACRO(mem, sizeof(struct item *));
    pstate[0] = pstate[1] = base;
    aryfil(clset.lset, tbitset, 0);
    putitem(prdptr[0] + 1, &clset);
    tystate[0] = MUSTDO;
    nstate = 1;
    pstate[2] = pstate[1];

    aryfil(amem, ACTSIZE, 0);

    /* now, the main state generation loop */

    more:
    SLOOP(i)
    {
	if (tystate[i] != MUSTDO)
	    continue;
	tystate[i] = DONE;
	aryfil(temp1, nnonter + 1, 0);
	/* take state i, close it, and do gotos */
	closure(i);
	WSLOOP(wsets,p)	    /* generate goto's */
	{
	    if (p->flag)
		continue;
	    p->flag = 1;
	    c = *(p->pitem);
	    if (c <= 1)
	    {
		/*
		** The following assignment to a temporary was done to get
		** around a bug in a C compiler.
		*/
		c_bug = pstate[i + 1] - pstate[i];
		if (c_bug <= p - wsets)
		    tystate[i] = MUSTLOOKAHEAD;
		continue;
	    }
	    /* do a goto on c */
	    WSLOOP(p,q)
	    {
		if (c == *(q->pitem))	/* this item contributes to the goto */
		{
		    putitem(q->pitem + 1, &q->ws);
		    q->flag = 1;
		}
	    }
	    if (c < NTBASE)
	    {
		state(c);  /* register new state */
	    }
	    else
	    {
		temp1[c - NTBASE] = state(c);
	    }
	}
	if (gsdebug && (foutput != NULL))
	{
	    SIfprintf(foutput, "%d: ", i);
	    NTLOOP(j)
	    {
		if (temp1[j])
		    SIfprintf(foutput, "%s %d, ", nontrst[j].name, temp1[j]);
	    }
	    SIfprintf(foutput, "\n");
	}
	indgo[i] = apack(&temp1[1], nnonter - 1) - 1;
	goto more; /* we have done one goto; do some more */
    }
    /* no more to do... stop */
}

GLOBALREF i4	cldebug;	/* debugging flag for closure */

/* generate the closure of state i */
closure(i)
i4	i;
{
    i4			    c, ch, work, k;
    register struct wset    *u, *v;
    i4			    *pi;
    i4			    **s, **t;
    struct item		    *q;
    register struct item    *p;

    ++zzclose;

    /* first, copy kernel of state i to wsets */

    cwp = wsets;
    ITMLOOP(i,p,q)
    {
	cwp->pitem = p->pitem;
	cwp->flag = 1;    /* this item must get closed */
	SETLOOP(k)
	{
	    cwp->ws.lset[k] = p->look->lset[k];
	}
	WSBUMP(cwp);
    }

    /* now, go through the loop, closing each item */

    work = 1;
    while (work)
    {
	work = 0;
	WSLOOP(wsets,u)
	{
	    if (u->flag == 0)
		continue;
	    c = *(u->pitem);  /* dot is before c */
	
	    if (c < NTBASE)
	    {
		u->flag = 0;
		continue;  /* only interesting case is where . is b4 nonterm */
	    }
	
	    /* compute the lookahead */
	    aryfil(clset.lset, tbitset, 0);

	    /* find items involving c */

	    WSLOOP(u,v)
	    {
		if (v->flag == 1 && *(pi = v->pitem) == c)
		{
		    v->flag = 0;
		    if (nolook)
			continue;
		    while ((ch = *++pi) > 0)
		    {
			if (ch < NTBASE)    /* terminal symbol */
			{
			    SETBIT(clset.lset, ch);
			    break;
			}
			/* nonterminal symbol */
			setunion(clset.lset, pfirst[ch - NTBASE]->lset);
			if (!pempty[ch - NTBASE])
			    break;
		    }
		    if (ch <= 0)
			setunion(clset.lset, v->ws.lset);
		}
	    }
	
	    /*  now loop over productions derived from c */
	
	    c -= NTBASE; /* c is now nonterminal number */
	
	    t = pres[c + 1];
	    for (s = pres[c]; s < t; ++s)
	    {
		/* put these items into the closure */
		WSLOOP(wsets,v)
		{
		    if (v->pitem == *s)	    /* is the item there */
		    {
			/* yes, it is there */
			if (nolook)
			    goto nexts;
			if (setunion(v->ws.lset, clset.lset))
			    v->flag = work = 1;
			goto nexts;
		    }
		}
	
		/*  not there; make a new entry */
		if (cwp - wsets + 1 >= WSETSIZE)
		    ERROR("working set overflow");
		cwp->pitem = *s;
		cwp->flag = 1;
		if (!nolook)
		{
		    work = 1;
		    SETLOOP(k)
		    {
			cwp->ws.lset[k] = clset.lset[k];
		    }
		}
		WSBUMP(cwp);
	
		nexts: ;
	    }
	}
    }

    /* have computed closure; flags are reset; return */

    if (cwp > zzcwp)
	zzcwp = cwp;
    if (cldebug && (foutput != NULL))
    {
	SIfprintf (foutput, "\nState %d, nolook = %d\n", i, nolook);
	WSLOOP(wsets,u)
	{
	    if (u->flag)
		SIfprintf(foutput, "flag set!\n");
	    u->flag = 0;
	    SIfprintf(foutput, "\t%s", writem(u->pitem));
	    prlook(&u->ws);
	    SIfprintf(foutput,  "\n");
	}
    }
}

struct looksets *
flset(p)
struct looksets	    *p;
{
    /* decide if the lookahead set pointed to by p is known */
    /* return pointer to a perminent location for the set */

    register struct looksets	*q;
    i4				j, *w;
    register i4		*u, *v;

    for (q = &lkst[nlset]; q-- > lkst; )
    {
	u = p->lset;
	v = q->lset;
	w = & v[tbitset];
	while (v < w)
	{
	    if (*u++ != *v++)
		goto more;
	}
	/* we have matched */
	return (q);
	more: ;
    }
    /* add a new one */
    q = &lkst[nlset++];
    if (nlset >= LSETSIZE)
	ERROR("too many lookahead sets");
    SETLOOP(j)
    {
	q->lset[j] = p->lset[j];
    }
    return( q );
}
