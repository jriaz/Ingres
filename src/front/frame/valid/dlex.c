/*
**	dlex.c
**
**	Copyright (c) 2004 Ingres Corporation
**	All rights reserved.
*/


/* # includes */
# include	<compat.h>
# include	<cv.h>
# include	<me.h>
# include	<gl.h>
# include	<sl.h>
# include	<iicommon.h>
# include	<fe.h>
# include	<ft.h>
# include	<fmt.h>
# include	<adf.h>
# include	<frame.h>
# include	"derive.h"
# include	<afe.h>
# include	<cm.h>
# include	<er.h>
# include	<si.h>
# include	<st.h>
# include	<erfi.h>

/**
**  Name:	dlex.c - Lex for derivation parser.
**
**  Description:
**	This file contains routines that provide lex support for
**	the derivation parser.
**
**	This file defines:
**
**	IIFVdeErr		Generic derivation parser error routine.
**	errcall			Error routine for derivation lex analyzer.
**	IIFVdbcbadchar		Bad character error routine for lex.
**	IIFVctCharType		Get character type for derivation lex.
**	IIFVdlex		Lex analyzer for derivation parser.
**	IIFVdsDrvString		Find a single quoted string in input.
**	IIFVdscDrvStrCnv	Convert a string for derviation parser.
**
**  History:
**	06/05/89 (dkh) - Initial version.
**	27-sep-90 (bruceb)
**		Made changes so parser understands/works with kanji.
**	19-oct-1990 (mgw)
**		Fixed #include of local derive.h to use "" instead of <>
**	03/24/91 (dkh) - Saved away text of floating point number so
**			 we don't have to reconvert later on if a an
**			 error occurs.
**	05/02/92 (dkh) - Fixed bug 44001.  Increased size of buffer sbuf
**			 so it can handle the count aggregate name.
**	08-jun-92 (leighb) DeskTop Porting Change:
**		Changed 'errno' to 'errnum' cuz 'errno' is a macro in WIN/NT
**	06/12/92 (dkh) - Added support for decimal datatype for 6.5.
**	10/28/92 (dkh) - Changed ALPHA to DV_ALPHA for compiling on DEC/ALPHA.
**	08/31/93 (dkh) - Fixed up compiler complaints due to prototyping.
**      24-sep-96 (hanch04)
**          Global data moved to data.c
**	10-Nov-1999 (kitch01)
**		Bug 98227. Change is based on the fix for bugs 79824 & 80718.
**		Change scanner IIFVdlex() to allow decimal point to be whatever
**		user specified via II_APP_DECIMAL.
**	21-jan-1999 (hanch04)
**	    replace nat and longnat with i4
**	31-aug-2000 (hanch04)
**	    cross change to main
**	    replace nat and longnat with i4
*/

/* # define's */
# define	GETCHAR()	IIFVgcGetChar()
# define	BACKUP()	(vl_buffer--)

GLOBALREF	char	IIFVfloat_str[];

/* extern's */

GLOBALREF	char	*vl_buffer;
GLOBALREF	FRAME	*vl_curfrm;
GLOBALREF	FLDHDR	*vl_curhdr;
GLOBALREF	FLDTYPE	*vl_curtype;
GLOBALREF	IIFVdstype IIFVdlval;
GLOBALREF	bool	vl_syntax;

FUNC_EXTERN	u_char	*IIFVgcGetChar();
FUNC_EXTERN	i4	IIFVdsDrvString();
FUNC_EXTERN	i4	IIFVdsvDrvStrCnv();
FUNC_EXTERN	VOID	IIFVdpError();
FUNC_EXTERN	char	*IIFVdfnDrvFrmName();
FUNC_EXTERN ADF_CB *FEadfcb();


/* static's */
static	char	*Lastok = NULL;

static	PTR	decdata = NULL;



/*{
** Name:	IIFVdeErr - Generic derivation error routine.
**
** Description:
**	This routine is called by derivation parser to signal a
**	error of some sort while parsing a derivation formula.
**	The real work is done by calling IIFVdpError() which
**	actually displays the error and exits the parser.
**	This provides a convienent cover where the name of
**	the form and fields can be added to the error message.
**
** Inputs:
**	errnum		Error number identifier the error that occurred.
**	argcount	Number of optional parameters to be formatted
**			with the error message.
**	a1		Parameter 1.
**	a2		Parameter 2.
**	a3		Parameter 3.
**	a4		Parameter 4.
**	a5		Parameter 5.
**
** Outputs:
**	None.
**	Returns:
**		None.
**	Exceptions:
**		None.
**
** Side Effects:
**	None.
**
** History:
**	06/05/89 (dkh) - Initial version.
*/
VOID
IIFVdeErr(errnum, argcount, a1, a2, a3, a4, a5)
ER_MSGID	errnum;
i4		argcount;
PTR		a1;
PTR		a2;
PTR		a3;
PTR		a4;
PTR		a5;
{
	IIFVdpError(errnum, argcount + 2, IIFVdfnDrvFrmName(),
		vl_curhdr->fhdname, a1, a2, a3, a4, a5);
}




/*{
** Name:	errcall - Error routine for the derivation lex analyzer.
**
** Description:
**	A simple cover routine for errors generated by the lex analyzer
**	of the derivation parser.  Calls IIFVdeErr() to do the real work.
**
** Inputs:
**	errnum		The error number.
**
** Outputs:
**	None.
**	Returns:
**		None.
**	Exceptions:
**		None.
**
** Side Effects:
**	None.
**
** History:
**	06/05/89 (dkh) - Initial version.
*/
static VOID
errcall(errnum)
ER_MSGID	errnum;
{
	IIFVdeErr(errnum, 1, Lastok);
}



/*{
** Name:	IIFVdbcbadchar - Bad character error routine for derivation.
**
** Description:
**	Error routine for handling bad/unknown characters found by the
**	lex analyzer of the derivation parser.
**
** Inputs:
**	ch		The unknown character.
**
** Outputs:
**	None.
**	Returns:
**		None.
**	Exceptions:
**		None.
**
** Side Effects:
**	None.
**
** History:
**	06/05/89 (dkh) - Initial version.
*/
static VOID
IIFVdbcBadChar(ch)
char	*ch;
{
	char	buf[40];

	if (CMcntrl(ch))
	{
		STcopy(CMunctrl(ch), buf);
	}
	else
	{
		buf[1] = EOS;
		buf[2] = EOS;
		CMcpychar(ch, buf);
	}
	IIFVdeErr(E_FI209E_8350, 1, (PTR) buf);
}



/*
**  v_chtype
**
**  Routine to return the type of the character passed to it
*/

/*{
** Name:	IIFVctCharType - Get character type for derivation lex.
**
** Description:
**	Given a character, this routine determines the class/type
**	of the character.  Characters that don't fall into any
**	known class are assumed to be of the CNTRL class.
**
** Inputs:
**	ch		Character to determine class/type on.
**
** Outputs:
**
**	Returns:
**		DV_ALPHA	Input char was in the alphabetic class.
**		NUMBR		Input char was in the numeric class.
**		PUNCT		Input char was in the punctuation class.
**		OPATR		Input char was in the operator class.
**		CNTRL		Input char was in the control/unknown class.
**	Exceptions:
**		None.
**
** Side Effects:
**	None.
**
** History:
**	06/05/89 (dkh) - Initial version.
**	10-Nov-1999 (kitch01)
**		Bug 98227. Allow comma to be used as decimal separator.
*/
char
IIFVctCharType(ch)
u_char	*ch;
{
	char	retval;

	if (CMnmstart(ch) || *ch == '_')
	{
		retval = DV_ALPHA;
	}
	else if (CMdigit(ch))
	{
		retval = NUMBR;
	}
	else
	{
		switch(*ch)
		{
			case ' ':
				retval = PUNCT;
				break;

			case '(':
			case ')':
			case '[':
			case ']':
			case '.':
			case ',':
			case '-':
			case '+':
			case '*':
			case '/':
			case '\'':
				retval = OPATR;
				break;

			default:
				retval = CNTRL;
				break;
		}
	}
	return(retval);
}





/*{
** Name:	IIFVdlex - Lex analyzer for derivation parser.
**
** Description:
**	This routine is called by the derivation parser to pass
**	back tokens found in the input string pointed to by vl_buffer.
**	Communication of the various constant value (integer, float, string)
**	back to the parser is via IIFVdlval.  Token type and values are
**	defined in derive.h and dtokens.h
**
**	The end of file token is zero.
**
** Inputs:
**	None.
**
** Outputs:
**
**	Returns:
**		token	Token types defined in derive.h and dtokens.h
**	Exceptions:
**		None.
**
** Side Effects:
**	None.
**
** History:
**	06/05/89 (dkh) - Initial version.
**	10-Nov-1999 (kitch01)
**		Bug 98227. Allow comma to be used as decimal separator.
*/
static ADF_CB *Adf_cb;
static char decimal;

i4
IIFVdlex()
{
	register u_char	*chr;
	register u_char	*ptr;
	register i2	rtval;
	register struct dspec *op;
	register i4	ttype;
static	f8		ftemp;
static	i4		ltemp;
static	DB_DATA_VALUE	decdbv;
static	char		sbuf[12];
	DB_DATA_VALUE	dmydbv;
	u_char		buf[256];
	u_char		*nextchar;
	i4		digit_count;
	i4		is_exponential;
	i4		scale_digits;
	DB_TEXT_STRING	*textstr;
	AFE_DCL_TXT_MACRO(fdVSTRLEN)	dectext;
	i4 status;
	char cp[2];

	if (Adf_cb == NULL)
	{
		Adf_cb = FEadfcb();
		/* Overwrite II_DECIMAL setting with II_APP_DECIMAL */
		cp[0]=cp[1]='\0';
		status = IIUGlocal_decimal(Adf_cb, cp);
		if (status)
		{
			vl_nopar_error(E_FI2270_8816_BadDecimal, 1, cp);
			return(status);
		}
		decimal = Adf_cb->adf_decimal.db_decimal;
	}
	/* Ensure we use the decimal decided by the provious logic */
	Adf_cb->adf_decimal.db_decimal = decimal;

	if (decdata == NULL)
	{
		decdata = MEreqmem((u_i4) 0, (u_i4) DB_MAX_DECLEN,
			FALSE, (STATUS *) NULL);
	}

	rtval = -1;
	Lastok = ERx("");
	op = &IIFVdTokens;
	/* GET NEXT TOKEN */
	do
	{
		chr = GETCHAR();
		if (*chr == EOS)
		{
			rtval = 0;
			break;
		}

		switch(IIFVctCharType(chr))
		{
		case DV_ALPHA:
			/* fill in the name */
			ptr = buf;
			*ptr = *chr;
			do
			{
				/*
				**  to ignore a 2-nd byte character of a
				**  Double Bytes character
				*/
				if (CMdbl1st(ptr))
				{
					chr = GETCHAR();
					*(++ptr) = *chr;
				}
				chr = GETCHAR();
				*(++ptr) = *chr;
				if ((ptr - buf) > FE_MAXNAME)
				{
					/* name too long */
					errcall(E_FI2080_8320);
				}

			}  while (CMnmchar(chr));
			BACKUP();
			*ptr = '\0';

			rtval = op->name;
			switch (buf[0])
			{
			  case 'A':
			  case 'a':
				if (STbcompare((char *) buf, 0, ERx("avg"), 0,
					TRUE) == 0)
				{
					rtval = op->average;
				}
				break;

			  case 'C':
			  case 'c':
				if (STbcompare((char *) buf, 0, ERx("count"),
					0, TRUE) == 0)
				{
					rtval = op->count;
				}
				break;

			  case 'M':
			  case 'm':
				if (STbcompare((char *) buf, 0, ERx("max"),
					0,TRUE) == 0)
				{
					rtval = op->d_max;
				}
				else if (STbcompare((char *) buf, 0, ERx("min"),
					0, TRUE) == 0)
				{
					rtval = op->d_min;
				}
				break;

			  case 'S':
			  case 's':
				if (STbcompare((char *) buf, 0, ERx("sum"), 0,
					TRUE) == 0)
				{
					rtval = op->sum;
				}
				break;
			}

			if (rtval != op->name)
			{
				/*
				**  If here, must have found an aggregate name.
				**  Note that there is no need to set
				**  IIFVdlval in this case since it is
				**  not use by the parser.
				*/
				STcopy((char *) buf, (char *) sbuf);
				Lastok = sbuf;
			}
			else
			{	/* else, USER DEFINED NAME */
				Lastok = IIFVdlval.name_type =
					STalloc((char *) buf);
			}
			break;

		case NUMBR:
			number:;
			is_exponential = FALSE;
			ptr = buf;
			/* compare to decimal */
			if ((*ptr = *chr) != decimal)
			{
				/*
				**  We already have one digit otherwise
				**  we would not be here.
				*/
				digit_count = 1;

				do
				{
					/*
					**  Keep track of the number of
					**  digits that we find.
					*/
					digit_count++;
					chr = GETCHAR();
					*++ptr = *chr;
				} while (IIFVctCharType(chr) == NUMBR);

				/*
				**  Subtract one since the above loop
				**  always counts ahead.
				*/
				digit_count--;
			}
			else
			{
               /*
               **  No digits yet since we are here due
               **  to a goto from the OPATR case having
               **  found a decimal point (. or ,).  
			   **  This will probably
               **  be a floating point number starting with
               **  a decimal point.
               */
               digit_count = 0;
			}

			/* do rest of type determination */
			switch (*ptr)
			{
			case '.':
			case ',':
				/* check if this is decimal or not */
				if (*ptr != decimal)
				{
					goto conv2;
				}
				/* floating point */
				scale_digits = 0;
				do
				{
					/* fill into ptr with up to next non-digit */
					digit_count++;
					scale_digits++;
					chr = GETCHAR();
					*++ptr = *chr;
				} while (IIFVctCharType(chr) == NUMBR);

				digit_count--;
				scale_digits--;

				if (*ptr != 'e' && *ptr != 'E')
				{
					BACKUP();
					*ptr = EOS;
					goto convr;
				}

			case 'e':
			case 'E':
				is_exponential = TRUE;
				chr = GETCHAR();
				*++ptr = *chr;
				if (IIFVctCharType(chr) == NUMBR ||
					*ptr == '-' || *ptr == '+')
				{
					do
					{
						/* get exponent */
						chr = GETCHAR();
						*++ptr = *chr;
					} while (IIFVctCharType(chr) == NUMBR);
				}
				BACKUP();
				*ptr = EOS;
			    convr:
				if (is_exponential ||
					digit_count > DB_MAX_DECPREC)
				{
					/*
					** Use decimal point 
					** since this is not user input,
					** but a specification.
					*/
					if (CVaf(buf, decimal, &ftemp))
					{
						/* floating conversion error */
						errcall(E_FI2081_8321);
					}
					STcopy(buf, IIFVfloat_str);
					IIFVdlval.F8_type = ftemp;
					Lastok = (char *) buf;
					rtval = IIFVdTokens.f8const;
				}
				else
				{
					/*
                                        **  Convert to a decimal datatype to
                                        **  retain precision since the number
                                        **  of digits found for the constant
                                        **  is less than DB_MAX_DEC_PREC and
                                        **  it was not in exponential format.
                                        */
                                        dmydbv.db_datatype = DB_LTXT_TYPE;
                                        dmydbv.db_length = sizeof(dectext);
                                        dmydbv.db_prec = 0;
					textstr = (DB_TEXT_STRING *) &dectext;
					textstr->db_t_count = digit_count + 1;
					MEcopy((PTR) buf,
						(u_i2) textstr->db_t_count,
						(PTR) textstr->db_t_text);
                                        dmydbv.db_data = (PTR) textstr;
                                        decdbv.db_datatype = DB_DEC_TYPE;
                                        decdbv.db_length =
                                          DB_PREC_TO_LEN_MACRO(digit_count);
                                        decdbv.db_prec =
                                                DB_PS_ENCODE_MACRO(digit_count,scale_digits);
                                        decdbv.db_data = decdata;
                                        if (afe_cvinto(Adf_cb, &dmydbv,
                                                &decdbv) != OK)
                                        {
                                                /*
                                                **  Decimal conversion error.
                                                */
						errcall(E_FI2081_8321);
                                        }
					STcopy(buf, IIFVfloat_str);
                                        IIFVdlval.dec_type = &decdbv;
                                        Lastok = (char *) buf;
                                        rtval = IIFVdTokens.decconst;
				}
				break;

            conv2:
			default:
				/* integer */
				BACKUP();
				*ptr = EOS;
				/* numeric conversion error */
				if (CVal(buf, &ltemp))
				{
					/*
					**  Set digit_count to a value
					**  that will force a floating
					**  point conversion for proper
					**  overflow handling.
					*/
					digit_count = DB_MAX_DECPREC + 1;
					scale_digits = 0;
					is_exponential = FALSE;
					goto convr;
				}
				IIFVdlval.I4_type = ltemp;
				Lastok = (char *) buf;
				rtval = IIFVdTokens.i4const;
				break;
			}
			break;

		case OPATR:
			/* get lookahead characer */
			buf[1] = '\0';
			buf[2] = '\0';
			CMcpychar(chr, buf);

			ttype = 0;
			switch (*chr)
			{
			  case '(':
				rtval = op->lparen;
				break;

			  case ')':
				rtval = op->rparen;
				break;

			  case '[':
				rtval = op->lbrak;
				break;

			  case ']':
				rtval = op->rbrak;
				break;

			  case '.':
			  case ',':
			  {
				char	*nextchr;
				if (*chr == '.' )
				   rtval = op->period;
				else
				   rtval = op->comma;

				nextchr = (char *)GETCHAR();
				BACKUP();
				if ((*chr == decimal) &&
					(IIFVctCharType(nextchr) == NUMBR))
					goto number;
				break;
			  }

			  case '-':
				rtval = op->minus;
				ttype = d_opMINUS;
				break;

			  case '+':
				rtval = op->plus;
				ttype = d_opPLUS;
				break;

			  case '*':
				/* check for "**" too */
				rtval = op->times;
				ttype = d_opTIMES;
				nextchar = GETCHAR();
				if (*nextchar == '*')
				{
					rtval = op->exp;
					ttype = d_opEXP;
				}
				else
				{
					BACKUP();
				}
				break;

			  case '/':
				rtval = op->div;
				ttype = d_opDIV;
				break;

			  case '\'':
				rtval = IIFVdsDrvString();
				break;

			  default:
				/* invalid v_operator */
				errcall(E_FI2082_8322);
				rtval = -1;
				break;
			}
			if (rtval == -1 ||
				rtval == op->svconst)
			{
				break;
			}

			STcopy(buf, sbuf);
			Lastok = sbuf;
			IIFVdlval.type_type = ttype;
			break;

		case PUNCT:
			continue;

		case CNTRL:
			IIFVdbcBadChar(chr);
			continue;

		default:
			/*
			**  Unknown character type.
			**  Shouldn't happen since
			**  default type is CNTRL.
			*/
			IIFVdbcBadChar(chr);
		}
	}  while (rtval == -1);
	if (rtval == 0)
	{
		Lastok = ERx("");
	}
	return (rtval);
}



/*{
** Name:	IIFVdsDrvString - Find a single quoted string in input.
**
** Description:
**	This routine finds a single quoted string in the input.
**	The first quote has already been seen.  Doubling of the
**	single quote produces a single quote and does not end
**	the string.  If an unknown character is found, it causes
**	an exit out of the parser.  The found string is put into
**	a DB_TEXT_STRING structure allocated from permanent memory.
**
** Inputs:
**	None.
**
** Outputs:
**
**	Returns:
**		svcont		The varchar string contant token.
**	Exceptions:
**		None.
**
** Side Effects:
**	Unknown characters found will cause exit out of the parser.
**
** History:
**	06/05/89 (dkh) - Initial version.
*/
i4
IIFVdsDrvString()
{
	i4	esc;
	char	buf[MAXSTRING + 1];
	u_i2	len;
	char	*ptr;

	ptr = buf;
	do
	{
		if ((*ptr = *GETCHAR()) == EOS)
		{
			/*  Ran out of input - probably
			**  a non-terminated string
			*/
			errcall(E_FI2083_8323);
			break;
		}
		/* Skip the second byte of a double byte character. */
		if (CMdbl1st(ptr))
		{
			if ((*++ptr = *GETCHAR()) == EOS)
			{
                                /*
                                **  This should never happen but we do
                                **  it just in case.
                                */
				errcall(E_FI2083_8323);
				break;
			}

                        /*
                        **  If we are dealing with a double byte character,
                        **  we can skip the checks in the rest of the
                        **  loop since they will never be true.  So we
                        **  just do a continue to speed things up.
                        */
			ptr++;
			continue;
		}
		esc = 0;
		if (*ptr == '\'')
		{
			if ((*++ptr = *GETCHAR()) == EOS)
			{
				BACKUP();
				break;
			}
			if (*ptr == '\'')
			{
				esc = 1;
				ptr--;
			}
			else
			{
				ptr--;
				BACKUP();
			}
		}
		if (CMcntrl(ptr))
		{
			IIFVdbcBadChar(ptr);
			break;
		}
	} while (*ptr++ != '\'' || esc == 1);

	/*
	**  Find out the length of the string in bytes.
	*/
	len = (ptr - buf) - 1;

	*--ptr = EOS;
	return(IIFVdscDrvStrCnv(buf, len));
}





/*{
** Name:	IIFVdscDrvStrCnv - Convert a string for derivation parser.
**
** Description:
**	Put passed in string into a DB_TEXT_STRING structure allocated
**	from permanent memory.
**
** Inputs:
**	buf		Temporary buffer containing string.
**	len		Lenght (in bytes) of string.
**
** Outputs:
**
**	Returns:
**		svconst		Token for varchar string constant.
**	Exceptions:
**		None.
**
** Side Effects:
**	None.
**
** History:
**	06/05/89 (dkh) - Initial version.
*/
i4
IIFVdscDrvStrCnv(buf, len)
char	*buf;
u_i2	len;
{
	DB_TEXT_STRING	*sptr;

	/*
	**  Convert the string to a DB_TXT_TYPE DB_DATA_VALUE.
	**  Also, save the internal value by putting it in IIFVdlval.
	*/
	sptr = (DB_TEXT_STRING *)MEreqmem((u_i4)0, (u_i4)DB_CNTSIZE + len,
	    FALSE, (STATUS *)NULL);
	sptr->db_t_count = len;
	MEcopy((PTR) buf, (u_i2) len, (PTR) sptr->db_t_text);

	Lastok = (char *)sptr->db_t_text;

	IIFVdlval.string_type = sptr;

	return(IIFVdTokens.svconst);
}
