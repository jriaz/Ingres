$! Copyright (c) 2007  Ingres Corporation
$!
$! This script generates the kitinstal.data in front!st!install_vms.
$!
$! The four kitinstal parameters are handled as follows:
$!
$! RELEASE identifier - as generated by genrelid
$! MINIMUM VMS - determined by the machine where this is running
$! MINIMUM SPACE - Hard-coded as min_space 
$!                 (use size of savesets A and B to modify)
$! LAST SAVESET -  Hard-coded as B since currently documentation
$!                 saveset (C) is no longer delivered
$!
$!! History:
$!!     24-Apr-2007   (UpaKe01)   
$!!        Created.
$!! 
$!
$ set symbol/scope=noglobal/verb
$!
$ min_space = 500000
$ last_saveset = "B"
$!
$ vms_ver = f$getsyi("VERSION")
$! get the position of decimal point to determine if there is one
$! digit or two digits before the decimal (V7.3 or V10.4)
$ vms_ver_dec_pos = f$locate(".",vms_ver)
$! remove "V", period, hyphen and spaces from the field
$ vms_ver = vms_ver - "V" - "." - "-"
$ vms_ver = f$edit(vms_ver,"TRIM")
$! put a "0" in front if the version # is below 10.x
$ if vms_ver_dec_pos .eq. 2 then vms_ver = "0" + vms_ver
$!
$ echo := write sys$output
$!
$!! In the following PIPE command, GENRELID will create a temporary
$!! file that will be deleted at the end.  The user may have a global
$!! symbol for DELETE that could be Delete/Confirm.  In that case the
$!! output file (Kitinstal.data) will also contain the delete prompt.
$!! The "set symbol" before GENRELID will disable global symbols.
$ pipe ( set symbol/scope=noglobal/verb ; genrelid ) | - 
       ( read sys$pipe rel_id ; define/job/nolog rel_id_log &rel_id ) ; - 
       rel_id_sym=f$trnlnm("rel_id_log") ; -
       deassign/job rel_id_log
$!
$ kit_file := "ING_SRC:[front.st.install_vms]KITINSTAL.DATA"
$ open/write/error=_kitinstal_open_w_err  outfile  'kit_file'
$ writekit := "write/error=_kitinstal_write_err outfile"
$!
$ writekit "                                 KITINSTAL.DATA"
$ writekit " "
$ writekit "This data file is used to pass data which may change from time to time to the"
$ writekit "KITINSTAL.COM procedure, thus avoiding having to edit kitinstal.com.  Please"
$ writekit "see the GET_KIT_DATA section of KITINSTAL.COM for usage information."
$ writekit " "
$ writekit "RELEASE:             ''rel_id_sym'"
$ writekit "MINIMUM VMS:         ''vms_ver'"
$ writekit "MINIMUM SPACE:       ''min_space'"
$ writekit "LAST SAVESET:        ''last_saveset'"
$!
$ close outfile
$ purge/nolog 'kit_file'
$ goto _exit_routine
$!
$ _kitinstal_open_w_err:
$ echo  "Error creating KITINSTAL.DATA file.  KITINSTAL.DATA not created"
$ goto _exit_routine
$!
$ _kitinstal_write_err:
$ close outfile
$ echo  "Error writing to KITINSTAL.DATA file.  KITINSTAL.DATA is incomplete"
$ delete/nolog/noconfirm   'kit_file';
$ goto _exit_routine
$!
$ _exit_routine:
$ exit
