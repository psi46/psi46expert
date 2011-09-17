# .bashrc

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# User specific aliases and functions
alias ls='ls --color'
alias ll='ls -l'
alias la='ls -la'
alias lt='ls -lt'
alias h='history'
alias xterm='xterm -fg white -bg black'

export LS_COLORS='di=36'

export PS1="[\j \h \W\$(if [ -d CVS ]; then if [ -e CVS/Tag ]; then cat CVS/Tag | sed -e 's/^T/ /' | sed -e 's/^N/ /' | sed -e 's/^D/ Date /' | tr -d '\n'; else echo -n ' HEAD'; fi; echo -n ' '; cat CVS/Repository; fi)]$ "


 #ROOT
export PS1="[\j \u@\h \W\$(if [ -d CVS ]; then if [ -e CVS/Tag ]; then cat CVS/Tag | sed -e 's/^T/ /' | sed -e 's/^N/ /' | sed -e 's/^D/ Date /' | tr -d '\n'; else echo -n ' HEAD'; fi; echo -n ' '; cat CVS/Repository; elif [ -d .svn ]; then echo -n ' '; grep -A1 -E 'dir' ./.svn/entries | grep -E '[0-9]+' | tr -d '[\r\n]'; fi)]$ "
export ROOTSYS=/opt/root/5.22.00
export PATH=$ROOTSYS/bin:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib

export MALLOC_CHECK_=0

export PSI46CC=g++
