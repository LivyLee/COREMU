#!/usr/bin/env ruby

require 'pty'
require 'expect'

require 'coremu'

#puts COREMU.corey_cmd 'record', 4
COREMU.run_corey 'record', 4
#COREMU.run_corey_autoexit 'record', 4

