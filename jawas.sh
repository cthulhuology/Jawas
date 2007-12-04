#!/bin/sh
#

# PROVIDE: jawas
# REQUIRE: LOGIN
# KEYWORD: shutdown

#
# Add the following lines to /etc/rc.conf to enable jawas:
#
#jawas_enable="YES"
#

. /etc/rc.subr

name=jawas
rcvar=`set_rcvar`

: ${jawas_enable:="NO"}
command=/usr/local/sbin/${name}

load_rc_config ${name}
run_rc_command "$1" prod
